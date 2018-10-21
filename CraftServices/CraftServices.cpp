/*
 * This file is part of CraftServices.
 *
 * CraftServices is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CraftServices is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CraftServices. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <boost/asio.hpp> 
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
#include <codecvt>
#include <iostream>
using std::cin;

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "SerialPortEnumerator.hpp"
#include "PhantomTestCraft.hpp"
#include "PhantomTestCraftFixed.hpp"
#include "PhantomWingman.hpp"
#include "MSPFlightControllerAsync.hpp"
#include "CraftServices.hpp"
#include "SerialPortDefaults.hpp"
#include "CommandLineArgumentsException.hpp"

#ifdef WIN32
#include <windows.h> 
#endif // WIN32

enum class PortDetectionType { Auto, Explicit };
//enum class PhantomWingmanPortType {All, Explicit };

std::string DateTimeLogFilePrefixString;

// When was CraftServices started?
boost::posix_time::ptime CraftServicesStartTime;

// TODO: Make smart pointer of some kind
spdlog::logger * pConsoleAndAllLogger;

auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
std::shared_ptr<spdlog::sinks::basic_file_sink_mt> all_log_file_sink;

std::vector<CraftServices::PhantomTestCraft *> PhantomTestCrafts;

boost::asio::io_context ioContext;
std::vector<CraftServices::MSPFlightControllerAsync *> AsyncFlightControllerSessions;

uint32_t RefreshIntervalInMilliseconds;

// Are we in the process of shutting down?
static bool ShutdownInProgress = false;
// Has the app already completed a shutdown? (We don't want to do the relevant operations twice)
static bool ShutdownCompleted = false;

// Refresh timer for all flight controllers
boost::asio::steady_timer * pFlightControllerSteadyTimer;

// Global logging pattern

std::string SpdLogLoggingPattern = std::string("[%H:%M:%S.%e] [%^%l%$] %v");

std::string GetPortDetectionTypeAsString(PortDetectionType portDetectionType)
{
	switch (portDetectionType)
	{
		case PortDetectionType::Auto:
			return "Auto";
			break;
		case PortDetectionType::Explicit:
			return "Explicit";
			break;
		default:
			throw new exception("Unknown PortDetectionType");
			break;
	}
}

po::variables_map ProcessArguments(int argc, char** argv)
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Get help on command line usage")
		("ports", po::value<std::string>(), "Set ports to use. List of ports like 'com4,com20,com48' or 'auto' to find ports automatically. Auto can work, but its generally better to figure out your ports ahead of time and specify them explicitly.")
		("baud", po::value<uint32_t>(), "Set baud rate to use. 9600, 19200, 57600 (for example). See documentation for specific suggestions.")
		("refresh", po::value<uint32_t>(), "Set refresh interval in milliseconds to use. 250 refreshes 4 times a second, 50 refreshes 20 times per second, etc. Faster is generally better, up to the point you start dropping messages, or get excessive errors. It may take some experimentation to find a happy value. There is a default; omit this parameter to try it before adjusting on your own. See documentation for specific suggestions.")
		("stale", po::value<uint32_t>(), "Set stale interval in milliseconds to use. This is the length of time beyond which a received craft position will be considered stale, and no longer forwarded to other crafts. For example, if set to 4000, any received craft position older than 4 seconds will be treated as stale and will not be forwarded to other crafts. If set to 0, craft positions will never be treated as stale and will always be forwarded.")
		("phantomwingman", po::value<std::string>(), "This mode is intended for testing. If set, a phantom craft will be injected that appears at the given angle and distance from the craft. This allows solo testing in a kind of loopback arrangement, so you can judge round-trip connectivity quality and latency.\r\n\r\nSyntax:\r\n\r\n--phantomwingman [port|'all'],[angle],[distInMeters],\r\n[relativeAltDifferenceInMeters].\r\n\r\nFor example \"-- phantomwingman com20,90,100,-35\" will put a phantom wingman 100 meters to the immediate right (90 degrees) of, and and 35 meters below, the craft on com20. \" --phantomwingman all,180,50,10\" will put a phantom wingman 50 meters directly behind (180 degrees) and 10 meters above all the crafts, no matter what com port they are connected to.")
		("loglevel", po::value<std::string>(), "Set log level to use in output. Levels are trace, debug, info, warn, err, critical and off in level of priority. trace gives you everything, off gives you nothing, info is the in-between default.")
	    ("omitgpspos", "Omits exact GPS positions in logging output. Will include relative distances however.")
	    ("exitgpsloss", "if GPS position is no longer heard from a running flight controller, after a certain interval the program will exit, allowing it to be restarted via a batch file, etc. This is a somewhat desperate hack, intended to exist only until serial port restarting code works properly.");
		;

	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch (...)
	{
		throw CommandLineArgumentsException("Error parsing command line arguments");
	}
	po::notify(vm);

	// Exit immediately if all they want is help, or they gave no arguments
	if (vm.count("help") || argc == 1) 
	{
		// This is not directly compatible with the logger, so it stays with cout for the moment.
		//pConsoleAndAllLogger->info("{}", desc);
		cout << desc << "\n";
		DoCleanupAndShutdown(false);
		exit(0);
	}

	return vm;
}

std::string GetAllSerialPortNames(const vector<std::string> & serialPortNames)
{
	std::string allSerialPortNames;
	for (std::vector<std::string>::const_iterator it = serialPortNames.begin(); it < serialPortNames.end(); it++)
	{
		allSerialPortNames += ' ' + *it;
	}
	return allSerialPortNames;
}

uint32_t ProcessBaudArgument(const po::variables_map & argumentVariablesMap)
{
	uint32_t baudRateToUse = DEFAULT_BAUD_RATE;
	if (argumentVariablesMap.count("baud"))
	{
		baudRateToUse = argumentVariablesMap["baud"].as<std::uint32_t>();
	}
	pConsoleAndAllLogger->info("Baud Rate: {}", baudRateToUse);

	return baudRateToUse;
}

uint32_t ProcessRefreshArgument(const po::variables_map & argumentVariablesMap)
{
	uint32_t refreshToUse = DEFAULT_REFRESH_INTERVAL_IN_MILLISECONDS;
	if (argumentVariablesMap.count("refresh"))
	{
		refreshToUse = argumentVariablesMap["refresh"].as<std::uint32_t>();
	}
	pConsoleAndAllLogger->info("Refresh Rate: {} ms", refreshToUse);

	return refreshToUse;
}

uint32_t ProcessStaleArgument(const po::variables_map & argumentVariablesMap)
{
	uint32_t staleToUse = DEFAULT_STALE_INTERVAL_IN_MILLISECONDS;
	if (argumentVariablesMap.count("stale"))
	{
		staleToUse = argumentVariablesMap["stale"].as<std::uint32_t>();
	}
	pConsoleAndAllLogger->info("Stale Interval: {} ms", staleToUse);

	return staleToUse;
}

spdlog::level::level_enum ProcessLogLevelArgument(const po::variables_map & argumentVariablesMap)
{
	spdlog::level::level_enum logLevel = CRAFT_SERVICES_DEFAULT_LOG_LEVEL;
	if (argumentVariablesMap.count("loglevel"))
	{
		std::string logLevelAsString = boost::to_lower_copy(argumentVariablesMap["loglevel"].as<std::string>());
		if (logLevelAsString == "trace")
		{
			logLevel = spdlog::level::level_enum::trace;
		}
		else if (logLevelAsString == "debug")
		{
			logLevel = spdlog::level::level_enum::debug;
		}
		else if (logLevelAsString == "info")
		{
			logLevel = spdlog::level::level_enum::info;
		}
		else if (logLevelAsString == "warn")
		{
			logLevel = spdlog::level::level_enum::warn;
		}
		else if (logLevelAsString == "err")
		{
			logLevel = spdlog::level::level_enum::err;
		}
		else if (logLevelAsString == "critical")
		{
			logLevel = spdlog::level::level_enum::critical;
		}
		else if (logLevelAsString == "off")
		{
			logLevel = spdlog::level::level_enum::off;
		}
		pConsoleAndAllLogger->info("Log Level: {}", logLevelAsString);
	}

	return logLevel;
}

bool ProcessExitGpsLoss(const po::variables_map & argumentVariablesMap)
{
	bool exitOnGpsLoss = argumentVariablesMap.count("exitgpsloss") > 0;
	pConsoleAndAllLogger->info("Exit on GPS Loss: {}", exitOnGpsLoss);
	return exitOnGpsLoss;
}

bool ProcessOmitGpsPos(const po::variables_map & argumentVariablesMap)
{
	bool omitGpsPosition = argumentVariablesMap.count("omitgpspos") > 0;
	pConsoleAndAllLogger->info("Omit GPS Position: {}", omitGpsPosition);
	return (omitGpsPosition);
}


// Returns relevant PortDetectionType, with list of ports that should
// have monitoring attempted on them in portNamesToMonitor
PortDetectionType ProcessPortsArgument(const po::variables_map & argumentVariablesMap, std::vector<std::string> & portNamesToMonitor)
{
	// Default to auto mode for port names
	PortDetectionType portDetectionType = PortDetectionType::Auto;

	// User-provided com port names. They may or may not actually exist or be valid. Will
	// be ignored if in PortDetectionType::Auto mode.
	std::vector<std::string> userProvidedComPortNames;

	// If they gave a ports parameter, and it isn't 'auto', the default..
	if (argumentVariablesMap.count("ports") && !boost::iequals(argumentVariablesMap["ports"].as<std::string>(), "auto"))
	{
		// .. parse out com port names from user input
		stringstream ss(argumentVariablesMap["ports"].as<std::string>());

		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			userProvidedComPortNames.push_back(substr);
		}
		portDetectionType = PortDetectionType::Explicit;
	}

	std::vector<std::string> currentScannedValidSerialPortNames = SerialPortEnumerator::GetSerialPortNames();
	// The list of port names we will actually attempt to connect to
	portNamesToMonitor = portDetectionType == PortDetectionType::Auto ? currentScannedValidSerialPortNames : userProvidedComPortNames;

	pConsoleAndAllLogger->info("Port detection type: {}", GetPortDetectionTypeAsString(portDetectionType));
	pConsoleAndAllLogger->info("Ports:{}", GetAllSerialPortNames(portNamesToMonitor));

	return portDetectionType;
}


std::vector<CraftServices::PhantomWingman *> ProcessPhantomWingmanArguments(const po::variables_map & argumentVariablesMap)
{
	// Wingman to return 
	std::vector<CraftServices::PhantomWingman *> phantomWingmanVector;

	// If they gave one or more phantomwingman arguments...
	if (argumentVariablesMap.count("phantomwingman") > 0)
	{
		std::string phantomWingmanArgumentValue = argumentVariablesMap["phantomwingman"].as<std::string>();

		// User-provided comma delimited arguments
		std::vector<std::string> commaDelimitedSubArguments;

		// Parse out comma delimited arguments
		stringstream argStringStream(phantomWingmanArgumentValue);

		const std::string invalidArgumentsForPhantomWingmanSubArguments = "Invalid sub-arguments for phantomwingman argument.";

		while (argStringStream.good())
		{
			string substr;
			getline(argStringStream, substr, ',');
			commaDelimitedSubArguments.push_back(substr);
		}
		
		if (commaDelimitedSubArguments.size() != 4)
		{
			throw new CommandLineArgumentsException(invalidArgumentsForPhantomWingmanSubArguments);
		}

		try
		{
			std::string portName = commaDelimitedSubArguments[0];
			double angle = boost::lexical_cast<double>(commaDelimitedSubArguments[1]);
			double distanceInMeters = boost::lexical_cast<double>(commaDelimitedSubArguments[2]);
			double relativeAltitudeDifferenceInMeters = boost::lexical_cast<double>(commaDelimitedSubArguments[3]);

			CraftServices::PhantomWingman * pNewPhantomWingMan = new CraftServices::PhantomWingman(portName, angle, distanceInMeters, relativeAltitudeDifferenceInMeters);
			phantomWingmanVector.push_back(pNewPhantomWingMan);

			pConsoleAndAllLogger->info("Phantom Wingman: {}", pNewPhantomWingMan->GetParametersAsString());
		}
		catch (...)
		{
			throw new CommandLineArgumentsException(invalidArgumentsForPhantomWingmanSubArguments);
		}
		return phantomWingmanVector;
	}

	return phantomWingmanVector;
}

void AddFixedCraft(std::vector<CraftServices::PhantomTestCraft *> * pPhantomTestCraftVector,
	std::string craftName,
	uint32_t UID_0,
	uint32_t UID_1,
	uint32_t UID_2,
	CraftServices::GeoSpatialPoint latLongGeoPoint,
	uint16_t altitude,
	uint16_t groundCourseInDecidegrees)
{
	// Hardwired, single craft
	CraftServices::PhantomTestCraftFixed * pFixedCraft = new CraftServices::PhantomTestCraftFixed(craftName);
	pFixedCraft->UID_0 = UID_0;
	pFixedCraft->UID_1 = UID_1;
	pFixedCraft->UID_2 = UID_2;
	pFixedCraft->SetFixedPositionWithGeoSpatialPoint(latLongGeoPoint, altitude, groundCourseInDecidegrees);
	pPhantomTestCraftVector->push_back(pFixedCraft);
}

void AddTheUsualSpot(std::vector<CraftServices::PhantomTestCraft *> * pPhantomTestCraftVector)
{
	// Hardwired, single craft
	CraftServices::PhantomTestCraftFixed * pFixedCraft = new CraftServices::PhantomTestCraftFixed("FakeCofpv_01");
	// Hacking FakeCraft further to have UIDs that are easily debuggable on other side
	pFixedCraft->UID_0 = 777;
	pFixedCraft->UID_1 = 888;
	pFixedCraft->UID_2 = 999;
	CraftServices::GeoSpatialPoint theUsualSpot(39.490756, -105.081577);
	// put in a fixed position slightly offset from home location normally seen with FC in windows
	pFixedCraft->SetFixedPositionWithGeoSpatialPoint(theUsualSpot, 5, 450);

	pPhantomTestCraftVector->push_back(pFixedCraft);
}


void AddFourCornersOfPlatte(std::vector<CraftServices::PhantomTestCraft *> * pPhantomTestCraftVector)
{
	pConsoleAndAllLogger->trace("Phantom Craft: AddFourCornersOfPlatte");

	AddFixedCraft(pPhantomTestCraftVector, "NE Corner", 2234, 2278, 2299, CraftServices::GeoSpatialPoint(39.492384, -105.083445), 100, 2250);
	AddFixedCraft(pPhantomTestCraftVector, "SE Corner", 3334, 3378, 3399, CraftServices::GeoSpatialPoint(39.487074, -105.082557), 100, 3150);
	AddFixedCraft(pPhantomTestCraftVector, "SW Corner", 4434, 4478, 4499, CraftServices::GeoSpatialPoint(39.487409, -105.091027), 100, 450);
	AddFixedCraft(pPhantomTestCraftVector, "Drain",      5534, 5578, 5599, CraftServices::GeoSpatialPoint(39.490240, -105.089252), 100, 0);
}

// Create PhantomTestCraft requested on the command line, if any.
// Caller is responsible for their ultimate destruction.
std::vector<CraftServices::PhantomTestCraft *> ProcessPhantomTestCraftArguments(po::variables_map & argumentVariablesMap)
{
	pConsoleAndAllLogger->trace("ProcessPhantomTestCraftArguments()");

	// Vector of all TestCraft
	std::vector<CraftServices::PhantomTestCraft *> phantomTestCraftVector;	

	// Add Phantom Wingman
	std::vector <CraftServices::PhantomWingman *> phantomWingmanVector = ProcessPhantomWingmanArguments(argumentVariablesMap);
	phantomTestCraftVector.insert(phantomTestCraftVector.end(), phantomWingmanVector.begin(), phantomWingmanVector.end());

	// For now, fixed positions are only compiled in, and not yet configurable from command line.
	// Here are examples of how to use them for the moment.
	//AddTheUsualSpot(&phantomTestCraftVector);
	//AddFourCornersOfPlatte(&phantomTestCraftVector);
	
	return phantomTestCraftVector;
}

void DoAsyncMonitoring(std::vector<std::string> portNamesToMonitor, 
					   uint32_t baudRateForAllPorts, 
					   uint32_t refreshIntervalInMillisecondsForAllPorts,
					   uint32_t staleIntervalInMilliseconds,
					   spdlog::level::level_enum spdLogLevel,
					   std::vector<CraftServices::PhantomTestCraft *> phantomTestCraft,
					   bool exitOnGpsLoss,
					   bool omitGpsPos)
{
	pConsoleAndAllLogger->trace("DoAsyncMonitoring()");

	// Go through each port and create an MSPFlightControllerAsync.
	// These will start off in a closed state.
	for (std::vector<std::string>::iterator comPortIt = portNamesToMonitor.begin(); comPortIt < portNamesToMonitor.end(); comPortIt++)
	{
		std::string currentSerialPortName = *comPortIt;
		CraftServices::MSPFlightControllerAsync * pCurrentFlightController = 
			new CraftServices::MSPFlightControllerAsync(&ioContext, currentSerialPortName, baudRateForAllPorts, refreshIntervalInMillisecondsForAllPorts, staleIntervalInMilliseconds, 
														console_sink, all_log_file_sink, spdLogLevel, SpdLogLoggingPattern, DateTimeLogFilePrefixString,
														&AsyncFlightControllerSessions, &phantomTestCraft, exitOnGpsLoss, omitGpsPos);
		AsyncFlightControllerSessions.push_back(pCurrentFlightController);

		pConsoleAndAllLogger->info("Created MSPFlightControllerAsync to monitor {}.", currentSerialPortName);
	}

	StartFlightControllerRefreshTimerFiring(refreshIntervalInMillisecondsForAllPorts);

	pConsoleAndAllLogger->trace("IO Context running...");
	ioContext.run();
	//pConsoleAndAllLogger->trace("IO Context exited...");
}

void StartFlightControllerRefreshTimerFiring(uint32_t refreshIntervalInMilliseconds)
{
	pConsoleAndAllLogger->trace("StartFlightControllerRefreshTimerFiring() - {} ms", refreshIntervalInMilliseconds);

	// Create main Flight Controller refresh timer
	pFlightControllerSteadyTimer = new boost::asio::steady_timer(ioContext, boost::asio::chrono::milliseconds(refreshIntervalInMilliseconds));

	// Start timer firing at regular interval
	boost::system::error_code errorCode;
	pFlightControllerSteadyTimer->async_wait([&](const boost::system::error_code & error) { RefreshFlightControllers(errorCode); });
}

static int currentFlightControllerIndex = 0;

// Called at a regular interval. Updates each flight controller in turn,
// picking a different flight controller to update each time it is called
void RefreshFlightControllers(const boost::system::error_code & error)
{
	if (IsShutdownInProgressOrComplete())
	{
		return;
	}

	if (error == boost::asio::error::operation_aborted)
	{
		// TODO - more of this checking elsewhere, if relevant!
		pConsoleAndAllLogger->debug("RefreshFlightControllers() - exiting because error code == operation aborted");
		return;
	}

	pConsoleAndAllLogger->trace("RefreshFlightControllers()");
	
	size_t flightControllerCount = AsyncFlightControllerSessions.size();
	if (flightControllerCount == 0)
	{
		pConsoleAndAllLogger->trace("No flight controllers to refresh.");
		return;
	}

	// Update this flight controller
	AsyncFlightControllerSessions[currentFlightControllerIndex]->RefreshFlightControllerState();

	// Move to the next index, so that the next time we trip this function, we update the next FlightController.
	currentFlightControllerIndex++;
	if (currentFlightControllerIndex >= flightControllerCount)
	{
		// Wrap back to first
		currentFlightControllerIndex = 0;
		// TODO:
		// At this point, once all have been serviced, we could sort the Flight Controllers by distance and service them
		// such that the crafts nearest each other hear about each other first. With about .5 second latency per craft, and multiple
		// (3 or more) crafts, this could actually make a difference in performance. -- SLG
	}

	if (!IsShutdownInProgressOrComplete())
	{
		//ScheduleNextFlightControllerTimerFiring();

	    // Reschedule the timer to fire again at interval
		pFlightControllerSteadyTimer->expires_at(pFlightControllerSteadyTimer->expires_at() + boost::asio::chrono::milliseconds(RefreshIntervalInMilliseconds));
		pFlightControllerSteadyTimer->async_wait([&](const boost::system::error_code & error) { RefreshFlightControllers(error); });
	}
}

void ScheduleNextFlightControllerTimerToFireImmediately()
{
	pConsoleAndAllLogger->trace("ScheduleNextFlightControllerTimerToFireImmediately()");

	// Reschedule the timer to fire immediately. (Current timer will be cancelled, and called with error code; we ignore that call.)
	pFlightControllerSteadyTimer->expires_after(boost::asio::chrono::milliseconds(0));
	pFlightControllerSteadyTimer->async_wait([&](const boost::system::error_code & error) { RefreshFlightControllers(error); });
}


void CleanUpPhantomTestCrafts(std::vector<CraftServices::PhantomTestCraft *> & phantomTestCraftVect)
{
	if (phantomTestCraftVect.size() > 0)
	{
		pConsoleAndAllLogger->debug("CleanUpPhantomTestCrafts(): Cleaning up {} phantom crafts..", phantomTestCraftVect.size());

		for (std::vector<CraftServices::PhantomTestCraft *>::iterator it = phantomTestCraftVect.begin(); it < phantomTestCraftVect.end(); it++)
		{
			CraftServices::PhantomTestCraft * pCurrentPhantomTestCraft = (*it);
			delete pCurrentPhantomTestCraft;
			pCurrentPhantomTestCraft = NULL;
		}
		phantomTestCraftVect.clear();
	}
}

void CleanupFlightControllers(std::vector<CraftServices::MSPFlightControllerAsync *> & asyncFlightControllerSessionsVect)
{
	if (asyncFlightControllerSessionsVect.size() > 0)
	{
		pConsoleAndAllLogger->debug("CleanupFlightControllers(): Cleaning up {} flight controllers..", asyncFlightControllerSessionsVect.size());

		for (std::vector<CraftServices::MSPFlightControllerAsync *>::iterator it = asyncFlightControllerSessionsVect.begin(); it < asyncFlightControllerSessionsVect.end(); it++)
		{
			CraftServices::MSPFlightControllerAsync * pCurrentFlightController = (*it);
			pCurrentFlightController->pSerialPort->close();

			delete pCurrentFlightController;
			pCurrentFlightController = NULL;
		}
		asyncFlightControllerSessionsVect.clear();
	}
}

std::wstring stringToWideString(const std::string & str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string wideStringToString(const std::wstring & wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::string GetDateTimeAsString(boost::posix_time::ptime dateTime)
{
	using namespace boost::posix_time;
	static std::locale loc(std::wcout.getloc(), new wtime_facet(L"%Y-%m-%d__%H_%M_%S"));

	std::basic_stringstream<wchar_t> wss;
	wss.imbue(loc);
	wss << dateTime;

	std::string dateTimeString = wideStringToString(wss.str());
	return dateTimeString;
}

void SetUpBaseLogger(spdlog::level::level_enum spdLogLevel)
{
	// All log file sets are preceeded with a consistent date/time string. Each run
	// gets a new such prefix.
	using namespace boost::posix_time;
	ptime now = second_clock::universal_time();
	DateTimeLogFilePrefixString = GetDateTimeAsString(now);

	console_sink->set_level(spdLogLevel);
	console_sink->set_pattern(SpdLogLoggingPattern);

	// TODO - Date and time as part of filename. And should be consistent across file sets.
	std::string allLogFilename = DateTimeLogFilePrefixString + std::string("--CraftServices_AllLog.txt");
	all_log_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(allLogFilename, true);
	all_log_file_sink->set_level(spdLogLevel);
	all_log_file_sink->set_pattern(SpdLogLoggingPattern);

	pConsoleAndAllLogger = new spdlog::logger("ConsoleAndAll", { console_sink, all_log_file_sink });
	pConsoleAndAllLogger->set_level(spdLogLevel);

	/*
	// HACKING
	// https://github.com/gabime/spdlog/issues/674
	auto other_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/SOME_COMPORT.txt", true);
	other_file_sink->set_level(spdlog::level::trace);
	
	auto & strippedSinks = const_cast<std::vector<spdlog::sink_ptr>&>(logger.sinks());
	strippedSinks.emplace_back(other_file_sink);
	*/
}

