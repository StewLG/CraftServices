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


#ifndef COMMAND_LINE_ARGUMENTS_EXCEPTION_HPP
#define COMMAND_LINE_ARGUMENTS_EXCEPTION_HPP

#include <stdexcept>

class CommandLineArgumentsException : public std::logic_error
{
	public:
		// Generic constructor
		CommandLineArgumentsException() : std::logic_error("Error processing command line arguments") { };
		// Constructor with specific message about lack of implmentation
		CommandLineArgumentsException(std::string message) : std::logic_error(message) { };
}; 


#endif // COMMAND_LINE_ARGUMENTS_EXCEPTION_HPP