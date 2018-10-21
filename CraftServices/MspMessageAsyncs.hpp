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

// MSP message definitions
// http://www.multiwii.com/wiki/index.php?title=Multiwii_Serial_Protocol

#ifndef MSPMESSAGEASYNCS_HPP
#define MSPMESSAGEASYNCS_HPP

#include <string>
#include <array>
#include <sstream>
#include <set>
#include <climits>
#include <cassert>

//#include "types.hpp"

#include "deserialise.hpp"
#include "AsyncMspMessageTypes.hpp"
// Unsure this will be includable
#include "MspFlightControllerAsync.hpp"

namespace CraftServices {
namespace msg {

#define MULTIWII_IDENTIFIER "MWII";
#define BASEFLIGHT_IDENTIFIER "BAFL";
#define BETAFLIGHT_IDENTIFIER "BTFL"
#define CLEANFLIGHT_IDENTIFIER "CLFL"
#define INAV_IDENTIFIER "INAV"
#define RACEFLIGHT_IDENTIFIER "RCFL"
#define CRAFTSERVICES_IDENTIFIER "CSVC"

const static size_t N_SERVO = 8;
const static size_t N_MOTOR = 8;

const static size_t BOARD_IDENTIFIER_LENGTH = 4;

const static size_t BUILD_DATE_LENGTH = 11;
const static size_t BUILD_TIME_LENGTH = 8;
const static size_t GIT_SHORT_REVISION_LENGTH = 7;

enum class MultiType : uint8_t {
    TRI             = 1,
    QUADP,  		// 2
    QUADX,  		// 3
    BI,     		// 4
    GIMBAL, 		// 5
    Y6,     		// 6
    HEX6,   		// 7
    FLYING_WING,	// 8
    Y4,     		// 9
    HEX6X,  		// 10
    OCTOX8, 		// 11
    OCTOFLATP,  	// 12
    OCTOFLATX,  	// 13
    AIRPLANE,   	// 14
    HELI_120_CCPM,  // 15
    HELI_90_DEG,    // 16
    VTAIL4,     	// 17
    HEX6H,      	// 18
    DUALCOPTER      = 20,
    SINGLECOPTER,   // 21
};

enum class Capability {
    BIND,
    DYNBAL,
    FLAP,
    NAVCAP,
    EXTAUX
};

enum class Sensor {
    Accelerometer,
    Barometer,
    Magnetometer,
    GPS,
    Sonar
};

const static size_t NAUX = 4;

enum class SwitchPosition : size_t {
    LOW  = 0,
    MID  = 1,
    HIGH = 2,
};

static const std::vector<std::string> FEATURES = {
    "RX_PPM", "VBAT", "INFLIGHT_ACC_CAL", "RX_SERIAL", "MOTOR_STOP",
    "SERVO_TILT", "SOFTSERIAL", "GPS", "FAILSAFE",
    "SONAR", "TELEMETRY", "AMPERAGE_METER", "3D", "RX_PARALLEL_PWM",
    "RX_MSP", "RSSI_ADC", "LED_STRIP", "DISPLAY", "ONESHOT125",
    "BLACKBOX", "CHANNEL_FORWARDING", "TRANSPONDER", "OSD"
};

// Many of these message definitions were lifted (with gratitude) 
// from Christian Rauch's MSP project:
//
// https://github.com/christianrauch/msp
//
// But many are not being used in this project. Those that aren't
// are generally commented out, and need to be massaged into shape
// if they are needed here. I've left them here so they can 
// be grabbed off the shelf if needed as a starting point.
//
// -- SLG 7/5/2018

// MSP_API_VERSION: 1
struct ApiVersion : public CraftServices::MspMessageAsync
{
    ID MessageID() const 
	{ 
		return ID::MSP_API_VERSION; 
	}

	size_t Protocol;
	size_t Major;
	size_t Minor;

	// Default constructor
	ApiVersion() : MspMessageAsync()
	{
		// TODO Default to the Protocol version we know about
		Protocol = 0;
		Major = 0;
		Minor = 0;
	}

	// Payload constructor
	ApiVersion(const ByteVector & payloadData)
	{
		DecodePayload(payloadData);
	}

