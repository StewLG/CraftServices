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

#ifndef DESERIALIZE_HPP
#define DESERIALIZE_HPP

#include "CraftServicesTypes.hpp"
#include <cstdlib>

#define GPS_DEGREES_DIVIDER 10000000L

namespace CraftServices 
{
	/////////////////////////////////////////////////////////////////////
	/// serialization/deserialization for unsigned integers

	// (The uint8_t functions aren't needed strictly speaking, but it's
	// nice to have a consistent interface.)

	static void serialize_uint8(const uint8_t val, ByteVector &data)
	{
		data.push_back(val);
	}

	static uint8_t deserialize_uint8(const ByteVector &data, const size_t start)
	{
		return (data[start]);
	}

	static void serialize_uint16(const uint16_t val, ByteVector &data)
	{
		data.push_back(val >> 0);
		data.push_back(val >> 8);
	}

	static uint16_t deserialize_uint16(const ByteVector &data, const size_t start)
	{
		return (data[start] << 0) | (data[start + 1] << 8);
	}

	static void serialize_int16(const int16_t val, ByteVector &data)
	{
		data.push_back(val >> 0);
		data.push_back(val >> 8);
	}

	static int16_t deserialize_int16(const ByteVector &data, const size_t start)
	{
		return (data[start] << 0) | (data[start + 1] << 8);
	}

	static int32_t deserialize_int32(const ByteVector &data, const size_t start)
	{
		return (data[start] << 0) | (data[start + 1] << 8) | (data[start + 2] << 16) | (data[start + 3] << 24);
	}

	static void serialize_uint32(const uint32_t val, ByteVector &data)
	{
		data.push_back(val >> 0);
		data.push_back(val >> 8);
		data.push_back(val >> 16);
		data.push_back(val >> 24);
	}

	static uint32_t deserialize_uint32(const ByteVector &data, const size_t start)
	{
		return (data[start] << 0) | (data[start + 1] << 8) | (data[start + 2] << 16) | (data[start + 3] << 24);
	}

	static inline const std::string BoolToString(bool b)
	{
		return b ? "True" : "False";
	}


}

#endif // DESERIALIZE_HPP
