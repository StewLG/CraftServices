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

#ifndef MSPFLIGHTCONTROLLERASYNC_HPP
#define MSPFLIGHTCONTROLLERASYNC_HPP

#include <vector>
#include <string>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "NotImplementedException.hpp"
#include "AsyncMspMessageTypes.hpp"
#include "MspMessageAsyncs.hpp"
#include "UidUtil.hpp"
#include "PhantomTestCraft.hpp"

namespace CraftServices
{
	enum class OverallPortState 
	{ 
		// The port has not been opened yet
		PortClosed, 
		// Port failed to open
		PortOpenFailed,
		// Port is opened but session is not initialized
		PortOpened,
		// Session is initialized, and we are running regular session operations
		SessionRunning
	};

	std::string OverallPortStateAsString(CraftServices::OverallPortState overallPortState);

	// Is this Port in a Closed or Failed state?
	bool IsClosedOrFailedPort(CraftServices::OverallPortState overallPortState);

	enum class MessageReadState
	{
		// Expecting preamble character 1 ('$')
		PreambleOne,
		// Expecting premable character 2 ('X' - we speak MSP V2 only)
		PreambleTwo,
		// Expecting direction character ('<' or '>')
		Direction,
		// Expecting zero flag (always 0 at present)
		ZeroFlag,
		// Expecting Message ID first byte (low byte)
		MessageIDLowByte,
		// Expecting Message ID second byte (high byte)
		MessageIDHighByte,
		// Expecting Data Payload Length Low Byte
		DataPayloadLengthLowByte,
		// Expecting Data Payload Length Low Byte
		DataPayloadLengthHighByte,
		// Expecting Data Payload
		DataPayload,
		// Expecting CRC byte
		CrcByte
	};

	const std::string NotSetString = "[Not Set]";

	// We can't work with any protocol besides this
	const int ExpectedMspProtocolVersion = 0;
	// Minimum Version numbers for the MSP API that should have Craft Services messages available
	const int MinimumMspApiMajorVersionForCraftServicesMessages = 2;
	const int MinimumMspApiMinorVersionForCraftServicesMessages = 3;

	// Information about this MSP connection to a Flight Controller
	class MspFlightControllerInfo
	{
		public:

			MspFlightControllerInfo()
			{
				ResetStateValues();
			}

			// Information about this connection - what flight controller type is it? 
			// "INAV", "BTFL", etc.			
			std::string FcVariantString;

			// Three UID bytes. Unique identifier for the Flight Controller
			uint32_t UID_0;
			uint32_t UID_1;
			uint32_t UID_2;

			// MSP API Version number the Flight Controller uses
			/*
			#define MSP_PROTOCOL_VERSION                0   // Same version over MSPv1 & MSPv2 - message format didn't change and it backward compatible
			#define API_VERSION_MAJOR                   2   // increment when major changes are made
			#define API_VERSION_MINOR                   2   // increment when any change is made, reset to zero when major changes are released after changing API_VERSION_MAJOR
			*/
			int8_t MSP_Version_Protocol;
			int8_t API_Version_Major;
			int8_t API_Version_Minor;

			// What's the name of the Craft for this Flight Controller ("Bob's Quad")
			std::string CraftName;

			// Does this flight controller want to be told about other Craft Positions?
			bool ShouldBeSentOtherCraftPositionUpdates;

			// These bools indicate if the values above have been
			// set from the connected Flight Controller properly.
			bool HasFcVariantString;
			bool HasUID;
			bool HasApiVersion;
			bool HasCraftName;
			bool HasOtherCraftPositionSetting;

			void ResetStateValues()
			{
				HasFcVariantString = false;
				FcVariantString = "";
				HasUID = false;
				UID_0 = 0;
				UID_1 = 0;
				UID_2 = 0;
				MSP_Version_Protocol = 0;
				API_Version_Major = 0;
				API_Version_Minor = 0;
				HasApiVersion = false;
				HasCraftName = false;
				CraftName = "";
				HasOtherCraftPositionSetting = false;
				ShouldBeSentOtherCraftPositionUpdates = false;
			}