	ByteVector EncodePayload() const
	{
		ByteVector encodedPayload;
		encodedPayload.push_back((uint8_t)Protocol);
		encodedPayload.push_back((uint8_t)Major);
		encodedPayload.push_back((uint8_t)Minor);
		return encodedPayload;
	}

	void DecodePayload(const ByteVector & payloadData)
	{
		assert(payloadData.size() == 3);
        Protocol = payloadData[0];
        Major = payloadData[1];
        Minor = payloadData[2];
    }
};

// MSP_FC_VARIANT: 2
struct FcVariant : public CraftServices::MspMessageAsync
{
    ID MessageID() const
	{ 
		return ID::MSP_FC_VARIANT; 
	}

	// e.g. "INAV"
    std::string CraftIdentifier;

	// Default constructor
	FcVariant() : MspMessageAsync()
	{
	}

	// Payload constructor
	FcVariant(const ByteVector & payloadData)
	{
		DecodePayload(payloadData);
	}
	
	ByteVector EncodePayload() const
	{
		return ByteVector(CraftIdentifier.begin(), CraftIdentifier.end());
	}

    void DecodePayload(const ByteVector & payloadData)
	{
        CraftIdentifier = std::string(payloadData.begin(), payloadData.end());
    }	
};

//// MSP_FC_VERSION: 3
//struct FcVersion : public CraftServices::MspMessageAsync 
//{
//    ID id() const 
//	{ 
//		return ID::MSP_FC_VERSION; 
//	}
//
//	size_t major;
//	size_t minor;
//	size_t patch_level;
//
//    void decode(const std::vector<uint8_t> &data) 
//	{
//        major = data[0];
//        minor = data[1];
//        patch_level = data[2];
//    }
//};

//// MSP_BOARD_INFO: 4
//struct BoardInfo : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_BOARD_INFO; }
//
//    std::string identifier;
//    uint16_t version;
//    uint8_t type;
//
//    void decode(const std::vector<uint8_t> &data) {
//        identifier = std::string(data.begin(), data.begin()+BOARD_IDENTIFIER_LENGTH);
//        version = CraftServices::deserialise_uint16(data,BOARD_IDENTIFIER_LENGTH);
//        type = data[BOARD_IDENTIFIER_LENGTH+2];
//    }
//};


// MSP_NAME: 10
// Request for the Craft Name
struct CraftNameMessage : public CraftServices::MspMessageAsync
{
	ID MessageID() const
	{
		return ID::MSP_NAME;
	}

	// e.g. "Bob's Quad"
	std::string CraftName;

	// Default constructor
	CraftNameMessage() : MspMessageAsync()
	{
	}

	// Payload constructor
	CraftNameMessage(const ByteVector & payloadData)
	{
		DecodePayload(payloadData);
	}

	ByteVector EncodePayload() const
	{
		return ByteVector(CraftName.begin(), CraftName.end());
	}

