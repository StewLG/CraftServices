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

#include <iostream>
#include <boost/asio/serial_port.hpp> 
#include <boost/asio.hpp> 
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "MSPFlightControllerAsync.hpp"
#include "SerialPortDefaults.hpp"
#include "MspMessageAsyncs.hpp"
#include "OtherCraftPositionMessage.hpp"
#include "GeoSpatialUtil.hpp"
#include "crc.hpp"
#include "CraftServices.hpp"

namespace CraftServices
{
	std::string OverallPortStateAsString(CraftServices::OverallPortState overallPortState)
	{
		switch (overallPortState)
		{
			case CraftServices::OverallPortState::PortClosed:
				return "PortClosed";
				break;
			case CraftServices::OverallPortState::PortOpenFailed:
				return "PortOpenFailed";
				break;
			case CraftServices::OverallPortState::PortOpened:
				return "PortOpened";
				break;
			case CraftServices::OverallPortState::SessionRunning:
				return "SessionRunning";
				break;
			default:
				throw NotImplementedException("Unknown OverallPortState");
				break;
		}
	}

	// Is this Port in a Closed or Failed state?
	bool IsClosedOrFailedPort(CraftServices::OverallPortState overallPortState)
	{
		return (overallPortState == OverallPortState::PortClosed ||
			    overallPortState == OverallPortState::PortOpenFailed);
	}

	// Constructor
	MSPFlightControllerAsync::MSPFlightControllerAsync(boost::asio::io_context * pIo_context,
													   std::string serialPortName,
													   uint32_t baudRate,
													   uint32_t refreshTimerIntervalInMilliseconds,
													   uint32_t staleIntervalInMilliseconds,
													   std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pConsoleSink,
													   std::shared_ptr<spdlog::sinks::basic_file_sink_mt> pAllSink,
													   spdlog::level::level_enum spdLogLevel,
													   const std::string & loggingPattern,
													   std::string dateTimeLogFilePrefixString,
													   std::vector<CraftServices::MSPFlightControllerAsync *> * pAllFlightControllers,
													   std::vector<CraftServices::PhantomTestCraft *> * pPhantomTestCraft,
													   bool exitOnGpsLoss,
													   bool omitGpsPos) : SerialPortName(serialPortName)
	{
		InProcessOfShuttingDownThisFlightController = false;
		// Non-owning pointer
		pIoContext = pIo_context;
		// Create the serial port, but do not open it yet
		pSerialPort = new boost::asio::serial_port(*pIoContext);
		// Baud rate to use on the port
		BaudRate = baudRate;
		// Refresh interval
		RefreshTimerIntervalInMilliseconds = refreshTimerIntervalInMilliseconds;
		// Stale interval
		StaleIntervalInMilliseconds = staleIntervalInMilliseconds;
		// We start in a closed state
		PortState = CraftServices::OverallPortState::PortClosed;
		// We keep track of the parent container of Flight Controllers so we can talk to our peers
		pAllMspFlightControllers = pAllFlightControllers;
		// Any Phantom craft
		pPhantomCraft = pPhantomTestCraft;
		// Have we marked when we first tried opening the port?
		HasMarkedPortStartupTime = false;
		// GPS position has not yet been set
		CurrentPositionEverBeenSet = false;
		// Should we exit when we lose GPS feedback from flight controller?
		ExitOnGpsLoss = exitOnGpsLoss;
		// Should we obscure/omit exact GPS positions from logging?
		OmitGpsPos = omitGpsPos;

		SetUpLogger(pConsoleSink, pAllSink, spdLogLevel, loggingPattern, dateTimeLogFilePrefixString);
		// This refresh period may well be the single most important parameter, at least as I type this. A bad one will
		// give terrible results.
		pSerialPortLogger->debug("{}: Refresh Interval {} milliseconds.", GetPortAndCraftNamePrefix(), refreshTimerIntervalInMilliseconds);
	}

	// Destructor
	MSPFlightControllerAsync::~MSPFlightControllerAsync()
	{
		InProcessOfShuttingDownThisFlightController = true;

		// TODO: Show total runtime per-port here?
		pSerialPortLogger->info("{}: shutting down.", GetPortAndCraftNamePrefix());

		//pSerialPort->cancel();
		//pSerialPort->close();
		delete pSerialPort;
		pSerialPort = NULL;

		delete pSerialPortLogger;
		pSerialPortLogger = NULL;
	}

	// Set up Logging
	void MSPFlightControllerAsync::SetUpLogger(std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pConsoleSink,
											   std::shared_ptr<spdlog::sinks::basic_file_sink_mt> pAllSink,
											   spdlog::level::level_enum spdLogLevel,
											   const std::string & loggingPattern,
		                                       std::string dateTimeLogFilePrefixString)
	{
		// TODO - Date and time as part of filename. And should be consistent across file sets.
		std::string serialPortLoggerFilename = dateTimeLogFilePrefixString + "--CraftServices_" + SerialPortName + ".txt";
		auto serial_port_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(serialPortLoggerFilename, true);
		serial_port_file_sink->set_level(spdLogLevel);
		serial_port_file_sink->set_pattern(loggingPattern);

		pSerialPortLogger = new spdlog::logger("SerialPort_" + SerialPortName, { serial_port_file_sink, pAllSink, pConsoleSink });
		pSerialPortLogger->set_level(spdLogLevel);
	}

	void MSPFlightControllerAsync::OpenPortAndStartSession()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: OpenPortAndStartSession()", SerialPortName);

		// Mark the time we first start trying to open the port
		if (!HasMarkedPortStartupTime)
		{
			InitialPortStartupTime = boost::posix_time::microsec_clock::universal_time();
			HasMarkedPortStartupTime = true;
		}

		try
		{
			pSerialPort->open(SerialPortName);
		}
		catch (const boost::system::system_error &e)
		{
			pSerialPortLogger->error("{}: could not connect", SerialPortName);
			pSerialPortLogger->error("{}: Port open Exception: {}", SerialPortName, e.what());

			PortState = OverallPortState::PortOpenFailed;
			return;
		}

