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

#ifndef GEOSPATIAL_POINT_HPP
#define GEOSPATIAL_POINT_HPP

#include <cstdlib>

namespace CraftServices
{
	class GeoSpatialPoint
	{
	public:
		// Stored as native MSP coordinates for now
		uint32_t MspLat;
		uint32_t MspLon;

		// Constructor from MSP native coordinates
		GeoSpatialPoint(uint32_t mspLat, uint32_t mspLon);

		// Constructor from decimal degrees (45.56890, -122.714209)
		GeoSpatialPoint(double lat, double lon);

		// Constructor from string with decimal degrees ("45.56890", "-122.714209")
		GeoSpatialPoint(std::string lat, std::string lon);

		// Destructor
		virtual ~GeoSpatialPoint();

		double LatInDecimalDegrees();
		double LonInDecimalDegrees();

		std::string GetLatLonStringForPoint();

		// TODO: Support setting with human-readable values "45.1112", "-122.93933", etc. This will
		// be needed when processing from command line or otherwise.


	private:

	};
}


#endif GEOSPATIAL_POINT_HPP