	void DecodePayload(const ByteVector & payloadData)
	{
		CraftName = std::string(payloadData.begin(), payloadData.end());
	}
};








//// MSP_FEATURE: 36
//struct Feature : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_FEATURE; }
//
//    std::set<std::string> features;
//
//    void decode(const std::vector<uint8_t> &data) {
//        const uint32_t mask = msp::deserialise_uint32(data,0);
//        for(size_t ifeat(0); ifeat<FEATURES.size(); ifeat++) {
//            if(mask & (1<<ifeat))
//                features.insert(FEATURES[ifeat]);
//        }
//    }
//};
//
//// MSP_SET_FEATURE: 37
//struct SetFeature : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_SET_FEATURE; }
//
//    std::set<std::string> features;
//
//    std::vector<uint8_t> encode() const {
//        std::vector<uint8_t> data;
//        uint32_t mask = 0;
//        for(size_t ifeat(0); ifeat<FEATURES.size(); ifeat++) {
//            if(features.count(FEATURES[ifeat]))
//                mask |= 1<<ifeat;
//        }
//		msp::serialise_uint32(mask, data);
//        return data;
//    }
//};

//// MSP_RX_CONFIG: 44
//struct RxConfig : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_RX_CONFIG; }
//
//    uint8_t serialrx_provider;
//    uint16_t maxcheck;
//    uint16_t midrc;
//    uint16_t mincheck;
//    uint8_t spektrum_sat_bind;
//    uint16_t rx_min_usec;
//    uint16_t rx_max_usec;
//
//    void decode(const std::vector<uint8_t> &data) {
//        serialrx_provider = data[0];
//        maxcheck = CraftServices::deserialise_uint16(data, 1);
//        midrc = CraftServices::deserialise_uint16(data, 3);
//        mincheck = CraftServices::deserialise_uint16(data, 5);
//        spektrum_sat_bind = data[7];
//        rx_min_usec = CraftServices::deserialise_uint16(data, 8);
//        rx_max_usec = CraftServices::deserialise_uint16(data, 10);
//    }
//};
//
//// MSP_SONAR_ALTITUDE: 58
//struct SonarAltitude : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_SONAR_ALTITUDE; }
//
//    float altitude; // meters
//
//    void decode(const std::vector<uint8_t> &data) {
//        altitude = CraftServices::deserialise_int32(data, 0)/100.0f;
//    }
//};

//// MSP_RX_MAP: 64
//struct RxMap : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_RX_MAP; }
//
//    std::vector<uint8_t> map;
//
//    void decode(const std::vector<uint8_t> &data) {
//        map = data;
//    }
//};
//
//// MSP_SET_RX_MAP: 65
//struct SetRxMap : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_SET_RX_MAP; }
//
//    std::vector<uint8_t> map;
//
//    std::vector<uint8_t> encode() const {
//        return map;
//    }
//};

//// MSP_REBOOT: 68
//struct Reboot : public CraftServices::MspMessageAsync 
//{
//    ID id() const { return ID::MSP_REBOOT; }
//    std::vector<uint8_t> encode() const {
//        return std::vector<uint8_t>();
//    }
//};


/////////////////////////////////////////////////////////////////////
/// Requests (1xx)

// --- Could use for Craft Type - Multirotor/Fixed wing/Tricopter -- SLG

//// MSP_IDENT: 100
//struct Ident : public CraftServices::MspMessageAsync 
//{
//    ID id() const 
//	{ 
//		return ID::MSP_IDENT; 
//	}
//
//	size_t version;
//    MultiType type;
//	size_t msp_version;
//    std::set<Capability> capabilities;
//
//    void decode(const std::vector<uint8_t> &data) 
//	{
//        version = data[0];
//
//        // determine multicopter type
//        type = MultiType(data[1]);
//
//        msp_version = data[2];
//
//        const uint32_t capability = msp::deserialise_uint32(data, 3);
//        if(capability & (1 << 0))
//            capabilities.insert(Capability::BIND);
//        if(capability & (1 << 2))
//            capabilities.insert(Capability::DYNBAL);
//        if(capability & (1 << 3))
//            capabilities.insert(Capability::FLAP);
//        if(capability & (1 << 4))
//            capabilities.insert(Capability::NAVCAP);
//        if(capability & (1 << 5))
//            capabilities.insert(Capability::EXTAUX);
//    }
//
//    bool has(const Capability &cap) const { return capabilities.count(cap) > 0; }
//
//    bool hasBind() const { return has(Capability::BIND); }
//
//    bool hasDynBal() const { return has(Capability::DYNBAL); }
//
//    bool hasFlap() const { return has(Capability::FLAP); }
//};




struct RawGPS : public CraftServices::MspMessageAsync
{
	public:
		ID MessageID() const
		{
			return ID::MSP_RAW_GPS;
		}

		// Several of these are actually signed variables, but the native format of the
		// message is unsigned, so we keep them in the format they were received. (Better to
		// get some minor display issue on the CraftServices side wrong than corrupt 
		// the actual data being sent between Flight Controllers.)
		// 
		// Actually signed:
		//
		// Lat, Lon, Altitude

		uint8_t FixType;
		uint8_t NumSat;
		uint32_t MspLat;
		uint32_t MspLon;
		uint16_t AltitudeInMeters;
		uint16_t Speed;
		uint16_t GroundCourseInDecidegrees;
		uint16_t HDOP;

		// Default constructor
		RawGPS() : MspMessageAsync()
		{
			ClearValues();
		}

		// Payload constructor
		RawGPS(const ByteVector & payloadData)
		{
			DecodePayload(payloadData);
		}

