#include "rtl.h"
#include "c_type.h"

//vypise obsah souboru, lze spojit vice souboru do jednoho vypisu
size_t __stdcall type(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;
	bool quietMode = false;
	size_t written;

	//parsovani argumentu
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("TYPE: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//zpracovani prepinacu
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Displays the contents of a text file or files.\n\n  TYPE [/q] [drive:][path]filename\n\n  /q   Quiet mode - do not print filenames\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "TYPE: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		//tichy mod, nevypisuje jmena souboru
		else if(tolower(switches[s]) == 'q') {
			quietMode = true;
		}
		else {
			std::string msg("TYPE: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	//zpracovani bez parametru
	if (args.size() == 0) {
		char * msg = "The syntax of the TYPE command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	else {
		//postupne zpracovani kazdeho souboru
		for (int i = 0; i < args.size(); i++) {
			char * path = (char*)args[i].c_str();
			FDHandle file;
			if (!Open_File(&file, path, F_MODE_READ)) {
				std::string msg = "TYPE: An error occurred while opening file: " + (std::string)path + "\n";
				Print_Last_Error(STDERR, msg);
				continue;
			}
			else {
				//pridani hlavicky pokud neni tichy
				if (!quietMode) {
					size_t written;
					std::string header = "\n" + (std::string)path + "\n\n";
					if (!Write_File(STDOUT, (char*)header.c_str(), header.length(), &written) || written != header.length()) {
						//osetreni chyby vypisu
						if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
							std::string msg = "TYPE: An error occurred while writing to: " + (std::string)path + "\n";
							Print_Last_Error(STDERR, msg);
							continue;
						}
					}
				}
				char buffer[1024];
				size_t size = 1024;
				size_t filled;
				std::string s = "";
				//cteni a rovnou vypis dokud se neprecte cely soubor
				do {
					if (!Read_File(file, size, buffer, &filled)) {
						std::string msg = "TYPE: An error occurred while reading from: " + (std::string)path + "\n";
						Print_Last_Error(STDERR, msg);
						break;
					}
					if (!Write_File(STDOUT, buffer, filled, &written) || written != filled) {
						//osetreni chyby vypisu
						if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
							std::string msg = "TYPE: An error occurred while writing to: " + (std::string)path + "\n";
							Print_Last_Error(STDERR, msg);
						}
						break;
					}
				} while (size == filled);	
				if (!Write_File(STDOUT,"\n", strlen("\n"), &written) || written != strlen("\n")) {
					//osetreni chyby vypisu
					if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
						std::string msg = "TYPE: An error occurred while writing to STDOUT\n";
						Print_Last_Error(STDERR, msg);
					}
				}
			}
		}
	}
	return (size_t)0;
}
