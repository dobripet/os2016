#include "rtl.h"
#include "c_dir.h"
#include <string>
#include <iostream>
#include <sstream>
/*Prints directory*/
HRESULT handle_dir(char *path, FDHandle STDOUT, FDHandle STDERR);

size_t __stdcall dir(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Displays list files and subdirectories in a directory.\n\n  DIR\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
		/*Handle not all has been written*/
		if (check_write("DIR", STDERR, success, written, size) != S_OK) {
			return (size_t)1;
		}
	}
	/*No params, current dir*/
	else if ((int)regs.Rcx == 0) {
		if (handle_dir(nullptr, STDOUT, STDERR) != S_OK) {
			return (size_t)1;
		}
	}
	else {
		/*multiple dirs*/
		for (int i = 0; i < (int)regs.Rcx; i++) {
			char * path = ((char**)regs.Rdx)[i];
			if (handle_dir(path, STDOUT, STDERR) != S_OK) {
				return (size_t)1;
			}
		}
	}
	
	return (size_t)0;
}


HRESULT handle_dir(char *path, FDHandle STDOUT, FDHandle STDERR) {
	bool success;
	size_t written;
	size_t size;
	size_t sum = 0;
	int dir_count = 0;
	size_t file_count = 0;
	std::vector<node_info*> all_info;
	Get_Dir_Nodes(&all_info, path);
	if (all_info.size() < 1) {
		/*Internal kernel error, no nodes returned*/
		char * msg = "Internal kernel error.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return S_FALSE;
	}
	std::stringstream ss;
	std::stringstream text;
	if (all_info[0]->type == TYPE_DIRECTORY) {
		text << "Directory of: " << all_info[0]->path << "\n\n";
		ss.width(30);
		ss << "Name";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "Size/Type";
		text <<  ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << ".";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "<DIR>";
		text << ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << "..";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "<DIR>";
		text << ss.str() << "\n";
	}
	else {
		text << "Directory of: " << all_info[0]->pathParent << "\n\n";
		ss.width(30);
		ss << "Name";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "Size/Type";
		text << ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << all_info[0]->name;
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << all_info[0]->size;
		sum += all_info[0]->size;
		file_count++;
		text << ss.str() << "\n";
	}
	delete all_info[0];
	for (size_t i = 1; i < all_info.size(); i++) {
		std::string line = "";
		std::stringstream ss;
		ss.width(30);
		ss << all_info[i]->name;
		line += ss.str();
		ss.str("");
		ss.width(20);
		if (all_info[i]->type == TYPE_DIRECTORY) {
			ss << "<DIR>";
			dir_count++;
		}
		else {
			ss << all_info[i]->size;
			sum += all_info[i]->size;
			file_count++;
		}
		line += ss.str();
		text << line << "\n";
		delete all_info[i];
	}
	text << "\t\t" << file_count << " File(s) \t" << sum << " bytes\n";
	text << "\t\t" << dir_count << " Dir(s) \n";
	size = text.str().length();
	success = Write_File(STDOUT, (char *)text.str().c_str(), size, &written);
	/*Handle not all has been written*/
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "DIR: writing to stdout failed.\n");
			return (size_t)1;
		}
	}
	return (size_t)0;
}