			// Have all values been set for this Flight Controller Info block?
			bool HaveAllRequiredValuesBeenSet()
			{
				bool otherCraftPositionSettingIsSetIfNeeded = false;
				if (HasApiVersion)
				{
					if (IsRunningMspApiWithCraftServicesMessagesAvailable())
					{
						// This message is supported, so we require it to be set
						otherCraftPositionSettingIsSetIfNeeded = HasOtherCraftPositionSetting;
					}
					else
					{
						// This message not supported, so we don't require it to be set
						otherCraftPositionSettingIsSetIfNeeded = true;
					}
				}

				return HasFcVariantString &&
					HasUID &&
					HasApiVersion &&
					HasCraftName &&
					// We can't directly use HasOtherCraftPositionSetting; see above.
					otherCraftPositionSettingIsSetIfNeeded;
			}

			std::string GetCraftName()
			{
				if (!HasCraftName)
				{
					return "[No Craft Name]";
				}

				return CraftName;
			}

			std::string GetUidAsHexString()
			{
				return HasUID ? CraftServices::UidUtil::UIDToHexString(UID_0, UID_1, UID_2) : NotSetString;
			}

			std::string GetMspAPIVersionString()
			{
				if (!HasApiVersion)
				{
					return NotSetString;
				}

				std::ostringstream outString;

				outString << "Protocol Version: " << (int)MSP_Version_Protocol << " - API Version: " << (int)API_Version_Major << "." << (int)API_Version_Minor;
				return outString.str();
			}

			bool IsRunningMspApiWithCraftServicesMessagesAvailable()
			{
				// Don't call this until you've set API version
				assert(HasApiVersion);
				// We can't work with any protocol besides this
				assert(MSP_Version_Protocol == ExpectedMspProtocolVersion);

				return (API_Version_Major > MinimumMspApiMajorVersionForCraftServicesMessages) ||
					   (API_Version_Major == MinimumMspApiMajorVersionForCraftServicesMessages && API_Version_Minor >= MinimumMspApiMinorVersionForCraftServicesMessages);
			}



	};

	// A single instance of a communications channel with an MSP-speaking flight controller (iNav, Betaflight, etc.)
	class MSPFlightControllerAsync
	{
		public:

			// Every flight controller / serial port gets its own logger.
			// This connects to two sinks, typically: the "all" sink (total output), and a sink just for this serial port.
			spdlog::logger * pSerialPortLogger;

			// Always 0 in MSP V2 protocol
			static const uint8_t ZeroFlag = 0;

			// Our current overall state of the port/connection
			CraftServices::OverallPortState PortState = CraftServices::OverallPortState::PortClosed;

			// The specific state of the current message read operation in progress
			CraftServices::MessageReadState ReadState = CraftServices::MessageReadState::PreambleOne;

			// Current count of sequential write errors on the serial port
			int32_t SequentialWriteErrorCount = 0;

			// Current count of sequential read errors on the serial port
			// (This is loose idea; a read problem could be a timeout, a bad crc, etc - no one reason.)
			int32_t SequentialReadErrorCount = 0;

			// Relevant IOContext
			boost::asio::io_context * pIoContext;

			// Name of Serial Port
			std::string SerialPortName;

			// Our serial port object
			boost::asio::serial_port * pSerialPort;

			// Baud rate to use on port
			uint32_t BaudRate;

			// Refresh interval in milliseconds
			uint32_t RefreshTimerIntervalInMilliseconds;

			// Stale interval in milliseconds
			uint32_t StaleIntervalInMilliseconds;

			// Immutable information about this connection - API version, Craft Name, etc, etc. 
			// Read once at startup and kept for reference.
			MspFlightControllerInfo MspFcInfo;

			// one-byte read buffer
			char ReadBuffer[1];

			// Scratch pad for the message being currently read
			MspMessageScratchPad MessageScratchPad;

			// Have we ever tried to open the port?
			bool HasMarkedPortStartupTime;
			// What time did we first try to open the port?
			boost::posix_time::ptime InitialPortStartupTime;

			// Are we currently in the process of shutting down this Flight Controller's operations?
			bool InProcessOfShuttingDownThisFlightController;

			// Current GPS position information
			// (When things go wrong with connectivity this will be "last known" position, but let's
			//  focus on the positive...)
			CraftServices::msg::RawGPS CurrentPosition;

			// Has a GPS position ever been set?
			bool CurrentPositionEverBeenSet;