void ChangeLogLevelForAllLoggers(spdlog::level::level_enum spdLogLevel)
{
	console_sink->set_level(spdLogLevel);
	all_log_file_sink->set_level(spdLogLevel);
	pConsoleAndAllLogger->set_level(spdLogLevel);
}

std::string GetCraftServicesVersionString()
{
	std::string compileDate = std::string(__DATE__);
	return "CraftServices v0.62 - " + compileDate;
}

static boost::mutex ShutdownMutex;

void DoCleanupAndShutdown(bool playBeepsAndShowExitLogging)
{
	ShutdownMutex.lock();
	if (!ShutdownInProgress && !ShutdownCompleted)
	{
		ShutdownInProgress = true;
		// Try calling this first
		ioContext.stop();
		if (playBeepsAndShowExitLogging)
		{
			OutputCraftServicesTotalRuntime();
		}
		CleanupFlightControllers(AsyncFlightControllerSessions);
		CleanUpPhantomTestCrafts(PhantomTestCrafts);
		
		delete pFlightControllerSteadyTimer;
		pFlightControllerSteadyTimer = NULL;

		if (pConsoleAndAllLogger != NULL)
		{
			if (playBeepsAndShowExitLogging)
			{
				pConsoleAndAllLogger->info("Exiting...");
				pConsoleAndAllLogger->flush();
			}
			delete pConsoleAndAllLogger;
			pConsoleAndAllLogger = NULL;
		}
		if (playBeepsAndShowExitLogging)
		{
			#ifdef WIN32
			Beep(1000, 200);
			Beep(1000, 200);
			Beep(1000, 200);
			#endif // WIN32
		}

		ShutdownInProgress = false;
		ShutdownCompleted = true;
	}
	ShutdownMutex.unlock();
}

