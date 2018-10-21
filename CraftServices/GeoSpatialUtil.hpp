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

#ifndef GEOSPATIAL_UTIL_HPP
#define GEOSPATIAL_UTIL_HPP

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include "GeoSpatialPoint.hpp"

// Spdlog here because it drags fmt library along with it.
#include "spdlog/spdlog.h"

#define GPS_DEGREES_DIVIDER 10000000L
#define RADIUS_EARTH_IN_METERS 6371e3
#define GEO_PI 3.141592653589793
#define CENTIMETERS_PER_METER 100

namespace CraftServices 
{
	namespace GeoSpatialUtil
	{
		static double DegreesToRadians(double degrees)
		{
			double radians = (degrees * GEO_PI) / 180;
			return radians;
		}

		static double RadiansToDegrees(double radians)
		{
			double degrees = (radians * 180) / GEO_PI;
			return degrees;
		}

		static double ConvertMspGpsValueToDecimalDegree(uint32_t mspGpsDegreeValue)
		{
			return ((int32_t)mspGpsDegreeValue) / ((double)GPS_DEGREES_DIVIDER);
		}

		static std::string GetMspGpsDegreeValueAsString(uint32_t mspGpsDegreeValue)
		{
			std::ostringstream outString;
			outString << std::setprecision(8) << ConvertMspGpsValueToDecimalDegree(mspGpsDegreeValue);
			return outString.str();
		}

		static uint32_t ConvertDecimalDegreeToMspGpsValue(double decimalDegree)
		{
			return (uint32_t)(decimalDegree * ((double)GPS_DEGREES_DIVIDER));
		}

		static uint32_t ConvertStringDegreeToMspGpsValue(std::string decimalDegreeAsString)
		{
			double decimalDegree = ::atof(decimalDegreeAsString.c_str());
			return (uint32_t)(decimalDegree * ((double)GPS_DEGREES_DIVIDER));
		}

		// Normalize a degree rotation to the 0..360 range
		static double NormalizeDegreeRotation(double rotation)
		{
			double adjustedRotation = std::fmod(rotation, 360);
			if (adjustedRotation < 0)
			{
				adjustedRotation += 360;
			}
			return adjustedRotation;
		}
		
		static std::string GetDecidegreesAsDegreeString(int16_t deciDegrees)
		{
			// unfortunately ° symbol doesn't immediately work with fmt library, so we say "deg" for now.
			std::string degreeString = fmt::format("{} deg", deciDegrees / 10);
			return degreeString;
		}

		static std::string GetHDOPAsString(uint16_t hdop)
		{
			std::string degreeString = fmt::format("{}", (float)hdop / 100);
			return degreeString;
		}

		static CraftServices::GeoSpatialPoint GetDestinationPoint(CraftServices::GeoSpatialPoint currentPoint,
			double destinationBearingInDegrees,
			uint32_t destinationDistanceInMeters)
		{
			double radiusOfEarth = 6371000;

			// sinφ2 = sinφ1⋅cosδ + cosφ1⋅sinδ⋅cosθ
			// tanΔλ = sinθ⋅sinδ⋅cosφ1 / cosδ−sinφ1⋅sinφ2
			// see mathforum.org/library/drmath/view/52049.html for derivation

			// angular distance in radians
			double delta = destinationDistanceInMeters / radiusOfEarth;
			double theta = DegreesToRadians(destinationBearingInDegrees);

			double phi1 = DegreesToRadians(currentPoint.LatInDecimalDegrees());
			double lambda1 = DegreesToRadians(currentPoint.LonInDecimalDegrees());

			double sinPhi1 = sin(phi1);
			double cosPhi1 = cos(phi1);
			double sinDelta = sin(delta);
			double cosDelta = cos(delta);
			double sinTheta = sin(theta);
			double cosTheta = cos(theta);

			double sinphi2 = sinPhi1 * cosDelta + cosPhi1 * sinDelta*cosTheta;
			double phi2 = asin(sinphi2);
			double y = sinTheta * sinDelta * cosPhi1;
			double x = cosDelta - sinPhi1 * sinphi2;
			double lambda2 = lambda1 + atan2(y, x);

			//return new LatLon(phi2.toDegrees(), (lambda2.toDegrees() + 540) % 360 - 180); // normalise to −180..+180°

			double destinationLat = RadiansToDegrees(phi2);
			// Normalize to -180..+180
			double lamba2InDegrees = RadiansToDegrees(lambda2);
			double destinationLon = std::fmod((lamba2InDegrees + 540), 360) - 180;

			CraftServices::GeoSpatialPoint destinationPoint(destinationLat, destinationLon);
			return destinationPoint;
		}

		// Written first, but used second version first, so, destined for scrap pile.
		/*
		static GeoSpatialPoint GetDestinationPointGivenDistanceAndBearingFromStartPoint(uint32_t latStartMsp_XX, uint32_t lonStartMsp_YY, uint32_t distanceInMeters, uint8_t bearing)
		{
			// With help from https://www.movable-type.co.uk/scripts/latlong.html
			double lat1_InRadians = DegreesToRadians(ConvertMspGpsValueToDecimalDegree(latStartMsp_XX));
			double lon1_InRadians = DegreesToRadians(ConvertMspGpsValueToDecimalDegree(lonStartMsp_YY));

			double angularDistanceInRadians = distanceInMeters / RADIUS_EARTH_IN_METERS;
			double bearingInRadians = DegreesToRadians(bearing);

			double sin_lat1 = sin(lat1_InRadians);
			double cos_lat1 = cos(lat1_InRadians);
			double sin_angdist = sin(angularDistanceInRadians);
			double cos_angdist = cos(angularDistanceInRadians);
			double sin_bearingrad = sin(bearingInRadians);
			double cos_bearingrad = cos(bearingInRadians);

			double sin_lat2 = sin_lat1*cos_angdist + cos_lat1*sin_angdist*cos_bearingrad;
			double lat2_InRadians = asin(sin_lat2);
			double y = sin_bearingrad * sin_angdist * cos_lat1;
			double x = cos_angdist - sin_lat1 * sin_lat2;
			double lon2_InRadians = lon1_InRadians + atan2(y, x);
			
			GeoSpatialPoint destinationPoint(RadiansToDegrees(lat2_InRadians), RadiansToDegrees(lon2_InRadians));

			// For debugging; remove eventually
			std::cout << "Lat2: " << std::setprecision(8) << RadiansToDegrees(lat2_InRadians) << " Lon2: " << std::setprecision(8) << RadiansToDegrees(lon2_InRadians) << std::endl;
			std::cout << "When printed from GeoSpatialPoint (should be same as above): " << destinationPoint.GetLatLonStringForPoint() << std::endl;
			return destinationPoint;
		}
		*/

		static std::string GetLatLonString(bool omitExactGspLocation, uint32_t mspLat, uint32_t mspLon)
		{
			std::ostringstream outString;
			outString << GetMspGpsDegreeValueAsString(mspLat) << ", " << GetMspGpsDegreeValueAsString(mspLon);
			// Some GPSes will show 0's for position when they don't have one yet, and we yet that leak through regardless. It's helpful,
			// but does not reveal position in the logs.
			if (omitExactGspLocation && (mspLat != 0 || mspLon != 0))
			{
				return "XX.XXXX, YY.YYYY";
			}

			return outString.str();
		}

		static int32_t MetersToCentimeters(int32_t meters)
		{
			return meters * CENTIMETERS_PER_METER;
		}
	}
}

#endif // GEOSPATIAL_UTIL_HPP
