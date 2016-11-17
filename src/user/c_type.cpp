#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>
/*Multiple params connect to one string and prints to stdout*/
size_t __stdcall type(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size; 
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Displays the contents of a text file or files.\n\n  TYPE[drive:][path]filename\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		char * msg = "The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	else {
		std::cout << "DEBUG:type volano s poctem parametru: " << regs.Rcx << "\n";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			std::cout << "DEBUG:type param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
		}
		for (int i = 0; i < (int)regs.Rcx; i++) {
			char * path = ((char**)regs.Rdx)[i];
			FDHandle file;
			if (!Open_File(&file, path, F_MODE_READ)) {
				switch (Get_Last_Error()) {
					case ERR_IO_PATH_NOEXIST: {
						std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
						Write_File(STDERR, (char *)msg.c_str(), msg.length());
						break;
					}
				}
			}
			else {
				char buffer[1024];
				size_t size = 1024;
				size_t filled;
				std::string s = "";
				do {
					if (!Read_File(file, size, buffer, &filled)) {
						/*TODO nejakej error, mozna zavrnej handle*/
						switch (Get_Last_Error()) {
						case ERR_IO_PATH_NOEXIST: {
							std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
							Write_File(STDERR, (char *)msg.c_str(), msg.length());
							break;
						}
						}
					}
					s += ((std::string)buffer).substr(0,filled);
				} while (size == filled);
				std::string text = "\n" + (std::string)path + "\n\n" + s + "\n";
				size_t written;
				/*Handle not all has been written*/
				if (!Write_File(STDOUT, (char *)text.c_str(), text.length(), &written) || written != text.length()) {
					std::string msg = "Failed to write out text.\nError occurred while processing: " + (std::string)path + "\n";
					Write_File(STDERR, (char *)msg.c_str(), msg.length());
					break;
				}
			}
		}
	}
	return (size_t)0;
}