bool IsShutdownInProgressOrComplete()
{
	return ShutdownInProgress || ShutdownCompleted;
}

void OutputCraftServicesTotalRuntime()
{
	// Compute total run time
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::time_duration timeDiff = (now - CraftServicesStartTime);
	
	if (pConsoleAndAllLogger != NULL)
	{
		pConsoleAndAllLogger->info("CraftServices elapsed run time: {} seconds ({} minutes)", timeDiff.total_seconds(), timeDiff.total_seconds() / 60);
	}
}

#ifdef WIN32

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		//Beep(750, 300);
		// Dirty and bad. Cleanup is likely mangled and dangling. Ugh, need to use exceptions / signalling and do this on main thread.
		DoCleanupAndShutdown();
		//exit(0);
		return TRUE;

/*
		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		Beep(600, 200);
		printf("Ctrl-Close event\n\n");
		// Dirty and bad. Cleanup is likely mangled and dangling. Ugh, need to use exceptions / signalling and do this on main thread.
		DoCleanupAndShutdown();
		exit(0);
		return TRUE;

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		Beep(900, 200);
		printf("Ctrl-Break event\n\n");
		return TRUE;

	case CTRL_LOGOFF_EVENT:
		Beep(1000, 200);
		printf("Ctrl-Logoff event\n\n");
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		Beep(750, 500);
		printf("Ctrl-Shutdown event\n\n");
		return FALSE;
*/
	default:
		return true;
	}
}

