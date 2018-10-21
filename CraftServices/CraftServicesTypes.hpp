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

#ifndef CRAFT_SERVICES_TYPES_HPP
#define CRAFT_SERVICES_TYPES_HPP

#include <vector>
#include <stdint.h>
//#include "msp_id.hpp"

namespace CraftServices 
{

/**
 * @brief ByteVector vector of bytes
 */
typedef std::vector<uint8_t> ByteVector;


/////////////////////////////////////////////////////////////////////
/// Generic message types

// struct Message 
// {
    // virtual ID id() const = 0;

    // virtual ~Message() { }
// };

// // send to FC
// struct Request : public Message 
// {
    // virtual void decode(const ByteVector &data) = 0;
// };

// // received from FC
// struct Response : public Message 
// {
    // virtual ByteVector encode() const = 0;
// };

} // namespace CraftServices

#endif // CRAFT_SERVICES_TYPES_HPP
