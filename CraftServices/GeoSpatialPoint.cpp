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

#include "GeoSpatialPoint.hpp"
#include "GeoSpatialUtil.hpp"
#include "NotImplementedException.hpp"

namespace CraftServices
{

// Constructor from MSP native coordinates
GeoSpatialPoint::GeoSpatialPoint(uint32_t mspLat, uint32_t mspLon) : MspLat(mspLat), MspLon(mspLon)
{
}

// Constructor from decimal degrees
GeoSpatialPoint::GeoSpatialPoint(double lat, double lon)
{
	MspLat = CraftServices::GeoSpatialUtil::ConvertDecimalDegreeToMspGpsValue(lat);
	MspLon = CraftServices::GeoSpatialUtil::ConvertDecimalDegreeToMspGpsValue(lon);
}

// Constructor from string with decimal degrees ("45.56890", "-122.714209")
GeoSpatialPoint::GeoSpatialPoint(std::string lat, std::string lon)
{
	MspLat = GeoSpatialUtil::ConvertStringDegreeToMspGpsValue(lat);
	MspLon = GeoSpatialUtil::ConvertStringDegreeToMspGpsValue(lon);
}

// Destructor
GeoSpatialPoint::~GeoSpatialPoint()
{

}

double GeoSpatialPoint::LatInDecimalDegrees()
{
	return CraftServices::GeoSpatialUtil::ConvertMspGpsValueToDecimalDegree(MspLat);
}

double GeoSpatialPoint::LonInDecimalDegrees()
{
	return CraftServices::GeoSpatialUtil::ConvertMspGpsValueToDecimalDegree(MspLon);
}

std::string GeoSpatialPoint::GetLatLonStringForPoint()
{
	std::ostringstream outString;
	outString << "Lat# " << CraftServices::GeoSpatialUtil::GetMspGpsDegreeValueAsString(MspLat) << " Lon# " << CraftServices::GeoSpatialUtil::GetMspGpsDegreeValueAsString(MspLon);
	return outString.str();
}


} // Namespace CraftServices