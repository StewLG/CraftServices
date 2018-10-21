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

#ifndef PHANTOMTESTCRAFTFIXED_HPP
#define PHANTOMTESTCRAFTFIXED_HPP

#include "CraftInfoAndPosition.hpp"
#include "PhantomTestCraft.hpp"
#include "GeoSpatialUtil.hpp"
#include "GeoSpatialPoint.hpp"

namespace CraftServices
{
	// A fake Craft for testing with a FIXED position.
	// 
	// Once set, this Craft will return an immobile Craft position. 
	// (Imagine a perfect quad with infinite battery life, hovering indefinitely.)
	class PhantomTestCraftFixed : public CraftServices::PhantomTestCraft
	{
		public:

			// Since position is fixed for this class, we can define these once and give them
			// values.
			uint8_t FixType;
			uint8_t NumSat;
			uint32_t MspLat;
			uint32_t MspLon;
			// Pretty dang sure this is meters, but surprises continue so stay wary!
			uint16_t AltitudeInMeters;  
			uint16_t Speed;
			// Decidegrees
			uint16_t GroundCourseInDecidegrees;

			// Minimal Constructor
			PhantomTestCraftFixed(std::string craftName) : PhantomTestCraft(craftName)
			{
				// Phantom craft has good GPS lock
				FixType = GPSFixType::GPS_FIX_3D;
				// GPS always will work perfectly in test-land. Using a ridiculously high
				// value to remind us this is synthetic.
				NumSat = CraftServices::PhantomTestCraft::VeryHighSatelliteCount;

				// We aren't moving, so by definition speed is 0
				Speed = 0;
				// Ground course in decidegrees. 0 is north, range = 0-3590.
				GroundCourseInDecidegrees = 0;
			}

			// Destructor
			virtual ~PhantomTestCraftFixed() 
			{
			}

			// Set the fixed position using MSP native-type values
			void SetFixedPositionWithMspNative(uint32_t lat, uint32_t lon, uint16_t altitudeInMeters, uint16_t groundCourseInDecidegrees)
			{
				MspLat = lat;
				MspLon = lon;
				AltitudeInMeters = altitudeInMeters;
				GroundCourseInDecidegrees = groundCourseInDecidegrees;
			}

			// Use the GeoSpatial point to set the fixed position
			void SetFixedPositionWithGeoSpatialPoint(CraftServices::GeoSpatialPoint geoSpatialPoint, uint16_t altitudeInMeters, uint16_t groundCourseInDecidegrees)
			{
				MspLat = geoSpatialPoint.MspLat;
				MspLon = geoSpatialPoint.MspLon;
				AltitudeInMeters = altitudeInMeters;
				GroundCourseInDecidegrees = groundCourseInDecidegrees;
			}

			// Is this PhantomTestCraft currently eligible to be sent to the given port?
			bool IsEligibleToBeSent(const std::string & portName, std::string & eligibilityMessage)
			{
				// Fixed craft don't have port affiliation yet, so, always eligible
				eligibilityMessage = "Fixed craft are always eligible";
				return true;
			}

			void UpdateReferenceCraftPosition(CraftInfoAndPosition & referenceCraftInfoAndPosition, bool craftPositionIsStale)
			{
				// Does nothing for a fixed craft
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
				positionToReturn.MspLat = MspLat;
				positionToReturn.MspLon = MspLon;
				positionToReturn.AltitudeInMeters = AltitudeInMeters;
				positionToReturn.Speed = Speed;
				positionToReturn.GroundCourseInDecidegrees = GroundCourseInDecidegrees;

				return positionToReturn;
			}	

			// Get a dump of the parameters as a string
			std::string GetParametersAsString() 
			{
				return "[TODO - add implementation]";
			}

	};

} // namespace CraftServices

#endif // PHANTOMTESTCRAFTFIXED_HPP



