﻿#include "rtl.h"
#include "c_sort.h"
#include <string>
#include <iostream>
#include <algorithm>
/*Sorts lines of input and print them*/

std::string sortText(std::string text);

size_t __stdcall sort(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Sorts input lines.\n\n  SORT[[drive:][path]]filename\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		/*Read from stdin until EOF*/
		size_t filled;
		std::string text = "";
		char buffer[1024];
		while (true){
			Read_File(STDIN, 1024, buffer, &filled);
			text += ((std::string)buffer).substr(0, filled);
			if (buffer[filled] == EOF) { //goodbye
				break;
			}

		}
		std::string sorted = sortText(text);
		size = sorted.length();
		success = Write_File(STDOUT, (char *)sorted.c_str(), size, &written);		
	}
	else if((int)regs.Rcx == 1){
		/*read from file*/
		FDHandle file;
		char * path = ((char**)regs.Rdx)[0];
		if (!Open_File(&file, path, F_MODE_READ)) {
		//	std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
		//	Write_File(STDERR, (char *)msg.c_str(), msg.length());
			Print_Last_Error(STDERR, "An error occurred while opening: " + std::string(path));
			/*switch (Get_Last_Error()) {
			case ERR_IO_PATH_NOEXIST: {
				std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
				Write_File(STDERR, (char *)msg.c_str(), msg.length());
				break;
			}
			}*/
			return (size_t)1;
		}
		char buffer[1024];
		size_t bsize = 1024;
		size_t filled;
		std::string text = "";
		/*read whole file*/
		do {
			if (!Read_File(file, bsize, buffer, &filled)) {
				Print_Last_Error(STDERR, "An error occurred while reading from: " + std::string(path));
				/*switch (Get_Last_Error()) {
				case ERR_IO_PATH_NOEXIST: {
					std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
					Write_File(STDERR, (char *)msg.c_str(), msg.length());
					break;
				}
				}*/
				continue;
			}
			text += ((std::string)buffer).substr(0, filled);
		} while (bsize == filled);
		std::string sorted = sortText(text);
		size = sorted.length();
		success = Write_File(STDOUT, (char *)sorted.c_str(), size, &written);		
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
			Print_Last_Error(STDERR, "Rgen: writing to stdout failed.");
			return (size_t)1;
		}
	}
	return (size_t)0;
}


std::string sortText(std::string text) {
	size_t position = 0;
	size_t from = 0;
	std::vector<std::string> lines;
	/*split by lines*/
	int offset = 2;
	while (true) {
		offset = 2;
		position = text.find("\r\n", from);
		if (position == std::string::npos) {
			offset = 1;
			position = text.find("\n", from);
			if (position == std::string::npos) {
				lines.push_back(text.substr(from));
				break;
			}
		}
		position += offset;
		lines.push_back(text.substr(from, position-from));
		from = position;
	}
	std::sort(lines.begin(), lines.end());
	std::string sorted = "";
	for (size_t i = 0; i < lines.size(); i++) {
		sorted += lines[i];
	}
	return sorted;
}