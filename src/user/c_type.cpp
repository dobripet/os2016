#include "rtl.h"
#include "c_type.h"
#include <string>
#include <iostream>
/*Multiple params connect to one string and prints to stdout*/
size_t __stdcall type(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	bool help = false, quietMode = false; 
	char * switches = (char *)regs.R12;
	for (int i = 0; i < strlen(switches); i++) {
		switch (switches[i]) {
		case 'h': help = true; break;
		case 'q': quietMode = true; break;
		}
	}


	size_t written; 
	size_t size; 
	bool success;
	/*Flag handling*/
	if (help) {
		char * msg = "Displays the contents of a text file or files.\n\n  TYPE [/q] [drive:][path]filename\n\n  /q   Quiet mode, do not print filename\n\0";
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
				std::cout << "DEBUG:type err: " << Get_Last_Error() << "\n";
				switch (Get_Last_Error()) {
					case ERR_IO_PATH_NOEXIST: {
						std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
						Write_File(STDERR, (char *)msg.c_str(), msg.length());
						break;
					}
				}
				continue;
			}
			else {
				if (!quietMode) {
					size_t written;
					std::string header = "\n" + (std::string)path + "\n\n";
					if (!Write_File(STDOUT, (char*)header.c_str(), header.length(), &written) || written != header.length()) {
						/*Handle not all has been written*/
						std::string msg = "Failed to write out text.\nError occurred while processing: " + (std::string)path + "\n";
						Write_File(STDERR, (char *)msg.c_str(), msg.length());
						continue;
					}
				}
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
						continue;
					}
					if (!Write_File(STDOUT, buffer, filled, &written) || written != filled) {
						/*Handle not all has been written*/
						std::string msg = "Failed to write out text.\nError occurred while processing: " + (std::string)path + "\n";
						Write_File(STDERR, (char *)msg.c_str(), msg.length());
						break;
					}
				} while (size == filled);	
				if (!Write_File(STDOUT,"\n", strlen("\n"), &written) || written != strlen("\n")) {
					/*Handle not all has been written*/
					std::string msg = "Failed to write out text.\nError occurred while processing: " + (std::string)path + "\n";
					Write_File(STDERR, (char *)msg.c_str(), msg.length());
				}
			}
		}
	}
	return (size_t)0;
}