		void ClearValues()
		{
			FixType = 0;
			NumSat = 0;
			MspLat = 0;
			MspLon = 0;
			AltitudeInMeters = 0;
			Speed = 0;
			GroundCourseInDecidegrees = 0;
			HDOP = 0;
		}

		ByteVector EncodePayload() const
		{
			ByteVector encodedPayload;

			serialize_uint8(FixType, encodedPayload);
			serialize_uint8(NumSat, encodedPayload);
			serialize_uint32(MspLat, encodedPayload);
			serialize_uint32(MspLon, encodedPayload);
			serialize_uint16(AltitudeInMeters, encodedPayload);
			serialize_uint16(Speed, encodedPayload);
			serialize_uint16(GroundCourseInDecidegrees, encodedPayload);
			serialize_uint16(HDOP, encodedPayload);
	
			return encodedPayload;
		}

		void DecodePayload(const ByteVector & payloadData)
		{
			assert(payloadData.size() == 18);
			ClearValues();

			FixType = CraftServices::deserialize_uint8(payloadData, 0);
			NumSat = CraftServices::deserialize_uint8(payloadData, 1);
			MspLat = CraftServices::deserialize_uint32(payloadData, 2);
			MspLon = CraftServices::deserialize_uint32(payloadData, 6);
			AltitudeInMeters = CraftServices::deserialize_uint16(payloadData, 10);
			Speed = CraftServices::deserialize_uint16(payloadData, 12);
			// This is the GPS heading, and may not be the definitive word on what the
			// true direction/attitude of the craft may actually be. I also think it
			// may not be accurate until the craft is actually moving. Is this correct?
			GroundCourseInDecidegrees = CraftServices::deserialize_uint16(payloadData, 14);
			HDOP = CraftServices::deserialize_uint16(payloadData, 16);
		}

};



//
//// MSP_COMP_GPS: 107
//struct CompGPS : public CraftServices::MspMessageAsync 
//{
//    ID id() const 
//	{ 
//		return ID::MSP_COMP_GPS; 
//	}
//
//    uint16_t distanceToHome;    // meter
//    uint16_t directionToHome;   // degree
//    uint8_t update;
//
//    void decode(const std::vector<uint8_t> &data) 
//	{
//        distanceToHome  = CraftServices::deserialise_uint16(data, 0);
//        directionToHome = CraftServices::deserialise_uint16(data, 2);
//        update          = data[4];
//    }
//};
//
//// MSP_ATTITUDE: 108
//struct Attitude : public CraftServices::MspMessageAsync {
//    ID id() const { return ID::MSP_ATTITUDE; }
//
//    float ang_x;        // degree
//    float ang_y;        // degree
//    int16_t heading;    // degree
//
//    void decode(const std::vector<uint8_t> &data) {
//        ang_x   = CraftServices::deserialise_int16(data, 0)/10.0f;
//        ang_y   = CraftServices::deserialise_int16(data, 2)/10.0f;
//        heading = CraftServices::deserialise_int16(data, 4);
//    }
//};
//
//// MSP_ALTITUDE: 109
//struct Altitude : public CraftServices::MspMessageAsync {
//    ID id() const { return ID::MSP_ALTITUDE; }
//
//    float altitude; // m
//    float vario;    // m/s
//
//    void decode(const std::vector<uint8_t> &data) {
//        altitude = CraftServices::deserialise_int32(data, 0)/100.0f;
//        vario    = CraftServices::deserialise_int16(data, 4)/100.0f;
//    }
//};
//
//
//// MSP_ANALOG: 110
//struct Analog : public CraftServices::MspMessageAsync {
//    ID id() const { return ID::MSP_ANALOG; }
//
//    float	vbat;           // Volt
//    float	powerMeterSum;  // Ah
//	size_t	rssi;  // Received Signal Strength Indication [0; 1023]
//    float	amperage;       // Ampere
//
//    void decode(const std::vector<uint8_t> &data) {
//        vbat          = data[0]/10.0f;
//        powerMeterSum = CraftServices::deserialise_uint16(data, 1)/1000.0f;
//        rssi          = CraftServices::deserialise_uint16(data, 3);
//        amperage      = CraftServices::deserialise_uint16(data, 5)/10.0f;
//    }
//};







//// MSP_WP: 118
//struct WayPoint : public CraftServices::MspMessageAsync {
//    ID id() const { return ID::MSP_WP; }
//
//    uint8_t wp_no;
//    uint32_t lat;
//    uint32_t lon;
//    uint32_t altHold;
//    uint16_t heading;
//    uint16_t staytime;
//    uint8_t navflag;
//
//    void decode(const std::vector<uint8_t> &data) {
//        wp_no = data[0];
//        lat = msp::deserialise_uint32(data, 1);
//        lon = msp::deserialise_uint32(data, 5);
//        altHold = msp::deserialise_uint32(data, 9);
//        heading = CraftServices::deserialise_uint16(data, 13);
//        staytime = CraftServices::deserialise_uint16(data, 15);
//        navflag = data[18];
//    }
//};



//// MSP_NAV_STATUS: 121
//struct NavStatus: public CraftServices::MspMessageAsync {
//    ID id() const { return ID::MSP_NAV_STATUS; }
//
//    uint8_t GPS_mode;
//    uint8_t NAV_state;
//    uint8_t mission_action;
//    uint8_t mission_number;
//    uint8_t NAV_error;
//    int16_t target_bearing; // degrees
//
//    void decode(const std::vector<uint8_t> &data) {
//        GPS_mode = data[0];
//        NAV_state = data[1];
//        mission_action = data[2];
//        mission_number = data[3];
//        NAV_error = data[4];
//        target_bearing = CraftServices::deserialise_int16(data, 5);
//    }
//};


// MSP_UID: 160
struct UidMessage : public CraftServices::MspMessageAsync
{
	ID MessageID() const
	{
		return ID::MSP_UID;
	}

