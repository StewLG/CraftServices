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

#ifndef CRAFT_SERVICE_EXCEPTION_HPP
#define CRAFT_SERVICE_EXCEPTION_HPP

#include <stdexcept>

class CraftServiceException : public std::logic_error
{
public:
	// Generic constructor
	CraftServiceException() : std::logic_error("General CraftService exception") { };
	// Constructor with specific message problem
	CraftServiceException(std::string message) : std::logic_error(message) { };
};


#endif // CRAFT_SERVICE_EXCEPTION_HPP