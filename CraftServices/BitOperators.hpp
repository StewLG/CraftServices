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

#ifndef BITOPERATORS_HPP
#define BITOPERATORS_HPP

using namespace std;
#include <cstdint>
#include <vector>

class BitOperators
{
	public:

		static uint8_t LowByte(uint16_t value)
		{
			return (uint8_t)value;
		}

		static uint8_t HighByte(uint16_t value)
		{
			return (uint8_t)(value >> 8);
		}

		static uint16_t MakeUint16(uint8_t lowByte, uint8_t highByte)
		{
			uint16_t wd = ((uint16_t) highByte << 8) | lowByte;
			return wd;
		}

		static std::vector<uint8_t> GetUint32AsByteVector(uint32_t value)
		{
			std::vector<uint8_t> byteVector;

			// Should be little endian
			byteVector.push_back(value);
			byteVector.push_back(value >> 8);
			byteVector.push_back(value >> 16);
			byteVector.push_back(value >> 24);

			return byteVector;
		}

		static uint32_t GetByteVectorAsUint32(std::vector<uint8_t> byteVector, int startingIndex)
		{
			uint32_t value = 0;

			value |= byteVector[startingIndex] << 24;
			value |= byteVector[startingIndex + 1] << 16;
			value |= byteVector[startingIndex + 2] << 8;
			value |= byteVector[startingIndex + 3];
			return value;
		}
};

#endif // BITOPERATORS_HPP
