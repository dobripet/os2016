#include "rtl.h"
#include "c_ps.h"
#include <sstream>

//vypise bezici procesy
size_t __stdcall ps(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parsovani argumentu
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("PS: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//zpracovani prepinacu
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Displays list of running processes.\n\n  PS\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "PS: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("PS: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	size_t written; 
	size_t size;
	bool success;

	//zpracovani bez parametru
	if (args.size() == 0) {
		//hlavicka
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
		//tabulka
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
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
	}
	else {
		//prilis mnoho parametru
		char * msg = "PS: The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	//osetreni chyby vypisu
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "PS: writing to stdout failed.");
			return (size_t)1;
		}
	}
	return (size_t)0;
}
