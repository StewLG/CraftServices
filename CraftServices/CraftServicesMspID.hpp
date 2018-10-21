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

#ifndef CRAFT_SERVICES_MSP_ID_HPP
#define CRAFT_SERVICES_MSP_ID_HPP

namespace CraftServices {

enum class ID : uint16_t {
    // Cleanflight
    MSP_API_VERSION = 1,
    MSP_FC_VARIANT  = 2,
    MSP_FC_VERSION  = 3,
    MSP_BOARD_INFO  = 4,
    MSP_BUILD_INFO  = 5,

	// out message          Returns user set board name - betaflight
	MSP_NAME        =  10,

    MSP_BATTERY_CONFIG            = 32,
    MSP_SET_BATTERY_CONFIG        = 33,
    MSP_MODE_RANGES               = 34,
    MSP_SET_MODE_RANGE            = 35,
    MSP_FEATURE                   = 36,
    MSP_SET_FEATURE               = 37,
    MSP_BOARD_ALIGNMENT           = 38,
    MSP_SET_BOARD_ALIGNMENT       = 39,
    MSP_AMPERAGE_METER_CONFIG     = 40,
    MSP_SET_AMPERAGE_METER_CONFIG = 41,
    MSP_MIXER                     = 42,
    MSP_SET_MIXER                 = 43,
    MSP_RX_CONFIG                 = 44,
    MSP_SET_RX_CONFIG             = 45,
    MSP_LED_COLORS                = 46,
    MSP_SET_LED_COLORS            = 47,
    MSP_LED_STRIP_CONFIG          = 48,
    MSP_SET_LED_STRIP_CONFIG      = 49,
    MSP_RSSI_CONFIG               = 50,
    MSP_SET_RSSI_CONFIG           = 51,
    MSP_ADJUSTMENT_RANGES         = 52,
    MSP_SET_ADJUSTMENT_RANGE      = 53,

    MSP_CF_SERIAL_CONFIG          = 54,
    MSP_SET_CF_SERIAL_CONFIG      = 55,

    MSP_VOLTAGE_METER_CONFIG      = 56,
    MSP_SET_VOLTAGE_METER_CONFIG  = 57,

    MSP_SONAR_ALTITUDE            = 58,

    MSP_ARMING_CONFIG             = 61,
    MSP_SET_ARMING_CONFIG         = 62,

    MSP_RX_MAP                    = 64,
    MSP_SET_RX_MAP                = 65,

    MSP_REBOOT                    = 68,

    MSP_BF_BUILD_INFO             = 69,

    MSP_DATAFLASH_SUMMARY         = 70,
    MSP_DATAFLASH_READ            = 71,
    MSP_DATAFLASH_ERASE           = 72,

    MSP_LOOP_TIME                 = 73,
    MSP_SET_LOOP_TIME             = 74,

    MSP_FAILSAFE_CONFIG           = 75,
    MSP_SET_FAILSAFE_CONFIG       = 76,

    MSP_RXFAIL_CONFIG             = 77,
    MSP_SET_RXFAIL_CONFIG         = 78,

    MSP_SDCARD_SUMMARY            = 79,

    MSP_BLACKBOX_CONFIG           = 80,
    MSP_SET_BLACKBOX_CONFIG       = 81,

    MSP_TRANSPONDER_CONFIG        = 82,
    MSP_SET_TRANSPONDER_CONFIG    = 83,

    MSP_OSD_CHAR_WRITE            = 87,

    MSP_VTX                       = 88,

    MSP_OSD_VIDEO_CONFIG          = 180,
    MSP_SET_OSD_VIDEO_CONFIG      = 181,
    MSP_OSD_VIDEO_STATUS          = 182,
    MSP_OSD_ELEMENT_SUMMARY       = 183,
    MSP_OSD_LAYOUT_CONFIG         = 184,
    MSP_SET_OSD_LAYOUT_CONFIG     = 185,

    MSP_3D                  = 124,
    MSP_RC_DEADBAND         = 125,
    MSP_SENSOR_ALIGNMENT    = 126,
    MSP_LED_STRIP_MODECOLOR = 127,
    MSP_VOLTAGE_METERS      = 128,
    MSP_AMPERAGE_METERS     = 129,
    MSP_BATTERY_STATE       = 130,
    MSP_PILOT               = 131,

