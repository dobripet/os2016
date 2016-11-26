#include "rtl.h"
#include "c_freq.h"
#include <string>
#include <iostream>
#include <algorithm>

/*Counts frequency of the input bytes and prints it*/

const int FREQ_SIZE = 256;
const int BUFFER_SIZE = 1024;

size_t __stdcall freq(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parse arg
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("DEL: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//switches
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Counts and prints frequency table of all input bytes.\n\n  FREQ\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "FREQ: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("FREQ: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}


	size_t written;
	size_t size;
	bool success;

	/*No params*/
	if (args.size() == 0) {
		/*Read from stdin until EOF*/
		std::string text = "";
		int freq[FREQ_SIZE] = { 0 };
		char buffer[BUFFER_SIZE+1];
		size_t filled;
		while (true) {
			Read_File(STDIN, BUFFER_SIZE, buffer, &filled);
			for (int i = 0; i < filled; i++) {
 				if (buffer[i] >= 0 || buffer[i] < FREQ_SIZE) {
					freq[buffer[i]]++;
				}
			}
			if (buffer[filled] == EOF) { //goodbye
				break;
			}

		}
		for (int i = 0; i < FREQ_SIZE; i++) {
			if (freq[i] > 0) {
				char buf[128];
				size_t n = sprintf_s(buf, "0x%hhx : %d\n", i, freq[i]);
				text += ((std::string)buf).substr(0, n);
			}
		}		
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);	
	}
	else {
		/*wrong number of params*/
		char * msg = "FREQ: The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "FREQ: writing to stdout failed.");
			return (size_t)1;
		}
	}
	return (size_t)0;
}
