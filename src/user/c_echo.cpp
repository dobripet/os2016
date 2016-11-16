﻿#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>
/*Multiple params connect to one string and prints to stdout*/
size_t __stdcall echo(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t total; 
	size_t size;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Displays messages\n\n  ECHO[message]\n\nType ECHO without parameters to display the current echo setting.\n\0";
		size = strlen(msg);
		bool success = Write_File(STDOUT, msg, size, &total);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		char * msg = "ECHO is always on.\0";
		size = strlen(msg);
		bool success = Write_File(STDOUT, (char *)msg, size, &total);
	}
	else {
		size_t written;
		std::cout << "DEBUG:echo volano s poctem parametru: " << regs.Rcx << "\n";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			std::cout << "DEBUG:echo param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
		}
		std::string text = "";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			text += (std::string)((char**)regs.Rdx)[i] + " ";
		}
		text.replace(text.length() - 1, 1, "\0");
		size = text.length();
		std::cout << "DEBUG:echo text je " << text << "\n";
		bool success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		total = written;
		while (success && total != size) {
			std::string rest = text.substr(total);
			success = Write_File(STDOUT, (char *)rest.c_str(), rest.length(), &written);
			total += written;
		}
	}
	/*Handle not all has been written*/
	if (total != size) {
		std::cout << "DEBUG:echo error\n";
		char * msg = "ECHO: error - not all data written(possibly closed file handle)\0";
		Write_File(STDERR, msg, strlen(msg), &total);
		return (size_t)1;
	}
	else {
		std::cout << "DEBUG:echo successful\n";
	}
	return (size_t)0;
}