    MSP_SET_3D               	= 217,
    MSP_SET_RC_DEADBAND      	= 218,
    MSP_SET_RESET_CURR_PID   	= 219,
    MSP_SET_SENSOR_ALIGNMENT 	= 220,
    MSP_SET_LED_STRIP_MODECOLOR = 221,
    MSP_SET_PILOT            	= 222,
    MSP_PASSTHROUGH_SERIAL      = 244,

    MSP_UID                 = 160,
    MSP_GPSSVINFO           = 164,
    MSP_SERVO_MIX_RULES     = 241,
    MSP_SET_SERVO_MIX_RULE  = 242,
    MSP_SET_4WAY_IF         = 245,

    // MultiWii
    MSP_IDENT       = 100,
    MSP_STATUS      = 101,
    MSP_RAW_IMU     = 102,
    MSP_SERVO       = 103,
    MSP_MOTOR       = 104,
    MSP_RC          = 105,
    MSP_RAW_GPS     = 106,
    MSP_COMP_GPS    = 107,
    MSP_ATTITUDE    = 108,
    MSP_ALTITUDE    = 109,
    MSP_ANALOG      = 110,
    MSP_RC_TUNING   = 111,
    MSP_PID         = 112,
    MSP_BOX         = 113,
    MSP_MISC        = 114,
    MSP_MOTOR_PINS  = 115,
    MSP_BOXNAMES    = 116,
    MSP_PIDNAMES    = 117,
    MSP_WP          = 118,
    MSP_BOXIDS      = 119,
    MSP_SERVO_CONF  = 120,

    MSP_NAV_STATUS  = 121,
    MSP_NAV_CONFIG  = 122,

    MSP_CELLS       = 130,

    MSP_SET_RAW_RC      = 200,
    MSP_SET_RAW_GPS     = 201,
    MSP_SET_PID         = 202,
    MSP_SET_BOX         = 203,
    MSP_SET_RC_TUNING   = 204,
    MSP_ACC_CALIBRATION = 205,
    MSP_MAG_CALIBRATION = 206,
    MSP_SET_MISC        = 207,
    MSP_RESET_CONF      = 208,
    MSP_SET_WP          = 209,
    MSP_SELECT_SETTING  = 210,
    MSP_SET_HEAD        = 211,
    MSP_SET_SERVO_CONF  = 212,
    MSP_SET_MOTOR       = 214,
    MSP_SET_NAV_CONFIG  = 215,

    MSP_SET_ACC_TRIM    = 239,
    MSP_ACC_TRIM        = 240,
    MSP_BIND            = 241,

    MSP_EEPROM_WRITE    = 250,

    MSP_DEBUGMSG        = 253,
    MSP_DEBUG           = 254,

	// Inav specific
	// -------------------
	MSP2_INAV_STATUS				= 0x2000,
	MSP2_INAV_OPTICAL_FLOW			= 0x2001,
	MSP2_INAV_ANALOG				= 0x2002,
	MSP2_INAV_MISC					= 0x2003,
	MSP2_INAV_SET_MISC				= 0x2004,
	MSP2_INAV_BATTERY_CONFIG		= 0x2005,
	MSP2_INAV_SET_BATTERY_CONFIG	= 0x2006,
	MSP2_INAV_RATE_PROFILE          = 0x2007,
	MSP2_INAV_SET_RATE_PROFILE      = 0x2008,
	MSP2_INAV_AIR_SPEED             = 0x2009,
	// INav specific.
	// These hopefully can be added to MSP V2 protocol for CraftServices
	// (and eventually other clients, surely.)

	// Settings for receiving regular Craft Position updates. 
	MSP2_INAV_OTHER_CRAFT_POSITION_SETTING = 0x201A,

	// A position message for a single other craft.
	MSP2_INAV_OTHER_CRAFT_POSITION = 0x201B,
};

} // namespace CraftServices

#endif // CRAFT_SERVICES_MSP_ID_HPP
