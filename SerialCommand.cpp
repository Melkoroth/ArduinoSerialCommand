/******************************************************************************* 
SerialCommand - An Arduino library to _tokenize and parse commands received over
a serial port. 
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

See SerialCommand.h for version history. 

This library is free software; you can redistribute it and/or
modify it under the _terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************************/

#include "SerialCommand.h"

// Constructor makes sure some things are set. 
SerialCommand::SerialCommand(const Stream *ser) : _serial(ser), _term('\r'), _numCommand(0)
{
	strncpy(_delim, " ", MAXDELIMETER);  // strtok_r needs a null-_terminated string
	clear_buffer(); 
}

//
// Initialize the command _buffer being processed to all null characters
//
void SerialCommand::clear_buffer()
{
	for (int i = 0; i < SERIALCOMMANDBUFFER; i++) 
	{
		_buffer[i] = '\0';
	}
	_buffPos = 0; 
}

// Retrieve the next _token ("word" or "argument") from the Command _buffer.  
// returns a NULL if no more _tokens exist.   
char* SerialCommand::next() 
{
	char *next_token;
	next_token = strtok_r(NULL, _delim, &_last); 
	return next_token; 
}

// This checks the Serial stream for characters, and assembles them into a _buffer.  
// When the _terminator character (default '\r') is seen, it starts parsing the 
// _buffer for a prefix command, and calls handlers setup by addCommand() member
void SerialCommand::readSerial() 
{
	while (_serial->available() > 0)
	{
		int i; 
		boolean matched; 
		_inChar = _serial->read();
		#ifdef SERIALCOMMANDDEBUG
		_serial->print(_inChar);   // Echo back to serial stream
		#endif
		if (_inChar ==_term) {     // Check for the _terminator (default '\r') meaning end of command
			#ifdef SERIALCOMMANDDEBUG
			_serial->println();
			_serial->print("Received: "); 
			_serial->println(_buffer);
		    #endif
			_buffPos = 0;           // Reset to start of _buffer
			_token = strtok_r(_buffer, _delim, &_last);   // Search for command at start of _buffer
			if (_token == NULL) return; 
			matched = false; 
			for (i = 0; i <_numCommand; i++) {
				#ifdef SERIALCOMMANDDEBUG
				_serial->print("Comparing ["); 
				_serial->print(_token); 
				_serial->print("] to [");
				_serial->print(CommandList[i].command);
				_serial->println("]");
				#endif
				// Compare the found command against the list of known commands for a match
				if (strncmp(_token,CommandList[i].command,SERIALCOMMANDBUFFER) == 0) 
				{
					#ifdef SERIALCOMMANDDEBUG
					_serial->print("Matched Command: "); 
					_serial->println(_token);
					#endif
					// Execute the stored handler function for the command
					(*CommandList[i].function)(this); 
					clear_buffer(); 
					matched = true; 
					break; 
				}
			}
			if (matched == false) {
				(*defaultHandler)(this); 
				clear_buffer(); 
			}
		}
		if (isprint(_inChar))   // Only printable characters into the _buffer
		{
			_buffer[_buffPos++] = _inChar;   // Put character into _buffer
			_buffer[_buffPos] = '\0';  // Null _terminate
			if (_buffPos > SERIALCOMMANDBUFFER-1) 
				_buffPos = 0; // wrap _buffer around if full  
		}
	}
}

// Adds a "command" and a handler function to the list of available commands.  
// This is used for matching a found _token in the _buffer, and gives the pointer
// to the handler function to deal with it. 
void SerialCommand::addCommand(const char *command, void(*function)(SerialCommand*))
{
	if (_numCommand < MAXSERIALCOMMANDS) {
		#ifdef SERIALCOMMANDDEBUG
		_serial->print(_numCommand); 
		_serial->print("-"); 
		_serial->print("Adding command for "); 
		_serial->println(command); 
		#endif
		
		strncpy(CommandList[_numCommand].command,command,SERIALCOMMANDBUFFER); 
		CommandList[_numCommand].function = function; 
		_numCommand++; 
	} else {
		// In this case, you tried to push more commands into the _buffer than it is compiled to hold.  
		// Not much we can do since there is no real visible error assertion, we just ignore adding
		// the command
		#ifdef SERIALCOMMANDDEBUG
		_serial->println("Too many handlers - recompile changing MAXSERIALCOMMANDS"); 
		#endif 
	}
}

// This sets up a handler to be called in the event that the receveived command string
// isn't in the list of things with handlers.
void SerialCommand::addDefaultHandler(void(*function)(SerialCommand*))
{
	defaultHandler = function;
}