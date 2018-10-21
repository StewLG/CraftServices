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

#include "PhantomWingman.hpp"
#include "GeoSpatialUtil.hpp"
#include "CraftServiceException.hpp"

namespace CraftServices
{
	// Constructor
	PhantomWingman::PhantomWingman(std::string targetPortName, double angleFromSourceCraft, double horizontalDistanceInMetersFromSourceCraft, double relativeAltitudeDifferenceInMetersFromSourceCraft) : PhantomTestCraft("phwing_" + targetPortName)
	{
		// Do we need to init anything else, UID? TODO
		TargetPortName = targetPortName;
		AngleFromSourceCraft = angleFromSourceCraft;
		HorizontalDistanceInMetersFromSourceCraft = horizontalDistanceInMetersFromSourceCraft;
		RelativeAltitudeDifferenceInMetersFromSourceCraft = relativeAltitudeDifferenceInMetersFromSourceCraft;
		ReferenceCraftInfoAndPositionSet = false;
		ReferenceCraftPositionIsStale = false;
	}

	// Destructor
	PhantomWingman::~PhantomWingman()
	{
	}

	// Is this PhantomTestCraft currently eligible to be sent to the given port?
	bool PhantomWingman::IsEligibleToBeSent(const std::string & portName, std::string & eligibilityMessage)
	{
		std::string lowerPortName = boost::to_lower_copy(portName);
		std::string lowerTargetPortName = boost::to_lower_copy(TargetPortName);
		bool hasMatchingPortName =  (lowerPortName == lowerTargetPortName || lowerTargetPortName == "all");

		if (!hasMatchingPortName)
		{
			eligibilityMessage = "Phantom Wingman does not have matching port name. " + lowerPortName + " != " + lowerTargetPortName + " ";
		}

		if (ReferenceCraftPositionIsStale)
		{
			eligibilityMessage += "Phantom Wingman target Craft Position is stale. ";
		}

		return hasMatchingPortName && !ReferenceCraftPositionIsStale;
	}

	// Get a dump of the parameters as a string
	std::string PhantomWingman::GetParametersAsString()
	{
		/*
		TargetPortName = targetPortName;
		AngleFromSourceCraft = angleFromSourceCraft;
		HorizontalDistanceInMetersFromSourceCraft = horizontalDistanceInMetersFromSourceCraft;
		RelativeAltitudeDifferenceInMetersFromSourceCraft = relativeAltitudeDifferenceInMetersFromSourceCraft;
		*/
		return fmt::format("Port {}, {} deg rotation, {} meters distant, {} meters alt difference", TargetPortName, AngleFromSourceCraft, HorizontalDistanceInMetersFromSourceCraft, RelativeAltitudeDifferenceInMetersFromSourceCraft);
	}


	// Update the location of the reference craft.
	void PhantomWingman::UpdateReferenceCraftPosition(CraftInfoAndPosition & referenceCraftInfoAndPosition, bool craftPositionIsStale)
	{
		ReferenceCraftInfoAndPosition = referenceCraftInfoAndPosition;
		ReferenceCraftInfoAndPositionSet = true;
		ReferenceCraftPositionIsStale = craftPositionIsStale;
	}

	// Get the current position information for the craft
	CraftInfoAndPosition PhantomWingman::GetCurrentCraftPosition()
	{
		// Be sure to regularly refresh the reference craft's position
		if (!ReferenceCraftInfoAndPositionSet)
		{
			std::string errorMessage = std::string("Reference craft position never set for Phantom Wingman, ") + std::string(TargetPortName);
			std::cout << errorMessage;
			throw CraftServiceException(errorMessage);
		}

		// Don't try to use it if it is stale (for the moment I can't think why you would)
		if (ReferenceCraftPositionIsStale)
		{
			std::string errorMessage = std::string("Reference craft position is stale for Phantom Wingman, ") + std::string(TargetPortName);
			std::cout << errorMessage;
			throw CraftServiceException(errorMessage);
		}

		// Get our offset GPS coordinates
		CraftServices::GeoSpatialPoint phantomWingmanCurrentLocation = GetCurrentLocationOfPhantomWingman();
		// Get our absolute (above sea level) altitude
		double phantomWingmanAbsoluteAlt = GetAbsoluteAltitudeOfPhantomCraft();

		// Build up new, adjusted position
		CraftInfoAndPosition newCraftInfoAndPosition(UID_0, UID_1, UID_2,
			GPS_FIX_3D, CraftServices::PhantomTestCraft::VeryHighSatelliteCount,
			phantomWingmanCurrentLocation.MspLat,
			phantomWingmanCurrentLocation.MspLon,
			phantomWingmanAbsoluteAlt,
			// We track the reference craft in speed and ground course exactly
			ReferenceCraftInfoAndPosition.Speed,
			ReferenceCraftInfoAndPosition.GroundCourseInDecidegrees,
			CraftName);

		// Give back position
		return newCraftInfoAndPosition;
	}

	// Private
	// -------

	double PhantomWingman::GetAdjustedAngleForPhantomWingman(uint16_t groundCourseInDecidegrees, double angleFromSourceCraft)
	{
		// Convert ground course to regular degrees
		double groundCourseInDegrees = groundCourseInDecidegrees / 10;
		// Get adjusted angle (which will always be relative to the craft we are tracking)
		double adjustedAngle = groundCourseInDegrees + angleFromSourceCraft;
		// Normalize to 0..360
		adjustedAngle = CraftServices::GeoSpatialUtil::NormalizeDegreeRotation(adjustedAngle);
		return adjustedAngle;
	}

	// Get the Phantom Wingman's current lat lon, based on the source craft's position
	CraftServices::GeoSpatialPoint PhantomWingman::GetCurrentLocationOfPhantomWingman()
	{
		// Get reference position of the craft we are tracking
		GeoSpatialPoint sourceCraftLocation = ReferenceCraftInfoAndPosition.GetGeoSpatialPoint();
		// Adjust the angle to be relative to the source craft (watch out for decidegrees here)
		auto theAdjustedAngle = GetAdjustedAngleForPhantomWingman(ReferenceCraftInfoAndPosition.GroundCourseInDecidegrees, AngleFromSourceCraft);
		// Return the adjusted position
		return CraftServices::GeoSpatialUtil::GetDestinationPoint(sourceCraftLocation, theAdjustedAngle, HorizontalDistanceInMetersFromSourceCraft);
	}

	// Get the Phantom Wingman's current absolute altitude in meters, based on the source craft's absolute altitude in meters
	uint16_t PhantomWingman::GetAbsoluteAltitudeOfPhantomCraft()
	{
		// I believe altitude is transmitted as an unsigned over the wire, but it is in fact a signed int. Correct me if I'm wrong! -- SLG
		int16_t signedAbsoluteAltitude = static_cast<int16_t>(ReferenceCraftInfoAndPosition.AltitudeInMeters) + RelativeAltitudeDifferenceInMetersFromSourceCraft;
		// returning as unsigned for implicit cast
		return signedAbsoluteAltitude;
	}


} // Namespace CraftServices