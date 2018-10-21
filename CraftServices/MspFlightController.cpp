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

#include "MspFlightController.hpp"
#include "msp.hpp"
#include "msp_msg.hpp"

namespace msp
{

	// NO, dupe this into Aysnc version. 

	// Default constructor
	MSPFlightController::MSPFlightController()
	{
	}

	// Constructor from port name to monitor
	MSPFlightController::MSPFlightController(std::string portName)
	{

	}

	// Constructor from existing MSP connection
	MSPFlightController::MSPFlightController(msp::MSP * pTheMsp)
	{
		assert(pTheMsp != NULL);
		pMsp = pTheMsp;

		// TODO - Do remaining setup and initialization. What variant, what API version, etc, etc.
	}

	// Destructor
	MSPFlightController::~MSPFlightController()
	{
		delete pMsp;
		pMsp = NULL;
	}


	bool MSPFlightController::GetFcVariant()
	{
		// First attempt at message
		msp::msg::FcVariant fcVariantRequest;
		bool result = pMsp->request(fcVariantRequest);
		if (result)
		{
			std::cout << "FC Variant: " << fcVariantRequest.identifier;
		}
		else
		{
			std::cout << "Problem receiving response from GetFcVariant";
		}
		std::cout << endl;
		return result;
	}

	void MSPFlightController::GetApiVersion()
	{
		// First attempt at message
		msp::msg::ApiVersion apiVersionRequest;
		bool result = pMsp->request(apiVersionRequest);
		if (result)
		{
			std::cout << "MSP Version " << apiVersionRequest.major << "." << apiVersionRequest.minor << "." << apiVersionRequest.protocol;
		}
		else
		{
			std::cout << "Problem receiving response from ApiVersion";
		}
		std::cout << endl;
	}

	void MSPFlightController::GetRawGPS()
	{
		//cout << "GetRawGPS" << endl;
		// First attempt at message
		msp::msg::RawGPS rawGpsRequest;
		bool result = pMsp->request(rawGpsRequest);
		//cout << "Got result from GetRawGPS" << endl;
		if (result)
		{
			std::cout << "Port " << pMsp->getPortName() << ": GPS position Lat " << rawGpsRequest.lat << " Lon " << rawGpsRequest.lon << " Alt " << rawGpsRequest.altitude;
		}
		else
		{
			std::cout << "Problem receiving response from GetRawGPS";
		}
		std::cout << endl;
	}

}