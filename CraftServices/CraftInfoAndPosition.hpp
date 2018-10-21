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

#ifndef CRAFTINFOANDPOSITION_HPP
#define CRAFTINFOANDPOSITION_HPP

#include "deserialise.hpp"
#include "GeoSpatialUtil.hpp"

namespace CraftServices 
{
	// from gpsFixType_e in iNav
	enum GPSFixType 
	{ 
		GPS_NO_FIX = 0,
		GPS_FIX_2D,
		GPS_FIX_3D
    };

	static std::string GetGpsFixTypeAsString(GPSFixType gpsFixType)
	{
		switch (gpsFixType)
		{
		case(GPS_NO_FIX):
			return "GPS_NO_FIX";
			break;
		case(GPS_FIX_2D):
			return "GPS_FIX_2D";
			break;
		case(GPS_FIX_3D):
			return "GPS_FIX_3D";
			break;
		default:
			return "UNKNOWN";
			break;
		}
	}

	// Craft Position information (Lat, Lon, Alt, etc.)
	// Also includes other information like Craft Name, Unique ID, etc.
	class CraftInfoAndPosition
	{
	public:

		// UID for the Flight Controller
		// Unique ID bytes for the Flight Controller 
		uint32_t U_ID_0;
		uint32_t U_ID_1;
		uint32_t U_ID_2;

		// From RawGPS
		uint8_t FixType;
		uint8_t NumSat;
		uint32_t MspLat;
		uint32_t MspLon;
		uint16_t AltitudeInMeters;
		uint16_t Speed;
		uint16_t GroundCourseInDecidegrees;

		// TODO: Craft Type 

		// Craft Name ("Bob's Quad")
		std::string CraftName;

		void ClearValues();

		// Default constructor
		CraftInfoAndPosition();

		// Complete constructor
		CraftInfoAndPosition(uint32_t U_ID_0,
			uint32_t U_ID_1,
			uint32_t U_ID_2,
			uint8_t FixType,
			uint8_t NumSat,
			uint32_t MspLat,
			uint32_t MspLon,
			uint16_t AltitudeInMeters,
			uint16_t Speed,
			uint16_t GroundCourse,
			std::string CraftName);

		CraftServices::GeoSpatialPoint GetGeoSpatialPoint();

		std::string GetCompleteCraftLocationString(bool omitExactGpsPos);
	};
} // Namespace CraftServices

#endif // CRAFTINFOANDPOSITION_HPP