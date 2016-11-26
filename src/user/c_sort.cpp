#include "rtl.h"
#include "c_sort.h"

#include <string>
#include <iostream>
#include <algorithm>

/*Sorts lines of input and prints them*/

std::string sortText(std::string text);

size_t __stdcall sort(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parse arg
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("SORT: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//switches
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Sorts input lines.\n\n  SORT[[drive:][path]]filename\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "SORT: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("SORT: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	size_t written;
	size_t size;
	bool success;

	/*No params*/
	if (args.size() == 0) {
		/*Read from stdin until EOF*/
		size_t filled;
		std::string text = "";
		char buffer[1025];
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
	else if (args.size() == 1) {
		/*read from file*/
		FDHandle file;
		char * path = (char*)args[0].c_str();
		if (!Open_File(&file, path, F_MODE_READ)) {
			Print_Last_Error(STDERR, "SORT: An error occurred while opening: " + std::string(path));
			return (size_t)1;
		}
		char buffer[1025];
		size_t bsize = 1024;
		size_t filled;
		std::string text = "";
		/*read whole file*/
		do {
			if (!Read_File(file, bsize, buffer, &filled)) {
				Print_Last_Error(STDERR, "SORT: An error occurred while reading from: " + std::string(path));
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
		char * msg = "SORT: The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "SORT: writing to stdout failed.");
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