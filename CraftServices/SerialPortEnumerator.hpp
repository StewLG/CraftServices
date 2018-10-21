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

#ifndef SERIALPORTENUMERATOR_HPP
#define SERIALPORTENUMERATOR_HPP

#include <vector>
#include <string>
#include <regex>

#include <boost/algorithm/string.hpp>
//#include <boost/range/as_array.hpp>

#ifdef WIN32
#include <Windows.h>
#endif

#include "NotImplementedException.hpp"

class SerialPortEnumerator
{
public:

	static std::vector<std::string> GetSerialPortNames()
	{
		std::vector<std::string> serialPortNames;

		#ifdef WIN32
		serialPortNames = GetSerialPortNamesForWin32();
		#elif
		throw NotImplemented();
		#endif	
		 	
		return serialPortNames;
	}

private:
	
	#ifdef WIN32
	// Windows-specific implementation
	static std::vector<std::string> GetSerialPortNamesForWin32()
	{
		char input_array[65536];

		QueryDosDeviceA(NULL, input_array, sizeof(input_array));

		std::vector<std::string> SerialPortNames;

		// QueryDosDeviceA will return us a big buffer of null-terminated strings, 
		// with a final single null at the end of the list. Here we parse this
		// into a list of strings that match "comXX" where XX is a number.
		char * pArray = input_array;
		std::regex comRegex("com[0-9]+", std::regex_constants::icase);
		std::string currentString;
		do 
		{
			currentString = std::string(pArray);
			if (regex_search(currentString, comRegex))
			{
				SerialPortNames.push_back(currentString);
			}
			// Advance pointer in buffer of strings
			pArray += currentString.size() + 1;
		} while (currentString.size() > 0);

		// Would need to sort by numeric portion only as number; TBD.
		//std::sort(SerialPortNames.begin(), SerialPortNames.end());

		return SerialPortNames;
	}
	#endif // WIN32
	
};

#endif // SERIALPORTENUMERATOR_HPP




