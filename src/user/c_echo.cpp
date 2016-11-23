#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>
/*Multiple params connect to one string and prints to stdout*/
size_t __stdcall echo(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Displays messages\n\n  ECHO[message]\n\nType ECHO without parameters to display the current echo setting.\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		char * msg = "ECHO is always on.\0";
		size = strlen(msg);
		success = Write_File(STDOUT, (char *)msg, size, &written);
	}
	else {
		/*std::cout << "DEBUG:echo volano s poctem parametru: " << regs.Rcx << "\n";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			std::cout << "DEBUG:echo param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
		}*/
		std::string text = "";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			text += (std::string)((char**)regs.Rdx)[i] + " ";
		}
		text.replace(text.length() - 1, 1, "\0");
		text += "\n";
		size = text.length();
	//	std::cout << "DEBUG:echo text je " << text << "\n";
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);	
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "Rgen: writing to stdout failed.");
			return (size_t)1;
		}
		//std::cout << "DEBUG:echo error\n";
		//char * msg = "ECHO: error - not all data written(possibly closed file handle)\0";
		//Write_File(STDERR, msg, strlen(msg));
		//return (size_t)1;
	}
	/*else {
		std::cout << "DEBUG:echo successful\n";
	}*/
	return (size_t)0;
}