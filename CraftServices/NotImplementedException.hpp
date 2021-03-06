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

#ifndef NOT_IMPLEMENTED_EXCEPTION_HPP
#define NOT_IMPLEMENTED_EXCEPTION_HPP

#include <stdexcept>

class NotImplementedException : public std::logic_error
{
	public:
		// Generic constructor
		NotImplementedException() : std::logic_error("Function not yet implemented") { };
		// Constructor with specific message about lack of implementation
		NotImplementedException(std::string message) : std::logic_error(message) { };
}; 


#endif // NOT_IMPLEMENTED_EXCEPTION_HPP