	// Unique ID bytes for the Flight Controller 
	uint32_t UID_0;
	uint32_t UID_1;
	uint32_t UID_2;

	// Default constructor
	UidMessage() : MspMessageAsync()
	{
		// Synthetic marker for now; should probably use something actually unique.
		UID_0 = 1111;
		UID_1 = 2222;
		UID_2 = 3333;
	}

	// Payload constructor
	UidMessage(const ByteVector & payloadData)
	{
		DecodePayload(payloadData);
	}

	ByteVector EncodePayload() const
	{
		ByteVector encodedPayload;

		serialize_uint32(UID_0, encodedPayload);
		serialize_uint32(UID_1, encodedPayload);
		serialize_uint16(UID_2, encodedPayload);

		return encodedPayload;
	}

	void DecodePayload(const ByteVector & payloadData)
	{
		UID_0 = CraftServices::deserialize_uint32(payloadData, 0);
		UID_1 = CraftServices::deserialize_uint32(payloadData, 4);
		UID_2 = CraftServices::deserialize_uint32(payloadData, 8);
	}

};


// Settings the sender has for receiving regular Craft Position updates. 
//
// MSP2_INAV_OTHER_CRAFT_POSITION_SETTING = 0x201A,
struct OtherCraftPositionSettingMessage : public CraftServices::MspMessageAsync
{
	ID MessageID() const
	{
		return ID::MSP2_INAV_OTHER_CRAFT_POSITION_SETTING;
	}

	// Boolean - should other Craft Position updates be sent to this Flight Controller? 
	uint8_t ShouldSendUpdates;

	// Default constructor
	OtherCraftPositionSettingMessage() : MspMessageAsync()
	{
		ShouldSendUpdates = (uint8_t)false;
	}

	// Boolean constructor
	OtherCraftPositionSettingMessage(bool shouldSendUpdates) : MspMessageAsync()
	{
		ShouldSendUpdates = (uint8_t)shouldSendUpdates;
	}

	// Payload constructor
	OtherCraftPositionSettingMessage(const ByteVector & payloadData)
	{
		DecodePayload(payloadData);
	}

	ByteVector EncodePayload() const
	{
		ByteVector encodedPayload;
		encodedPayload.push_back(ShouldSendUpdates);
		return encodedPayload;
	}

	void DecodePayload(const ByteVector & payloadData)
	{
		ShouldSendUpdates = payloadData[0];
	}
};





} // namespace msg
} // namespace msp

#endif // MSPMESSAGEASYNCS_HPP