			// When was the CurrentPosition last retrieved, with millisecond resolution
			boost::posix_time::ptime CurrentPositionRetrievalTime;

			// Should we exit when we lose GPS feedback from flight controller?
			bool ExitOnGpsLoss;

			// Should we omit/obscure exact GPS positions from logging?
			bool OmitGpsPos;

			// The parent container of flight controllers (so we can examine our peers)
			std::vector<CraftServices::MSPFlightControllerAsync *> * pAllMspFlightControllers;

			// Any Phantom Test craft. Usually empty, unless testing.
			std::vector<CraftServices::PhantomTestCraft *> * pPhantomCraft;

			// Constructor
			MSPFlightControllerAsync(boost::asio::io_context * pIo_context,
									 std::string serialPortName,
									 uint32_t baudRate,
									 uint32_t refreshTimerIntervalInMilliseconds,
									 uint32_t staleIntervalInMilliseconds,
									 std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pConsoleSink,
									 std::shared_ptr<spdlog::sinks::basic_file_sink_mt> pAllSink,
									 spdlog::level::level_enum spdLogLevel,
									 const std::string & loggingPattern,
									 std::string dateTimeLogFilePrefixString,
									 std::vector<CraftServices::MSPFlightControllerAsync *> * pAllMspFlightControllers,
									 std::vector<CraftServices::PhantomTestCraft *> * pPhantomTestCraft,
									 bool exitOnGpsLoss,
									 bool omitGpsPos);

			// Destructor
			~MSPFlightControllerAsync();
			
			// Set up Logging
			void SetUpLogger(std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pConsoleSink,
							 std::shared_ptr<spdlog::sinks::basic_file_sink_mt> pAllSink,
							 spdlog::level::level_enum spdLogLevel,
							 const std::string & loggingPattern,
				             std::string dateTimeLogFilePrefixString);

			// Open 
			void OpenPortAndStartSession();
			// TODO: Maybe make private? 
			void RequestInitialInformationFromFlightController();

			void RefreshFlightControllerState();

			void ResetPortSoftish();
			void ResetPortHard();
			void RestartPortIfNecessary();

			void DoSessionRunningOperations();
			void SendNoticesAboutOtherCrafts();
			void SendNoticesAboutPhantomCrafts();

			bool CraftPositionIsStale(CraftServices::MSPFlightControllerAsync * pMspFlightController, int64_t & timeDifferenceInMilliseconds);

			void StartReadMessageReceiveLoopForFlightController();
			void MessageReceiveOneByteReadCallback(const boost::system::error_code & error, std::size_t bytes_transferred);
			bool ProcessReceivedMessageByte(char messageByte, std::string & errorMessage);
			bool ProcessMessageScratchPad(MspMessageScratchPad & messageScratchPadToProcess, std::string & errorMessage);

			void CountErrorsAfterRead(const boost::system::error_code & error, size_t sizeRead);
			void CountErrorsAfterWrite(const boost::system::error_code& error, size_t sizeWritten);

			void RequestFcVariant();
			void RequestUid();
			void RequestApi();
			void RequestCraftName();
			void RequestRawGPSPosition();
			void RequestOtherCraftPositionSetting();

			std::string GetPortAndCraftNamePrefix();
			size_t GetExpectedTransmitTimeInMillisecondsForByteCount(size_t byteCount);

			void SendOtherCraftPositionSettingMessage(bool thisServerWantsToBeToldAboutOtherCrafts);
			void SendOtherCraftPositionMessage(MSPFlightControllerAsync & mspFlightControllerWithCraftToSendPositionOf);
			void SendPhantomCraftPositionMessage(CraftServices::PhantomTestCraft & phantomTestCraft);

			bool IsThisFlightControllerShuttingDown();

		private:
			// Timer that fires to prompt refreshing of information from this Flight Controller
			boost::asio::steady_timer * pRefreshTimer;

			ByteVector BuildMspMessageAsByteVector(CraftServices::MspMessageAsync & mspMessageAsync);
			ByteVector BuildMspMessageAsByteVector(const uint16_t messageId, const ByteVector & data);			

			void ClearSerialBuffer();
			uint8_t CalculateCrcOfMessage(const uint8_t flag, const uint16_t id, const ByteVector &data);

	};
}

#endif // MSPFLIGHTCONTROLLERASYNC_HPP


