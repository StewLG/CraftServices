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
 
#ifndef UID_UTIL_HPP
#define UID_UTIL_HPP

#include <string>
#include <iostream> 
#include <sstream>  
#include <random>

namespace CraftServices
{
	namespace UidUtil
	{
		// We use a deterministic seed here because we'd like to be able to reproduce any problems.
		// (We need entropy in our generated UIDs, but don't want actual randomness.)
		static std::mt19937 merTwister(1729);

		static void MakeUID(uint32_t & uid_0, uint32_t & uid_1, uint32_t & uid_2)
		{
			static std::uniform_int_distribution<uint32_t> uniformIntDist(UINT32_MAX/2, UINT32_MAX-1);
			uid_0 = uniformIntDist(merTwister);
			uid_1 = uniformIntDist(merTwister);
			uid_2 = uniformIntDist(merTwister);
		}

		template< typename T >
		std::string IntToHex(T i, bool prefixWithZeroX = true)
		{
			std::stringstream stream;
			if (prefixWithZeroX)
			{
				stream << "0x";
			}
			stream << std::setfill('0') << std::setw(sizeof(T) * 2)
				// This is awkward and error prone. May need to resort to straight C.
				<< std::hex << (int)i;
			return stream.str();
		}

		// Probably belongs elsewhere; move?
		static std::string UIDToHexString(const uint32_t U_ID_0, const uint32_t U_ID_1, const uint32_t U_ID_2)
		{
			std::ostringstream outString;
			outString << IntToHex(U_ID_0, true) << IntToHex(U_ID_1, false) << IntToHex(U_ID_2, false);
			return outString.str();
		}

	} // Namespace UidUtil
} // Namespace CraftServices

#endif // UID_UTIL_HPP