		// TODO: Try other (higher?) baud rates. Make configurable from command line.
		pSerialPort->set_option(boost::asio::serial_port::baud_rate(BaudRate));
		pSerialPort->set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none));
		pSerialPort->set_option(boost::asio::serial_port::character_size(boost::asio::serial_port::character_size(8)));
		pSerialPort->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));

		// clear buffer for new session
		ClearSerialBuffer();

		// Mark as opened
		PortState = CraftServices::OverallPortState::PortOpened;

		// Start trying to read
		StartReadMessageReceiveLoopForFlightController();

		pSerialPortLogger->info("Connected to: {} at {} baud - {}", SerialPortName, BaudRate, OverallPortStateAsString(PortState));
	}

	// Request the initial information about this flight controller -- Type, API version number, UID, etc.
	// These are properties that are immutable for the life of the connection.
	void MSPFlightControllerAsync::RequestInitialInformationFromFlightController()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: RequestInitialInformationFromFlightController()", SerialPortName);

		// Here we only ask about things we don't already have
		if (!MspFcInfo.HasFcVariantString)
		{
			RequestFcVariant();
		}
		if (!MspFcInfo.HasUID)
		{
			RequestUid();
		}
		if (!MspFcInfo.HasApiVersion)
		{
			RequestApi();
		}
		if (!MspFcInfo.HasCraftName)
		{
			RequestCraftName();
		}
		// We only attempt to get this setting if we think the FC is going to suport the Craft Services messages.
		if (MspFcInfo.HasApiVersion && !MspFcInfo.HasOtherCraftPositionSetting)
		{
			if (MspFcInfo.IsRunningMspApiWithCraftServicesMessagesAvailable())
			{
				pSerialPortLogger->info("{}: MSP API {} supports Craft Services. Requesting Craft Services setting...", GetPortAndCraftNamePrefix(), MspFcInfo.GetMspAPIVersionString());
				RequestOtherCraftPositionSetting();
			}
			else
			{
				pSerialPortLogger->info("{}: MSP API {} does not support Craft Services.", GetPortAndCraftNamePrefix(), MspFcInfo.GetMspAPIVersionString());
			}
		}

		// TODO: Craft type (quad, fixed wing, tricopter...)	
	}

	void MSPFlightControllerAsync::RefreshFlightControllerState()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: RefreshFlightControllerState() - {}", GetPortAndCraftNamePrefix(), OverallPortStateAsString(PortState));

		RestartPortIfNecessary();
		if (IsThisFlightControllerShuttingDown())
		{
			// Set error_code here??
			return;
		}

		switch (PortState)
		{
			case CraftServices::OverallPortState::PortClosed:
			case CraftServices::OverallPortState::PortOpenFailed:
			{
				// Aggressively try to open ports that aren't already open.
				// TODO: A setting to have it give up after some user-defined interval.
				OpenPortAndStartSession();
				break;
			}
			case CraftServices::OverallPortState::PortOpened:
			{
				// Once the port is opened, we need to figure out some baseline information
				// about the Flight Controller we are connected to. This is a requirement 
				// for proper operation, so we will retry indefinitely to be sure we get all these.
				if (!MspFcInfo.HaveAllRequiredValuesBeenSet())
				{
					RequestInitialInformationFromFlightController();
				}
				else
				{
					// Time to start normal session operations
					PortState = CraftServices::OverallPortState::SessionRunning;
				}
				break;
			}
			case CraftServices::OverallPortState::SessionRunning:
			{
				// Regular operations once we have baseline info
				DoSessionRunningOperations();
				break;
			}
			default:
			{
				std::string unknownMessageTypeErrorString = "RefreshFlightControllerState - Unhandled OverallPortState -- (Unknown)";
				pSerialPortLogger->error("{}: {}", GetPortAndCraftNamePrefix(), unknownMessageTypeErrorString);

				throw NotImplementedException(unknownMessageTypeErrorString);
				break;
			}
		}

		// TODO: More real work here
	}

	bool MSPFlightControllerAsync::IsThisFlightControllerShuttingDown()
	{
		// This is redundant and paranoid; it really needs to be cleaned up and consolidated.
		return (IsShutdownInProgressOrComplete() || InProcessOfShuttingDownThisFlightController);
	}

	
	// I've been having problems with restarting the serial port on Windows using Boost. Below 
	// are experimental attempts at dealing with the problem.

	// A softer attempt at closing serial port.
	void MSPFlightControllerAsync::ResetPortSoftish()
	{
		try
		{
			pSerialPort->cancel();
			ClearSerialBuffer();
			pSerialPort->close();
			PortState = CraftServices::OverallPortState::PortClosed;
			MspFcInfo.ResetStateValues();
			CurrentPositionEverBeenSet = false;
			HasMarkedPortStartupTime = false;
		}
		catch (const boost::system::system_error &e)
		{
			pSerialPortLogger->error("{}: Could not close", GetPortAndCraftNamePrefix());
			pSerialPortLogger->error("{}: Port close Exception: {}", GetPortAndCraftNamePrefix(), e.what());

			PortState = OverallPortState::PortOpened;
			return;
		}
	}

	// A harder attempt at closing the serial port.
	void MSPFlightControllerAsync::ResetPortHard()
	{
		ResetPortSoftish();
		delete pSerialPort;

		// Suggested here: https://stackoverflow.com/questions/541062/boostasioserial-port-reading-after-reconnecting-device#541097
		pIoContext->stop();
		pIoContext->reset();

		// Create a new serial port object, but do not open it yet
		pSerialPort = new boost::asio::serial_port(*pIoContext);		
	}

	// If we need to (i.e. lack of response from remote Flight Controller), reboot the port and restart the session.
	// 
	// Note: It may be that the need to do this can better be addressed with a more direct and less drastic solution. I'm writing it
	// because I find in dry ground testing that I stop hearing back from one or both of the flight controllers after 
	// some time of running, but if I restart communications it works. So, consider this a start at the problem, not a final
	// or confident solution. -- SLG
	void MSPFlightControllerAsync::RestartPortIfNecessary()
	{
		pSerialPortLogger->trace("{}: RestartPortIfNecessary()", GetPortAndCraftNamePrefix());

		// TODO: Make configurable
		const int RESTART_PORT_TIMEOUT_IN_MILLISECONDS = 15000;

		// Timeout can happen either when GPS was heard at least once, OR when it was never heard at all
		boost::posix_time::ptime comparisonTime;
		if (CurrentPositionEverBeenSet)
		{
			comparisonTime = CurrentPositionRetrievalTime;
		}
		else if (HasMarkedPortStartupTime)
		{
			comparisonTime = InitialPortStartupTime;
		}
		else
		{
			// Too early/not in the right state to check for port restart
			pSerialPortLogger->trace("{}: Too early to check for Port Restart timeout.", GetPortAndCraftNamePrefix());
			return;
		}

		// How stale is the last position in milliseconds?
		boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
		boost::posix_time::time_duration timeDiff = (now - comparisonTime);
		uint32_t elapsedMilliseconds = timeDiff.total_milliseconds();
		if (elapsedMilliseconds > RESTART_PORT_TIMEOUT_IN_MILLISECONDS)
		{
			auto elapsedSeconds = timeDiff.total_seconds();
			pSerialPortLogger->error("{}: has not heard GPS position response in {} ms ({} sec).", GetPortAndCraftNamePrefix(), elapsedMilliseconds, elapsedSeconds);

			if (ExitOnGpsLoss)
			{
				pSerialPortLogger->error("{}: Failed to get GPS location. Exiting.", GetPortAndCraftNamePrefix());
				//throw exception("GPS loss, exiting...");
				// Dirty and bad. Cleanup is likely mangled and dangling. Ugh, need to use exceptions / signalling and do this on main thread.
				DoCleanupAndShutdown();
				return;
			}

			// Attempting a soft restart at first, but it may be this turns out to be futile, and we always need to do a hard reset.
			pSerialPortLogger->error("{}: restarting port.", GetPortAndCraftNamePrefix());
			ResetPortSoftish();
		}
	}

	// Operations done regularly once a session is up and running normally
	void MSPFlightControllerAsync::DoSessionRunningOperations()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: DoSessionRunningOperations() - {}", GetPortAndCraftNamePrefix(), OverallPortStateAsString(PortState));

		// First, tell this Session (if it is participating) about other, real craft
		SendNoticesAboutOtherCrafts();

		// Next, tell this Session (if it is participating) about any fake, simulated craft 
		SendNoticesAboutPhantomCrafts();

		// Request updated position for the Craft associated with this Session
		RequestRawGPSPosition();
	}

	// Send notices about other Craft to the Craft associated with this Session
	void MSPFlightControllerAsync::SendNoticesAboutOtherCrafts()
	{
		if (IsShutdownInProgressOrComplete())
		{
			return;
		}

		pSerialPortLogger->trace("{}: SendNoticesAboutOtherCrafts()", GetPortAndCraftNamePrefix());

		// If this Flight Controller wants other Craft position updates...
		if (MspFcInfo.ShouldBeSentOtherCraftPositionUpdates)
		{
			pSerialPortLogger->trace("{}: SendNoticesAboutOtherCraft() - {}", GetPortAndCraftNamePrefix(), OverallPortStateAsString(PortState));

			// TODO, possibly: Sort by distance, and update positions of closest crafts first. This would be working
			// under the assumption that latency becomes more important the closer you are to the other craft. Here's 
			// the rough idea:
			//
			// Given 3 crafts A, B, C, there are 3 distances between them:
			// 
			// A <--> B = Distance DAB = 250 meters
			// B <--> C = Distance DBC = 15 meters
			// C <--> A = Distance DCA = 25 meters
			//
			// To service the closest craft first, we'd service in the following orders for each craft:
			//
			//  1. Servicing Craft C:  B, A  (15, 25 meters)  (Distance sum = 15 + 25  = 40)
			//  2. Servicing Craft B:  C, A  (15, 250 meters) (Distance sum = 15 + 250 = 265)
			//  3. Servicing Craft A:  C, B  (25, 250 meters) (Distance sum = 25 + 250 = 275)
			// 
			// There are two orders here: The overall order of crafts to service (in this case A, C, B),
			// and then the order to service PER CRAFT. In both cases we favor the near craft.
			// This still assumes we are servicing one entire craft at a time. 
			// 
			// To determine the overall order (1,2,3 above), you sort by the distance sum.
			//
			// If we could change between crafts while we were doing servicing, we could take this farther and
			// send out the closest pairs, one side at a time, like so:
			// 
			//  1. Service C, send B (DBC = 15 meters)
			//  2. Service B, send C (DBC = 15 meters)
			//  3. Service A, send C (DCA = 25 meters)
			//  4. Service C, send A (DCA = 25 meters)
			//  5. Service A, send B (DAB = 250 meters)
			//  4. Service B, send A (DAB = 250 meters)
			//
			// This will take loop restructuring though. Ok, enough pre-optimization/hyperoptimization.		
			// -- SLG

			// Go through all the Flight Controllers known to Craft Services
			for (std::vector<CraftServices::MSPFlightControllerAsync *>::iterator fcIter = pAllMspFlightControllers->begin(); fcIter < pAllMspFlightControllers->end(); fcIter++)
			{
				CraftServices::MSPFlightControllerAsync * pCurrentFc = (*fcIter);

				// Skip over ourself. Only tell Flight Controller about OTHER crafts we know the position of.
				if (pCurrentFc->MspFcInfo.GetUidAsHexString() != MspFcInfo.GetUidAsHexString())
				{
					if (!pCurrentFc->CurrentPositionEverBeenSet)
					{
						pSerialPortLogger->warn("{}: Other Craft {} - GPS position not yet received, not sending notice to other crafts, skipping.", GetPortAndCraftNamePrefix(), pCurrentFc->MspFcInfo.GetCraftName());
					}
					else
					{
						std::string currentCraftName = MspFcInfo.GetCraftName();
						std::string otherCraftName = pCurrentFc->MspFcInfo.GetCraftName();

						// How stale is information about this Craft? How old is the last position in milliseconds?
						int64_t timeDiffInMilliseconds = 0;
						bool currentCraftPositionIsStale = CraftPositionIsStale(pCurrentFc, timeDiffInMilliseconds);
						if (!currentCraftPositionIsStale)
						{
							pSerialPortLogger->debug("{}: Other craft {} position sufficiently fresh, is {} ms old. Sending...", GetPortAndCraftNamePrefix(), otherCraftName, timeDiffInMilliseconds);
							// Send position information about one particular Craft
							SendOtherCraftPositionMessage(*pCurrentFc);
						}
						else
						{
							pSerialPortLogger->warn("{}: Other Craft {} has stale position, is {} ms old. Not sending to other craft.", GetPortAndCraftNamePrefix(), otherCraftName, timeDiffInMilliseconds);
						}

						// Note that another way to handle stale detection would be to forward the position with a timestamp to the craft, and let the craft decide if the position is stale.
						// This would have the advantage of being a single place we have to adjust the stale interval, rather than in CraftServices and on the Craft itself. I think the
						// common GPS time source would actually make this possible -- everyone has access to the same, very accurate time. For now I'm not doing it though. I think that at the
						// very least there would always need to be some ground override so we could avoid sending positions about long-gone crafts (10+ minutes stale or whatever).
					}
				}
			}
		}
	}

	bool MSPFlightControllerAsync::CraftPositionIsStale(CraftServices::MSPFlightControllerAsync * pMspFlightController, int64_t & timeDifferenceInMilliseconds)
	{
		// How stale is information about this Craft? How old is the last position in milliseconds?
		boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
		boost::posix_time::time_duration timeDiff = (now - pMspFlightController->CurrentPositionRetrievalTime);
		timeDifferenceInMilliseconds = timeDiff.total_milliseconds();

		bool staleIntervalSetToNeverTimeout = StaleIntervalInMilliseconds == 0;
		bool craftPositionIsStale = (timeDifferenceInMilliseconds >= StaleIntervalInMilliseconds && (!staleIntervalSetToNeverTimeout));
		return craftPositionIsStale;
	}

	// Send notices about other, fake Craft
	void MSPFlightControllerAsync::SendNoticesAboutPhantomCrafts()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		// If this Flight Controller wants other Craft position updates...
		if (MspFcInfo.ShouldBeSentOtherCraftPositionUpdates)
		{
			pSerialPortLogger->trace("{}: SendNoticesAboutPhantomCrafts() - {}", GetPortAndCraftNamePrefix(), OverallPortStateAsString(PortState));

			// Go through all the Phantom Craft
			for (std::vector<CraftServices::PhantomTestCraft *>::iterator phantIter = pPhantomCraft->begin(); phantIter < pPhantomCraft->end(); phantIter++)
			{
				CraftServices::PhantomTestCraft * pCurrentPhantomCraft = (*phantIter);

				// How stale is information about this Craft? How old is the last position in milliseconds?
				int64_t timeDiffInMilliseconds = 0;
				bool currentCraftPositionIsStale = CraftPositionIsStale(this, timeDiffInMilliseconds);

				// Update all the reference positions (although not all phantom craft will care about this)
				CraftServices::msg::OtherCraftPositionMessage OtherCraftPositionMessage(*this);
				pCurrentPhantomCraft->UpdateReferenceCraftPosition(OtherCraftPositionMessage.MessageCraftInfoAndPosition, currentCraftPositionIsStale);

				// If this Phantom Craft is eligible to be sent..
				std::string eligibiltyMessage;
				if (pCurrentPhantomCraft->IsEligibleToBeSent(SerialPortName, eligibiltyMessage))
				{
					// Send position information about test Craft
					SendPhantomCraftPositionMessage(*pCurrentPhantomCraft);
				}
				else
				{
					std::string phantomCraftName = pCurrentPhantomCraft->CraftName;
					pSerialPortLogger->warn("{}: Skipping sending Phantom Craft {}, target position is not eligible: {}", GetPortAndCraftNamePrefix(), phantomCraftName, eligibiltyMessage);
				}
			}
		}
	}

	// This is responsible for *reading* in messages, byte by byte.
	void MSPFlightControllerAsync::StartReadMessageReceiveLoopForFlightController()
	{
		pSerialPortLogger->trace("{}: StartReadMessageReceiveLoopForFlightController()", GetPortAndCraftNamePrefix());

		// Clear read buffer
		ReadBuffer[0] = 0;
		boost::asio::async_read(*pSerialPort, boost::asio::buffer(&ReadBuffer, 1), [this](const boost::system::error_code& error, size_t sizeRead) { MessageReceiveOneByteReadCallback(error, sizeRead); });
	}

	// Callback routine when one byte is received for a particular Flight Controller connection
	void MSPFlightControllerAsync::MessageReceiveOneByteReadCallback(const boost::system::error_code& error, std::size_t sizeRead)
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		//pSerialPortLogger->trace("{}: MessageReceiveOneByteReadCallback()", SerialPortName);

		CountErrorsAfterRead(error, sizeRead);

		// No problems reading a single byte
		if (!error && sizeRead == 1)
		{
			std::string errorMessage = "";
			char currentCharacterRead = ReadBuffer[0];
			//pSerialPortLogger->trace("{}: Read character: ({})", SerialPortName, (static_cast<int>(currentCharacterRead)));
			bool byteProcessedOk = ProcessReceivedMessageByte(currentCharacterRead, errorMessage);
			if (!byteProcessedOk)
			{
				pSerialPortLogger->error("{}: Had problem parsing message byte: {}", GetPortAndCraftNamePrefix(), errorMessage);

				// RESET somehow here?? Restart the message loop? 
				// A softer alternative would be to reset the buffers, perhaps? Errors here could
				// maybe lead to enjambment?
			}
		}

		// There was a problem reading
		if (error)
		{
			// We expect (and ignore) read errors when the port is in a failed or closed state
			if (!IsClosedOrFailedPort(PortState))
			{
				pSerialPortLogger->error("{}: Had problem reading byte from port", GetPortAndCraftNamePrefix());
				pSerialPortLogger->error("{}: Doing hard reset on port {}", GetPortAndCraftNamePrefix(), SerialPortName);

				ResetPortHard();
				// Delay to allow the port to hopefully actually close before we attempt to reopen it
				boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
			}
		}

		// There was no problem, but we didn't read anything
		// (This happens after the end of message received and appears to be normal)
		if (!error && sizeRead == 0)
		{
			pSerialPortLogger->trace("{}: ## No Error, nothing read", GetPortAndCraftNamePrefix());
		}

		// Attempt to read the next byte (Yes, no matter what happened above. We solider on in the face of errors.)
		pSerialPortLogger->trace("{}: Setting up callback to read next byte", GetPortAndCraftNamePrefix());
		boost::asio::async_read(*pSerialPort, boost::asio::buffer(&ReadBuffer, 1), [this](const boost::system::error_code& error, size_t sizeRead) { MessageReceiveOneByteReadCallback(error, sizeRead); });
	}

	// Process a single received message byte
	// Returns true if processed successfully, false if there was a problem of some kind, with message place in errorMessage
	bool MSPFlightControllerAsync::ProcessReceivedMessageByte(char messageByte, std::string & errorMessage)
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return false;
			errorMessage = "Shutting down";
		}

		errorMessage = "";

		// Here we start to use a little structure - MessageScratchPad - to
		// build up the message information before we process it.
		bool messageScratchPadReadyToBeProcessed = false;

		switch (ReadState)
		{
		case CraftServices::MessageReadState::PreambleOne:
			MessageScratchPad.ClearValue();
			if (messageByte != '$')
			{
				errorMessage = "Expected $ for preamble byte 1";
				return false;
			}
			ReadState = CraftServices::MessageReadState::PreambleTwo;
			break;
		case CraftServices::MessageReadState::PreambleTwo:
			if (messageByte != 'X')
			{
				errorMessage = "Expected X for preamble byte 2";
				return false;
			}
			ReadState = CraftServices::MessageReadState::Direction;
			break;
		case CraftServices::MessageReadState::Direction:
			if (messageByte != '<' && messageByte != '>' && messageByte != '!')
			{
				// Something we did not expect
				errorMessage = "Expected <, >, or ! for direction, but got " + CraftServices::UidUtil::IntToHex(messageByte);
				return false;
			}
			if (messageByte == '!')
			{
				// Just debugging
				//cout << "Received error response" << endl;
			}

			MessageScratchPad.MessageDirectionCharacter = messageByte;
			ReadState = CraftServices::MessageReadState::ZeroFlag;
			break;
		case CraftServices::MessageReadState::ZeroFlag:
			if (messageByte != 0)
			{
				errorMessage = "Expected 0 for Zero Flag";
				return false;
			}
			ReadState = CraftServices::MessageReadState::MessageIDLowByte;
			break;
		case CraftServices::MessageReadState::MessageIDLowByte:
			MessageScratchPad.MessageIDLowByte = messageByte;
			ReadState = CraftServices::MessageReadState::MessageIDHighByte;
			break;
		case CraftServices::MessageReadState::MessageIDHighByte:
			MessageScratchPad.MessageIDHighByte = messageByte;
			MessageScratchPad.InitMessageID();
			ReadState = CraftServices::MessageReadState::DataPayloadLengthLowByte;
			break;
		case CraftServices::MessageReadState::DataPayloadLengthLowByte:
			MessageScratchPad.DataPayloadLengthLowByte = messageByte;
			ReadState = CraftServices::MessageReadState::DataPayloadLengthHighByte;
			break;
		case CraftServices::MessageReadState::DataPayloadLengthHighByte:
			MessageScratchPad.DataPayloadLengthHighByte = messageByte;
			MessageScratchPad.InitDataPayload();
			if (MessageScratchPad.DataPayloadLength > 0)
			{
				// There's a payload, so expect it
				ReadState = CraftServices::MessageReadState::DataPayload;
			}
			else
			{
				// Otherwise, skip right to the CRC
				ReadState = CraftServices::MessageReadState::CrcByte;
			}
			break;
		case CraftServices::MessageReadState::DataPayload:
			assert(MessageScratchPad.pDataPayload != NULL);
			MessageScratchPad.pDataPayload[MessageScratchPad.DataPayloadBytesRead] = messageByte;
			MessageScratchPad.DataPayloadBytesRead++;
			// If we've read the entire expected payload, we move on to the CRC. Otherwise,
			// keep expecting data payload bytes.
			if (MessageScratchPad.DataPayloadBytesRead == MessageScratchPad.DataPayloadLength)
			{
				ReadState = CraftServices::MessageReadState::CrcByte;
			}
			break;
		case CraftServices::MessageReadState::CrcByte:
			MessageScratchPad.CrcByte = messageByte;
			// Handled outside of the case statement to highlight it's importance
			messageScratchPadReadyToBeProcessed = true;
			ReadState = CraftServices::MessageReadState::PreambleOne;
			break;
		default:
			throw NotImplementedException("Unknown MessageReadState");
			break;
		}

		bool readStatus = true;

		// We have a complete message ready to be processed. Attempt to do so.
		if (messageScratchPadReadyToBeProcessed)
		{
			readStatus = ProcessMessageScratchPad(MessageScratchPad, errorMessage);
			// Clean up immediately for clarity
			MessageScratchPad.ClearValue();
		}

		// Everything OK
		return readStatus;
	}

	bool MSPFlightControllerAsync::ProcessMessageScratchPad(MspMessageScratchPad & messageScratchPadToProcess, std::string & errorMessage)
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return false;
			errorMessage = "Shutting down";
		}

		errorMessage = "";

		// Check CRC first
		ByteVector dataPayloadAsByteVector(messageScratchPadToProcess.pDataPayload, messageScratchPadToProcess.pDataPayload + messageScratchPadToProcess.DataPayloadLength);
		uint8_t calculatedCrc = CalculateCrcOfMessage(ZeroFlag, messageScratchPadToProcess.MessageID, dataPayloadAsByteVector);
		if (messageScratchPadToProcess.CrcByte != calculatedCrc)
		{
			errorMessage = "CRC Mismatch";
			return false;
		}

		// Was this an error response?
		if (messageScratchPadToProcess.MessageDirectionCharacter == '!')
		{
			std::ostringstream outString;
			outString << "Received error - ! message direction error response for Message ID " << CraftServices::UidUtil::IntToHex(messageScratchPadToProcess.MessageID) << ", did not process.";
			errorMessage = outString.str();
			return false;
		}

		bool processedSuccessfully = true;

		// Ok, message appears like it is valid. 
		// See if it's a message type we understand and can construct.		

		try
		{
			switch ((CraftServices::ID)messageScratchPadToProcess.MessageID)
			{
				case ID::MSP_FC_VARIANT:
				{
					CraftServices::msg::FcVariant fcVariantMsg(messageScratchPadToProcess.GetPayloadDataAsVector());
					MspFcInfo.FcVariantString = fcVariantMsg.CraftIdentifier;
					MspFcInfo.HasFcVariantString = true;
					pSerialPortLogger->debug("{}: Successfully parsed FcVariant message: {}", GetPortAndCraftNamePrefix(), MspFcInfo.FcVariantString);
					break;
				}

				case ID::MSP_UID:
				{
					CraftServices::msg::UidMessage uidMessage(messageScratchPadToProcess.GetPayloadDataAsVector());
					MspFcInfo.UID_0 = uidMessage.UID_0;
					MspFcInfo.UID_1 = uidMessage.UID_1;
					MspFcInfo.UID_2 = uidMessage.UID_2;
					MspFcInfo.HasUID = true;
					pSerialPortLogger->debug("{}: Successfully parsed UID message: {}", GetPortAndCraftNamePrefix(), MspFcInfo.GetUidAsHexString());
					break;
				}

				case ID::MSP_API_VERSION:
				{
					// ApiVersion
					CraftServices::msg::ApiVersion apiVersionMessage(messageScratchPadToProcess.GetPayloadDataAsVector());
					MspFcInfo.MSP_Version_Protocol = (int8_t)apiVersionMessage.Protocol;
					MspFcInfo.API_Version_Major = (int8_t)apiVersionMessage.Major;
					MspFcInfo.API_Version_Minor = (int8_t)apiVersionMessage.Minor;
					MspFcInfo.HasApiVersion = true;
					pSerialPortLogger->debug("{}: Successfully parsed MSP API message: {}", GetPortAndCraftNamePrefix(), MspFcInfo.GetMspAPIVersionString());
					break;
				}

				case ID::MSP_NAME:
				{
					// Craft Name

					// We grab the port/craft name early, so that it will reflect the craft name understanding BEFORE the message
					// was parsed. This gives us a better idea if this message actually did work or not (i.e. we did not already
					// have the CraftName.)
					std::string portAndCraftNamePrefix = GetPortAndCraftNamePrefix();
					CraftServices::msg::CraftNameMessage craftNameMessage(messageScratchPadToProcess.GetPayloadDataAsVector());
					MspFcInfo.CraftName = craftNameMessage.CraftName;
					MspFcInfo.HasCraftName = true;
					pSerialPortLogger->debug("{}: Successfully parsed Craft Name: {}", portAndCraftNamePrefix, MspFcInfo.CraftName);
					break;
				}

				case ID::MSP_RAW_GPS:
				{
					// Current location of the Craft
					CraftServices::msg::RawGPS gpsPositionMessage(messageScratchPadToProcess.GetPayloadDataAsVector());

					// Stash the updated position
					CurrentPosition = gpsPositionMessage;
					// Mark that we've gotten the GPS position at least once
					CurrentPositionEverBeenSet = true;
					// Also track how old the position information is. We only need millisecond resolution, and in fact
					// we may only get millisecond resolution depending on the platform, but we have to ask for microsecond
					// resolution from Boost.
					CurrentPositionRetrievalTime = boost::posix_time::microsec_clock::universal_time();
					std::string latLonString = CraftServices::GeoSpatialUtil::GetLatLonString(OmitGpsPos, CurrentPosition.MspLat, CurrentPosition.MspLon);

					// TODO: Speed needs units/formatting here...
					std::string groundCourseString = CraftServices::GeoSpatialUtil::GetDecidegreesAsDegreeString((int16_t)CurrentPosition.GroundCourseInDecidegrees);
					std::string hdopString = CraftServices::GeoSpatialUtil::GetHDOPAsString(CurrentPosition.HDOP);
					std::string fixTypeString = CraftServices::GetGpsFixTypeAsString(static_cast<CraftServices::GPSFixType>(CurrentPosition.FixType));
					pSerialPortLogger->info("{}: got new GPS position: {} - Alt {} meters - Course {} - Speed {} - {} (HDOP {}, {} sat)", GetPortAndCraftNamePrefix(), latLonString, (int16_t)CurrentPosition.AltitudeInMeters, groundCourseString, CurrentPosition.Speed, fixTypeString, hdopString, CurrentPosition.NumSat);

					// NOPE: This optimization still needs more work. It runs, but I'm seeing more contention over the air than hardwired led me to believe. Taking out for now.
					// 
					// Currently, the last operation we do on the port during a refresh cycle is to request the GPS position from the Flight Controller.
					// So, when we retrieve it successfully, we know we can move directly on to the next Flight Controller, without waiting any longer in refresh interval.
					// This means that when things are running smoothly, with no errors, we run as fast as possible.					
					/*
					if (!IsShutdownInProgressOrComplete())
					{
						ScheduleNextFlightControllerTimerToFireImmediately();
					}
					*/
					
					break;
				}

				case ID::MSP2_INAV_OTHER_CRAFT_POSITION_SETTING:
				{
					// We are being TOLD about the settings the Flight Controller on the other side has for being sent position updates.
					// (Does the MSP connected Flight Controller want to be told about the other Crafts that Craft Services is tracking?)
					CraftServices::msg::OtherCraftPositionSettingMessage otherCraftPositionSettingMessage(messageScratchPadToProcess.GetPayloadDataAsVector());

					MspFcInfo.ShouldBeSentOtherCraftPositionUpdates = otherCraftPositionSettingMessage.ShouldSendUpdates;
					MspFcInfo.HasOtherCraftPositionSetting = true;
					pSerialPortLogger->info("{}: Received reply about OtherCraftPositionSetting. Wants updates: {}", GetPortAndCraftNamePrefix(), CraftServices::BoolToString(MspFcInfo.ShouldBeSentOtherCraftPositionUpdates));

					break;
				}

				case ID::MSP2_INAV_OTHER_CRAFT_POSITION:
				{
					// Flight controller is acknowledging that we've sent it a OTHER_CRAFT_POSITION message
					CraftServices::ByteVector payloadDataVector = messageScratchPadToProcess.GetPayloadDataAsVector();

					// May be a generic type check we can make for a variety of messages, we'll see if this becomes needed.
					if (payloadDataVector.size() != 0)
					{
						processedSuccessfully = false;
						errorMessage = "Data payload for MSP2_INAV_OTHER_CRAFT_POSITION unexpectedly non-empty, so not an ACK - Message ID: " + std::to_string(messageScratchPadToProcess.MessageID);

						// Should the need arise, we could try something like this, but so far no need.
						/*
						CraftServices::msg::OtherCraftPositionMessage otherCraftPositionMessage(payloadDataVector);
						cout << SerialPortName << ": Received OtherCraftPositionMessage response: " << otherCraftPositionMessage.MessageCraftInfoAndPosition.GetCompleteCraftLocationString() << endl;
						*/
						break;
					}
					
					pSerialPortLogger->debug("{}: Received OtherCraftPositionMessage ACK-type response", GetPortAndCraftNamePrefix());

					break;
				}

				// Add more incoming message types here if you need them processed.
				default:
				{
					processedSuccessfully = false;
					errorMessage = "ProcessMessageScratchPad() - Unknown message ID: " + std::to_string(messageScratchPadToProcess.MessageID);
					break;
				}
			} // end switch

		} // end try
		catch (...)
		{
			// Something went wrong with message comprehension
			processedSuccessfully = false;
			errorMessage = "Unknown exception while processing MessageID: " + std::to_string(messageScratchPadToProcess.MessageID);
		}

		return processedSuccessfully;
	}

	void MSPFlightControllerAsync::CountErrorsAfterWrite(const boost::system::error_code& error, size_t sizeWritten)
	{
		//pSerialPortLogger->trace("{}: CountErrorsAfterWrite()", SerialPortName);

		// If there is an error...
		if (error)
		{
			// TODO: Make use of these errors to prompt a re-try?

			// .. Increase the count of sequential errors
			SequentialWriteErrorCount++;
		}
		else
		{
			// Success, clear error count
			SequentialWriteErrorCount = 0;
		}
	}

	void MSPFlightControllerAsync::CountErrorsAfterRead(const boost::system::error_code& error, size_t sizeRead)
	{
		//pSerialPortLogger->trace("{}: CountErrorsAfterRead()", SerialPortName);

		// If there is an error...
		if (error)
		{
			// We expect (and ignore) read errors when the port is in a failed or closed state
			if (!IsClosedOrFailedPort(PortState))
			{
				pSerialPortLogger->error("{}: Read error: {}", GetPortAndCraftNamePrefix(), error.message());
			}
			// TODO: Make use of these errors to prompt a re-try?


			// .. Increase the count of sequential errors
			SequentialReadErrorCount++;
		}
		else
		{
			// Success, clear error count
			SequentialReadErrorCount = 0;
		}
	}

	// Send request for FC Variant information (e.g. "INAV", etc.)
	void MSPFlightControllerAsync::RequestFcVariant()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->debug("{}: RequestFcVariant()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::FcVariant fcVariantRequest;
		ByteVector fcVariantMessageBytes = BuildMspMessageAsByteVector(fcVariantRequest);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(fcVariantMessageBytes.data(), fcVariantMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Send request for FC Variant information (e.g. "INAV", etc.)
	void MSPFlightControllerAsync::RequestUid()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->debug("{}: RequestUid()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::UidMessage uidMessage;
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(uidMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Send request for MSP API version (2.0.1 for example)
	void MSPFlightControllerAsync::RequestApi()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->debug("{}: RequestApi()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::ApiVersion apiVersionMessage;
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(apiVersionMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Send request for Craft Name ("Bob's MegaQuad")
	void MSPFlightControllerAsync::RequestCraftName()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->debug("{}: RequestCraftName()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::CraftNameMessage craftNameRequestMessage;
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(craftNameRequestMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Send request for current Raw GPS position
	void MSPFlightControllerAsync::RequestRawGPSPosition()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->debug("{}: RequestRawGPSPosition()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::RawGPS rawGpsRequestMessage;
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(rawGpsRequestMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	void MSPFlightControllerAsync::RequestOtherCraftPositionSetting()
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: RequestOtherCraftPositionSetting()", GetPortAndCraftNamePrefix());

		// For now, Craft Services never wants to know from the Flight Controller about other Craft Positions.
		// This is just a way to prompt the iNav side into sending us the settings.
		SendOtherCraftPositionSettingMessage(false);
	}

	std::string MSPFlightControllerAsync::GetPortAndCraftNamePrefix()
	{
		std::string optionalCraftNameAndParens = (MspFcInfo.CraftName != "") ? fmt::format(" ({})", MspFcInfo.CraftName) : "";
		std::string portAndCraftNamePrefix = fmt::format("{}{}", SerialPortName, optionalCraftNameAndParens);
		return portAndCraftNamePrefix;
	}

	// Perhaps someday we could use such math to try to optimize our refresh/wait intervals, calculating how much time it will
	// take to transmit, and how much time it might take to receive what's expected. Hmmm..
	size_t MSPFlightControllerAsync::GetExpectedTransmitTimeInMillisecondsForByteCount(size_t byteCount)
	{
		// Assumes 1 start, 1 stop bit
		const int BitsToSendAByte = 10; 
		const int MillisecondsInSecond = 1000;
		return ((double)byteCount * (double)BitsToSendAByte / (double)BaudRate) * (double)MillisecondsInSecond;
	}

	// -- Craft Services Messages --
	// -----------------------------

	// Send message from Craft Services to other Flight Controller about Other Craft Position Settings.
	// For the moment, iNav will interpret this only as a request, but I believe we could use directional settings
	// eventually to distinguish a query being sent from a response being sent. And if we could do that, both
	// sides could ask the other about Craft Position Settings. This could be relevant if the Flight Controller
	// had knowledge of other Crafts that Craft Services did not already via a direct connection.
	void MSPFlightControllerAsync::SendOtherCraftPositionSettingMessage(bool thisServerWantsToBeToldAboutOtherCrafts)
	{	
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: SendOtherCraftPositionSettingMessage()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::OtherCraftPositionSettingMessage OtherCraftPositionSettingMessage(thisServerWantsToBeToldAboutOtherCrafts);
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(OtherCraftPositionSettingMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Here we are actually telling the external Flight Controller about a particular Craft that Craft Services knows about.
	void MSPFlightControllerAsync::SendOtherCraftPositionMessage(MSPFlightControllerAsync & mspFlightControllerWithCraftToSendPositionOf)
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: SendOtherCraftPositionMessage()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::OtherCraftPositionMessage OtherCraftPositionMessage(mspFlightControllerWithCraftToSendPositionOf);
		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(OtherCraftPositionMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	// Here we are actually telling the external Flight Controller about a particular Craft that Craft Services knows about.
	void MSPFlightControllerAsync::SendPhantomCraftPositionMessage(CraftServices::PhantomTestCraft & phantomTestCraft)
	{
		if (IsThisFlightControllerShuttingDown())
		{
			return;
		}

		pSerialPortLogger->trace("{}: SendPhantomCraftPositionMessage()", GetPortAndCraftNamePrefix());

		// Build up the request
		CraftServices::msg::OtherCraftPositionMessage OtherCraftPositionMessage(phantomTestCraft);
		auto phantomCraftPosInfoString = OtherCraftPositionMessage.MessageCraftInfoAndPosition.GetCompleteCraftLocationString(OmitGpsPos);
		// Again, note the deliberate cast here to signed!
		int16_t altInMeters = OtherCraftPositionMessage.MessageCraftInfoAndPosition.AltitudeInMeters;
		pSerialPortLogger->info("{}: Sending Phantom Craft: {} - Alt {} meters", GetPortAndCraftNamePrefix(), phantomCraftPosInfoString, altInMeters);

		ByteVector uidMessageBytes = BuildMspMessageAsByteVector(OtherCraftPositionMessage);

		// Write it out to the serial port ASYNC FASHION
		boost::asio::async_write(*pSerialPort, boost::asio::buffer(uidMessageBytes.data(), uidMessageBytes.size()), [this](const boost::system::error_code& error, size_t sizeWritten) { CountErrorsAfterWrite(error, sizeWritten); });
	}

	ByteVector MSPFlightControllerAsync::BuildMspMessageAsByteVector(CraftServices::MspMessageAsync & mspMessageAsync)
	{
		// TODO: Why am I forced into this cast??
		uint16_t messageID = (uint16_t)mspMessageAsync.MessageID();
		return BuildMspMessageAsByteVector(messageID, mspMessageAsync.EncodePayload());
	}

	// Build up an outgoing message as a vector of bytes
	ByteVector MSPFlightControllerAsync::BuildMspMessageAsByteVector(const uint16_t messageId, const ByteVector & payloadData)
	{
		ByteVector msg;
		msg.reserve(8 + payloadData.size());

		msg.push_back('$');
		// 'X' is used in MSP V2 instead of 'M' in MSP V1. 
		// (We only speak V2)
		msg.push_back('X');
		// Direction indicator
		msg.push_back('<');

		msg.push_back(uint8_t(ZeroFlag));						  	    // Unused flag (always 0 for now)
		msg.push_back(BitOperators::LowByte(messageId));                // message_id (function) - low byte
		msg.push_back(BitOperators::HighByte(messageId));               // message_id (function) - high byte
		msg.push_back(BitOperators::LowByte((uint16_t)payloadData.size()));    // data payload length - low byte
		msg.push_back(BitOperators::HighByte((uint16_t)payloadData.size()));   // data payload length - low byte
		msg.insert(msg.end(), payloadData.begin(), payloadData.end());                // data payload

		uint8_t calculatedCrc = CalculateCrcOfMessage(ZeroFlag, messageId, payloadData);
		msg.push_back(calculatedCrc);

		auto messageSize = msg.size();
		auto expectedTransmitTimeinMilliseconds = GetExpectedTransmitTimeInMillisecondsForByteCount(messageSize);
		pSerialPortLogger->trace("{}: Built MSP Message Packet - MessageID {} - {} bytes long. Expected transmit time: {} ms", GetPortAndCraftNamePrefix(), messageId, messageSize, expectedTransmitTimeinMilliseconds);

		return msg;
	}

	void MSPFlightControllerAsync::ClearSerialBuffer()
	{
#if __unix__ || __APPLE__
		tcflush(pimpl->port.native_handle(), TCIOFLUSH);
#elif _WIN32
		PurgeComm(pSerialPort->native_handle(), PURGE_TXCLEAR);
#else
		#warning "clear() will be unimplemented"
#endif
	}

	// "crc8_dvb_s2 checksum algorithm. This is a single byte CRC algorithm that is much more robust than the XOR checksum in MSP v1."
	uint8_t MSPFlightControllerAsync::CalculateCrcOfMessage(const uint8_t flag, const uint16_t id, const ByteVector &data)
	{
		// flag byte
		uint8_t crc = Crc::crc8_dvb_s2(0, flag);

		// id(byte1, byte2)
		crc = Crc::crc8_dvb_s2(crc, BitOperators::LowByte(id));
		crc = Crc::crc8_dvb_s2(crc, BitOperators::HighByte(id));

		// length(byte1, byte2)
		uint16_t dataLength = (uint16_t)data.size();
		crc = Crc::crc8_dvb_s2(crc, BitOperators::LowByte(dataLength));
		crc = Crc::crc8_dvb_s2(crc, BitOperators::HighByte(dataLength));

		// all data bytes
		for (int i = 0; i < dataLength; i++)
		{
			crc = Crc::crc8_dvb_s2(crc, data[i]);
		}

		return crc;
	}

} // Namespace CraftServices