#endif // WIN32


int main(int argc, char** argv) 
{
	CraftServicesStartTime = boost::posix_time::microsec_clock::universal_time();

#ifdef WIN32
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
#endif // WIN32

	try
	{
		// Set up logging using default log level
		SetUpBaseLogger(CRAFT_SERVICES_DEFAULT_LOG_LEVEL);

		pConsoleAndAllLogger->info("{}", GetCraftServicesVersionString());

		// Get ready to process command line arguments
		po::variables_map argumentVariablesMap = ProcessArguments(argc, argv);

		// Process log level
		spdlog::level::level_enum spdLogLevel = ProcessLogLevelArgument(argumentVariablesMap);
		// Change Logging level (may do nothing depending on what user input was)
		ChangeLogLevelForAllLoggers(spdLogLevel);

		// Determine which ports to monitor
		std::vector<std::string> portNamesToMonitor;
		PortDetectionType portDetectionType = ProcessPortsArgument(argumentVariablesMap, portNamesToMonitor);
		// Parse baud rate
		uint32_t baudRateForAllPorts = ProcessBaudArgument(argumentVariablesMap);
		// Parse various refresh rates
		RefreshIntervalInMilliseconds = ProcessRefreshArgument(argumentVariablesMap);
		uint32_t staleIntervalInMilliseconds = ProcessStaleArgument(argumentVariablesMap);
		// Parse any instructions about phantom, test Craft to inject into telemetry.		
		PhantomTestCrafts = ProcessPhantomTestCraftArguments(argumentVariablesMap);
		// Should we exit on GPS loss? This is a brutal and desperate hack.
		bool exitOnGpsLoss = ProcessExitGpsLoss(argumentVariablesMap);
		bool omitGpsPos = ProcessOmitGpsPos(argumentVariablesMap);

		// Loop and repeatedly exchange messages between various crafts
		DoAsyncMonitoring(portNamesToMonitor, baudRateForAllPorts, RefreshIntervalInMilliseconds, staleIntervalInMilliseconds, spdLogLevel, PhantomTestCrafts, exitOnGpsLoss, omitGpsPos);

		DoCleanupAndShutdown();
		return EXIT_SUCCESS;
	}
	catch (NotImplementedException & notImplementedException)
	{		
		pConsoleAndAllLogger->error("NotImplemented exception: {}", notImplementedException.what());
	}
	catch (CommandLineArgumentsException & commandLineArgumentsException)
	{
		pConsoleAndAllLogger->error("CommandLineArguments exception: {}", commandLineArgumentsException.what());
	}
	catch(...)
	{
		pConsoleAndAllLogger->error("Unhandled exception, exiting...");
	}

	// Abnormal exit, indicating failure
	DoCleanupAndShutdown();
	return EXIT_FAILURE;
}









