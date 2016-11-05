/******************************************************************************* 
SerialCommand - An Arduino library to _tokenize and parse commands received over
a serial port. 
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

Version 20131021A.   

Version History:
May 11 2011 - Initial version
May 13 2011 -	Prevent overwriting bounds of SerialCommandCallback[] array in addCommand()
			defaultHandler() for non-matching commands
Mar 2012 - Some const char * changes to make compiler happier about deprecated warnings.  
           Arduino 1.0 compatibility (Arduino.h header) 
Oct 2013 - SerialCommand object can be created using a SoftwareSerial object, for SoftwareSerial
           support.  Requires #include <SoftwareSerial.h> in your sketch even if you don't use 
           a SoftwareSerial port in the project.  sigh.   See Example Sketch for usage. 
Oct 2013 - Conditional compilation for the SoftwareSerial support, in case you really, really
           hate it and want it removed.  

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
#ifndef SerialCommand_h
#define SerialCommand_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Stream.h>
#include <string.h>

#define SERIALCOMMANDBUFFER 16
#define MAXSERIALCOMMANDS	16
#define MAXDELIMETER 2

#define SERIALCOMMANDDEBUG 1
#undef SERIALCOMMANDDEBUG      // Comment this out to run the library in debug mode (verbose messages)

class SerialCommand
{
	public:
		SerialCommand(const Stream* = &Serial);      // Constructor, use default Serial0 Hardware Serial object

		void clear_buffer();   // Sets the command _buffer to all '\0' (nulls)
		char* next();         // returns pointer to next _token found in command _buffer (for getting arguments to commands)
		void readSerial();    // Main entry point.  
		void addCommand(const char *, void(*)(SerialCommand*));   // Add commands to processing dictionary
		void addDefaultHandler(void(*function)(SerialCommand*));    // A handler to call when no valid command received.
	
	private:
		char _inChar;          // A character read from the serial stream 
		char _buffer[SERIALCOMMANDBUFFER];   // _buffer of stored characters while waiting for _terminator character
		int  _buffPos;                        // Current position in the _buffer
		char _delim[MAXDELIMETER];           // null-_terminated list of character to be used as _delimeters for _tokenizing (default " ")
		char _term;                          // Character that signals end of command (default '\r')
		char* _token;                        // Returned _token from the command _buffer as returned by strtok_r
		char* _last;                         // State variable used by strtok_r during processing

		typedef struct _callback {
			char command[SERIALCOMMANDBUFFER];
			void(*function)(SerialCommand*);
		} SerialCommandCallback;            // Data structure to hold Command/Handler function key-value pairs

		int _numCommand;
		SerialCommandCallback CommandList[MAXSERIALCOMMANDS];   // Actual definition for command/handler array
		void(*defaultHandler)(SerialCommand*);           // Pointer to the default handler function 
		Stream *_serial;
};

#endif //SerialCommand_h