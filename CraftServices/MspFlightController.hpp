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

#ifndef MSPFLIGHTCONTROLLER_HPP
#define MSPFLIGHTCONTROLLER_HPP

#include <vector>
#include <string>
#include <regex>

#include <boost/algorithm/string.hpp>
#include "NotImplementedException.hpp"
#include "msp.hpp"

namespace msp
{
	// A single instance of a communications channel with an MSP-speaking flight controller (iNav, Betaflight, etc.)
	class MSPFlightController
	{
	public:
		// Our MSP connection object
		msp::MSP * pMsp;

		// Information about this connection - what flight controller type is it? 
		std::string FcVariantString;

		// Default constructor
		MSPFlightController();
		// Constructor from port name to monitor
		MSPFlightController(std::string portName);
		// Constructor from existing MSP session
		MSPFlightController(msp::MSP * pTheMsp);

		// Destructor
		~MSPFlightController();

		bool GetFcVariant();
		void GetApiVersion();		
		void GetRawGPS();

	private:

	};
}

#endif // MSPFLIGHTCONTROLLER_HPP


