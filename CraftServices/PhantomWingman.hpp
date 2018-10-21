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

#ifndef PHANTOM_WINGMAN_HPP
#define PHANTOM_WINGMAN_HPP

#include <cstdlib>
#include <string>
#include "PhantomTestCraft.hpp"
#include "GeoSpatialPoint.hpp"


namespace CraftServices
{
	class PhantomWingman : public CraftServices::PhantomTestCraft
	{
	public:

		// Portname this Phantom Wingman is associated with
		std::string TargetPortName;

		// Offsets from reference craft
		double AngleFromSourceCraft;
		double HorizontalDistanceInMetersFromSourceCraft;
		double RelativeAltitudeDifferenceInMetersFromSourceCraft;

		// Last known position of the reference craft
		bool ReferenceCraftInfoAndPositionSet;
		CraftInfoAndPosition ReferenceCraftInfoAndPosition;
		// Is that last known position stale?
		bool ReferenceCraftPositionIsStale;

		// Constructor
		PhantomWingman(std::string portName, 
					   double angleFromSourceCraft, 
					   double horizontalDistanceInMetersFromSourceCraft, 
			           double relativeAltitudeDifferenceInMetersFromSourceCraft);

		// Destructor
		virtual ~PhantomWingman();

		virtual void UpdateReferenceCraftPosition(CraftInfoAndPosition & referenceCraftInfoAndPosition, bool craftPositionIsStale);

		virtual bool IsEligibleToBeSent(const std::string & portName, std::string & eligibilityMessage);

		virtual std::string GetParametersAsString();

		// Get the current position information for the craft
		virtual CraftInfoAndPosition GetCurrentCraftPosition();

	private:

		static double GetAdjustedAngleForPhantomWingman(uint16_t groundCourse, double angleFromSourceCraft);
		CraftServices::GeoSpatialPoint GetCurrentLocationOfPhantomWingman();
		uint16_t PhantomWingman::GetAbsoluteAltitudeOfPhantomCraft();

	};
}


#endif GEOSPATIAL_POINT_HPP
