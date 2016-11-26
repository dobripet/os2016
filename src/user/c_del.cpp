#include "rtl.h"
#include "c_del.h"

#include <string>
#include <iostream>

/*Removes a file (not a folder)*/
size_t __stdcall del(const CONTEXT &regs) {

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
			char * msg = "Deletes one or more files.\n\n  DEL names\n\nnames Specifies a list of one or more files.\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "DEL: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("DEL: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	/*Calling with zero params*/
	if (args.size() == 0) {
		char * msg = "DEL: The syntax of the command is incorrect. Missing params.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	else {
		for (size_t i = 0; i < args.size(); i++) {
			char * path = (char*)args[i].c_str();
			if (!Remove_File(path)) {
				std::string msg = "DEL: An error occurred while deleting: " + (std::string)path + ".\n";
				Print_Last_Error(STDERR, msg);
			}
		}
	}
	return (size_t)0;
}