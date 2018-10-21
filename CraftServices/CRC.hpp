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
using namespace std;
#include <cstdint>

class Crc
{
	public:

	static uint8_t crc8_dvb_s2(uint8_t crc, unsigned char c)
	{
		crc ^= c;
		for (int ii = 0; ii < 8; ++ii) 
		{
			if (crc & 0x80) 
			{
				crc = (crc << 1) ^ 0xD5;
			}
			else 
			{
				crc = crc << 1;
			}
		}
		return crc;
	}

	static uint8_t crc8_dvb_s2_update(uint8_t crc, const void *data, uint32_t length)
	{
		const uint8_t *p = (const uint8_t *)data;
		const uint8_t *pend = p + length;

		for (; p != pend; p++) 
		{
			crc = crc8_dvb_s2(crc, *p);
		}
		return crc;
	}

	/*
	static void crc8_dvb_s2_sbuf_append(sbuf_t *dst, uint8_t *start)
	{
		uint8_t crc = 0;
		const uint8_t * const end = dst->ptr;
		for (const uint8_t *ptr = start; ptr < end; ++ptr) 
		{
			crc = crc8_dvb_s2(crc, *ptr);
		}
		sbufWriteU8(dst, crc);
	}
	*/

};