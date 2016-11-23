#include "rtl.h"
#include "c_ps.h"
#include <string>
#include <iostream>
#include <sstream>
/*Prints running processes*/
size_t __stdcall ps(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Displays list of running processes.\n\n  PS\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		std::vector<process_info*> all_info;
		Get_Processes(&all_info);
		std::string text = "";
		std::stringstream ss;
		ss.width(10);
		ss << "Name";
		text += ss.str();
		ss.str("");
		ss.width(5);
		ss << "PID";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "ThreadID";
		text += ss.str();
		ss.str("");
		ss.width(15);
		ss << "Working DIR";
		text += ss.str() + "\n";
		for (size_t i = 0; i < all_info.size(); i++) {
			std::string line = "";
			std::stringstream ss;
			ss.width(10);
			ss << all_info[i]->name;
			line += ss.str();
			ss.str("");
			ss.width(5);
			ss << all_info[i]->pid;
			line += ss.str();
			ss.str("");
			ss.width(10);
			ss << all_info[i]->threadID;
			line += ss.str();
			ss.str("");
			ss.width(15);
			ss << all_info[i]->workingDir;
			line += "  " + ss.str();
			text += line + "\n";
			delete all_info[i];
		}
//		delete &all_info;
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
	}
	else {
		/*wrong number of params*/
		char * msg = "The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "PS: writing to stdout failed.");
			return (size_t)1;
		}
	}
	return (size_t)0;
}
