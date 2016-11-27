#include "rtl.h"
#include "c_md.h"

//vytvori adresar/e, cesta musi existovat
size_t __stdcall md(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parsovani argumentu
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("MD: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//zpracovani prepinacu
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Creates a directory.\n\n  MD[drive:]path\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "MD: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("MD: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	//zpracovani bez parametru
	if (args.size() == 0) {
		char * msg = "MD: The syntax of the command is incorrect. Missing parameters.\n\0";
		Write_File(STDERR, msg, strlen(msg)); 
		return (size_t)1;
	}
	else {
		//postupne zpracovani kazdeho adresare
		for (int i = 0; i < args.size(); i++) {
			char * path = (char*)args[i].c_str();
			if (!Make_Dir(path)) {
				Print_Last_Error(STDERR, "MD: An error occured while creating directory: " + std::string(path) + ".\n");
			}
		}
	}		
	return (size_t)0;
}