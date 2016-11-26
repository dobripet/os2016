#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>

/*prints whole input argument and a newline*/
size_t __stdcall echo(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	size_t written;
	size_t size;
	bool success;

	//parse arg
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("ECHO: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//switches
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Displays messages\n\n  ECHO[message]\n\nType ECHO without parameters to display the current echo setting.\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "ECHO: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("ECHO: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}
	/*No params*/
	size = strlen(arg);
	if (size == 0) {
		char * msg = "ECHO is always on.\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, (char *)msg, size, &written);
	}
	else {
		std::string m = std::string(arg) + "\n";
		size = m.length();
		success = Write_File(STDOUT, (char*)m.c_str(), size, &written);
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "ECHO: writing to stdout failed.");
			return (size_t)1;
		}
	}
	return (size_t)0;
}