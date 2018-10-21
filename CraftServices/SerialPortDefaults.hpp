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

#ifndef SERIALPORTDEFAULTS_HPP
#define SERIALPORTDEFAULTS_HPP

#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>

const int DEFAULT_BAUD_RATE = 19200;

// On the bench this appears to be a decent default, but I haven't proved that out in the field. 
// It's now configurable though, so users hopefully will offer feedback.
const int DEFAULT_REFRESH_INTERVAL_IN_MILLISECONDS = 100;

// This is currently just a guess about what will be useful
const int DEFAULT_STALE_INTERVAL_IN_MILLISECONDS = 4000;

#endif // SERIALPORTDEFAULTS_HPP



// For reference:

/*
// Note: all the following except BAUD are the exact same as the default values

// what baud rate do we communicate at
boost::asio::serial_port_base::baud_rate DEFAULT_BAUD_RATE(19200);
// how big is each "packet" of data (default is 8 bits)
boost::asio::serial_port_base::character_size DEFAULT_CSIZE(8);
// what flow control is used (default is none)
boost::asio::serial_port_base::flow_control DEFAULT_FLOW(boost::asio::serial_port_base::flow_control::none);
// what parity is used (default is none)
boost::asio::serial_port_base::parity DEFAULT_PARITY(boost::asio::serial_port_base::parity::none);
// how many stop bits are used (default is one)
boost::asio::serial_port_base::stop_bits DEFAULT_STOP(boost::asio::serial_port_base::stop_bits::one);
*/

//// An example of port startup for reference
//void samplePortStartup()
//{
//	// create the I/O service that talks to the serial device
//	boost::asio::io_service io;
//	// create the serial device, note it takes the io service and the port name
//	boost::asio::serial_port port(io, PORT);
//
//	// go through and set all the options as we need them
//	// all of them are listed, but the default values work for most cases
//	port.set_option(BAUD);
//	port.set_option(CSIZE);
//	port.set_option(FLOW);
//	port.set_option(PARITY);
//	port.set_option(STOP);
//}
