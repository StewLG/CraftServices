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

#ifndef PhantomTestCraftFlex_HPP
#define PhantomTestCraftFlex_HPP

#include "CraftInfoAndPosition.hpp"
#include "PhantomTestCraft.hpp"
#include "GeoSpatialUtil.hpp"
#include "GeoSpatialPoint.hpp"

namespace CraftServices
{
	// Movable phantom test craft
	class PhantomTestCraftFlex : public CraftServices::PhantomTestCraft
	{
		public:

			// Since position is fixed for this class, we can define these once and give them
			// values.
			uint8_t FixType;
			uint8_t NumSat;

			//uint32_t CurrentLat;
			//uint32_t CurrentLon;
			//uint16_t CurrentAltitude;

			// These should be calculated..
			//uint16_t Speed;
			// Decidegrees
			//uint16_t GroundCourseInDecidegrees;

			

			// Minimal Constructor
			PhantomTestCraftFlex(std::string craftName) : PhantomTestCraft(craftName)
			{
				// Phantom craft has good GPS lock
				FixType = GPSFixType::GPS_FIX_3D;
				// GPS always will work perfectly in test-land. Using a ridiculously high
				// value to remind us this is synthetic.
				NumSat = 50;
				/*
				// We aren't moving, so by definition speed is 0
				Speed = 0;
				// Ground course in decidegrees. 0 is north, range = 0-3590.
				GroundCourseInDecidegrees = 0;
				*/
			}

			// Destructor
			virtual ~PhantomTestCraftFlex() { }

			// Set the fixed position using MSP native-type values
			void SetFixedPositionWithMspNative(uint32_t lat, uint32_t lon, uint16_t altitude, uint16_t groundCourseInDecidegrees)
			{
				CurrentLat = lat;
				CurrentLon = lon;
				CurrentAltitude = altitude;
				//GroundCourseInDecidegrees = groundCourseInDecidegrees;
			}

			// Use the GeoSpatial point to set the fixed position
			void SetFixedPositionWithGeoSpatialPoint(CraftServices::GeoSpatialPoint geoSpatialPoint, uint16_t altitude, uint16_t groundCourseInDecidegrees)
			{
				Lat = geoSpatialPoint.MspLat;
				Lon = geoSpatialPoint.MspLon;
				Altitude = altitude;
				GroundCourseInDecidegrees = groundCourseInDecidegrees;
			}

			bool IsMoving()
			{
				return false;
			}

			// Get the current position information for the craft
			CraftInfoAndPosition GetCurrentCraftPosition()
			{
				CraftInfoAndPosition positionToReturn;

				// UID set in PhantomTestCraft automatically
				positionToReturn.U_ID_0 = UID_0;
				positionToReturn.U_ID_1 = UID_1;
				positionToReturn.U_ID_2 = UID_2;

				// Set in base class
				positionToReturn.CraftName = CraftName;

				// Set in this class
				positionToReturn.FixType = FixType;
				positionToReturn.NumSat = NumSat;
				positionToReturn.Lat = Lat;
				positionToReturn.Lon = Lon;
				positionToReturn.Altitude = Altitude;
				positionToReturn.Speed = Speed;
				positionToReturn.GroundCourseInDecidegrees = GroundCourseInDecidegrees;

				return positionToReturn;
			}	



	};

} // namespace CraftServices

#endif // PhantomTestCraftFlex_HPP



