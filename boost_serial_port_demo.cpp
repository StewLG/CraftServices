// boost_serial_port_demo.cpp
// Demo using boost::asio::serial_port to send simple commands to a device
// Written by Brian Paden, 2008-11-6
// This code is released under the WTFPL
//-----------------------------------------------------------------------------
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, December 2004
//
// Copyright (C) 2004 Sam Hocevar
//  14 rue de Plaisance, 75014 Paris, France
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
//  0. You just DO WHAT THE FUCK YOU WANT TO.
//
//    This program is free software. It comes without any warranty, to
//    the extent permitted by applicable law. You can redistribute it
//    and/or modify it under the terms of the Do What The Fuck You Want
//    To Public License, Version 2, as published by Sam Hocevar. See
//    http://sam.zoy.org/wtfpl/COPYING for more details. */
//-----------------------------------------------------------------------------

#include <boost/asio.hpp> // include boost
using namespace::boost::asio;  // save tons of typing
#include <iostream>
using std::cin;

// These are the values our port needs to connect
#ifdef _WIN32
// windows uses com ports, this depends on what com port your cable is plugged in to.
const char *PORT = "COM3";
#else
// *nix com ports
const char *PORT = "dev/ttyS3";
#endif
// Note: all the following except BAUD are the exact same as the default values

// what baud rate do we communicate at
serial_port_base::baud_rate BAUD(19200);
// how big is each "packet" of data (default is 8 bits)
serial_port_base::character_size CSIZE(8);
// what flow control is used (default is none)
serial_port_base::flow_control FLOW(serial_port_base::flow_control::none);
// what parity is used (default is none)
serial_port_base::parity PARITY(serial_port_base::parity::none);
// how many stop bits are used (default is one)
serial_port_base::stop_bits STOP(serial_port_base::stop_bits::one);

int main()
{
	// create the I/O service that talks to the serial device
	io_service io;
	// create the serial device, note it takes the io service and the port name
	serial_port port(io, PORT);

	// go through and set all the options as we need them
	// all of them are listed, but the default values work for most cases
	port.set_option(BAUD);
	port.set_option(CSIZE);
	port.set_option(FLOW);
	port.set_option(PARITY);
	port.set_option(STOP);

	// buffer to store commands
	// this device reads 8 bits, meaning an unsigned char, as instructions
	// varies with the device, check the manual first
	unsigned char command[1] = { 0 };

	// read in user value to be sent to device
	int input;
	cin >> input;

	// Simple loop, since the only good values are [0,255]
	//  break when a negative number is entered.
	// The cast will convert too big numbers into range.
	while (input >= 0)
	{
		// convert our read in number into the target data type
		command[0] = static_cast<unsigned char>(input);

		// this is the command that sends the actual bits over the wire
		// note it takes a stream and a asio::buffer
		// the stream is our serial_port
		// the buffer is constructed using our command buffer and
		//  the number of instructions to send
		write(port, buffer(command, 1));

		// read in the next input value
		cin >> input;
	}

	// all done sending commands
	return 0;
}