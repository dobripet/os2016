#include "rtl.h"
#include "c_rd.h"

#include <string>
#include <iostream>

/*Removes an empty directory*/
size_t __stdcall rd(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parse arg
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("RD: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//switches
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Removes (deletes) an empty directory.\n\n  RD [drive:]path\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "RD: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("RD: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	/*Calling with zero params*/
	if (args.size() == 0) {
		char * msg = "The syntax of the RD command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	else {
		for (int i = 0; i < args.size(); i++) {
			char * path = (char*)args[i].c_str();
			if (!Remove_Dir(path)) {
				Print_Last_Error(STDERR, "RD: An error occured while deleting folder: " + std::string(path) + ".\n");
			}
		}
	}
	return (size_t)0;
}