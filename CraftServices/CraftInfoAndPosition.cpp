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

#include "CraftInfoAndPosition.hpp"
#include "UidUtil.hpp"

namespace CraftServices
{

	// Default constructor
	CraftInfoAndPosition::CraftInfoAndPosition()
	{
		ClearValues();
	}

	void CraftInfoAndPosition::ClearValues()
	{
		U_ID_0 = 0;
		U_ID_1 = 0;
		U_ID_2 = 0;

		FixType = 0;
		NumSat = 0;
		MspLat = 0;
		MspLon = 0;
		AltitudeInMeters = 0;
		Speed = 0;
		GroundCourseInDecidegrees = 0;

		CraftName = "";
	}

	// Complete constructor
	CraftInfoAndPosition::CraftInfoAndPosition(uint32_t uid0,
											   uint32_t uid1,
											   uint32_t uid2,
											   uint8_t fixType,
											   uint8_t numSat,
											   uint32_t mspLat,
											   uint32_t mspLon,
											   uint16_t altitudeInMeters,
											   uint16_t speed,
											   uint16_t groundCourse,
											   std::string craftName)
	{
		U_ID_0 = uid0;
		U_ID_1 = uid1;
		U_ID_2 = uid2;

		FixType = fixType;
		NumSat = numSat;
		MspLat = mspLat;
		MspLon = mspLon;
		AltitudeInMeters = altitudeInMeters;
		Speed = speed;
		GroundCourseInDecidegrees = groundCourse;

		CraftName = craftName;
	}

	CraftServices::GeoSpatialPoint CraftInfoAndPosition::GetGeoSpatialPoint()
	{
		// Using MSP native units here
		return CraftServices::GeoSpatialPoint(MspLat, MspLon);
	}

	std::string CraftInfoAndPosition::GetCompleteCraftLocationString(bool omitExactGpsPos)
	{
		std::ostringstream outString;

		CraftServices::GeoSpatialPoint position = CraftInfoAndPosition::GetGeoSpatialPoint();
		std::string latLonString = CraftServices::GeoSpatialUtil::GetLatLonString(omitExactGpsPos, position.MspLat, position.MspLon);
		std::string uidString = CraftServices::UidUtil::UIDToHexString(U_ID_0, U_ID_1, U_ID_2);

		outString /*<< uidString << ": " */ << CraftName << " " << latLonString;
		return outString.str();
	}

} // Namespace CraftServices