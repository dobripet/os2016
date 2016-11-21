
#include <iostream>
#include <mutex>
#include "rtl.h"
#include "c_wc.h"

#include <sstream>
#include <atomic>

/*
size_t __stdcall wc(const CONTEXT &regs) {
	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	FDHandle CURRENT_DIR = (FDHandle)regs.R11;
	char *switches = (char *)regs.R12;
	char * myName = (char *)regs.Rax;
	//int myPid = (int)regs.Rbx;
	std::thread::id myPid = std::this_thread::get_id();
	int argc = (int)regs.Rcx;
	char ** argv = (char**)regs.Rdx;


	//for each (open_file(cesta z parametru))

	int charCnt = 0, wordCnt = 0, lineCnt = 1;


	boolean hasEOF = false;
	while (!hasEOF) {

		char * buf = (char*)malloc(101 * sizeof(char));
		size_t filled;
		Read_File(STDIN, 100, buf, &filled); //TODO ze souboru podle parametru

		charCnt += filled;

		for (int i = 0; i < filled; i++) {
			switch (buf[i]) {
			case EOF: {
				charCnt--;
				hasEOF = !hasEOF;
				break;
			}
			case '\n': {
				lineCnt++;
				break;
			}

			//TODO
			}
		}
	}


	//TODO pocty na stdout
}
*/


std::mutex aaa;


const int BUFFER_SIZE = 1024;
size_t __stdcall wc(const CONTEXT &regs) {
/*
	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	FDHandle CURRENT_DIR = (FDHandle)regs.R11;
	char *switches = (char *)regs.R12;
	char * myName = (char *)regs.Rax;
	//int myPid = (int)regs.Rbx;
	std::thread::id myPid = std::this_thread::get_id();
	int argc = (int)regs.Rcx;
	char ** argv = (char**)regs.Rdx;

	while (true) {	

		char * buf = (char*)malloc(21 * sizeof(char));
		size_t filled;
		Read_File(STDIN, 20, buf, &filled);	
		if (buf[filled - 1] == EOF || buf[filled] == EOF) {
			buf[filled] = '\0';
			aaa.lock();
			std::cout << "pid=" << myPid << ", read=" << filled << ", text=" << buf << ", EOF" << std::endl;
			aaa.unlock();
			Write_File(STDOUT, buf, filled);
			break;
		}
		else {
			buf[filled] = '\0';
			aaa.lock();
			std::cout << "pid=" << myPid << ", read=" << filled << ", text=" << buf << std::endl;
			aaa.unlock();
			Write_File(STDOUT, buf, filled);
		}
	}
	
	return (size_t)0;
}*/
	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written;
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Count lines, words and bytes of an input.\n\n  WC[[drive:][path]]filename\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
		/*Handle not all has been written*/
		if (check_write("WC", STDERR, success, written, size) != S_OK) {
			return (size_t)1;
		}
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		/*Read from stdin until EOF*/
		size_t filled;
		//std::string text = "";
		size_t words = 0;
		size_t lines = 0;
		size_t bytes = 0;
		char buffer[BUFFER_SIZE];
		bool prev_blank = false;
		while (true) {
			Read_File(STDIN, BUFFER_SIZE, buffer, &filled);
			for (size_t i = 0; i < filled; i++) {
				if (isblank(buffer[i])) {
					//spaces and tabs
					if (!prev_blank) {
						words++;
						prev_blank = true;
					}
				}
				else if (buffer[i] == '\r') {
					//does nothing
				}
				else if (buffer[i] == '\n') {
					//newline
					lines++; 
					if (!prev_blank) {
						words++;
					}
					prev_blank = true;
				}
				else {
					//regular char
					prev_blank = false;
				}
				bytes++;
			}
			if (buffer[filled] == EOF) { //goodbye
				break;
			}

		}
		std::string text = "";
		std::stringstream ss;
		ss.width(10);
		ss << "Lines";
		text +=ss.str();
		ss.str("");
		ss.width(10);
		ss << "Words";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Bytes" << "\n";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << lines;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << words;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << bytes << "\n";;
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		/*Handle not all has been written*/
		if (check_write("WC", STDERR, success, written, size) != S_OK) {
			return (size_t)1;
		}
	}
	else {
		std::string text = "";
		std::stringstream ss;
		ss.width(10);
		ss << "Lines";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Words";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Bytes";
		text += ss.str();
		ss.str("");
		ss.width(20);
		ss << "Name" << "\n";
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		/*Handle not all has been written*/
		if (check_write("WC", STDERR, success, written, size) != S_OK) {
			return (size_t)1;
		}
		size_t sum_words = 0;
		size_t sum_lines = 0;
		size_t sum_bytes = 0;
		/*read from files*/
		for (int i = 0; i < (int)regs.Rcx; i++) {
			char * path = ((char**)regs.Rdx)[i];
			FDHandle file;
			if (!Open_File(&file, path, F_MODE_READ)) {
				//	std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
				//	Write_File(STDERR, (char *)msg.c_str(), msg.length());
				switch (Get_Last_Error()) {
				case ERR_IO_PATH_NOEXIST: {
					std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
					Write_File(STDERR, (char *)msg.c_str(), msg.length());
					break;
				}
				}
				continue;//continue to next file
			}
			char buffer[BUFFER_SIZE];
			bool prev_blank = false;
			size_t filled;
			size_t words = 0;
			size_t lines = 0;
			size_t bytes = 0;
			text = "";
			/*read whole file*/
			do {
				if (!Read_File(file, BUFFER_SIZE, buffer, &filled)) {
					/*TODO nejakej error, mozna zavrnej handle*/
					switch (Get_Last_Error()) {
					case ERR_IO_PATH_NOEXIST: {
						std::string msg = "The system cannot find the file specified.\nError occurred while processing: " + (std::string)path + "\n";
						Write_File(STDERR, (char *)msg.c_str(), msg.length());
						break;
					}
					}
					break;
				}
				for (size_t i = 0; i < filled; i++) {
					if (isblank(buffer[i])) {
						//spaces and tabs
						if (!prev_blank) {
							words++;
							prev_blank = true;
						}
					}
					else if (buffer[i] == '\r') {
						//does nothing
					}
					else if (buffer[i] == '\n') {
						//newline
						lines++;
						if (!prev_blank) {
							words++;
						}
						prev_blank = true;
					}
					else {
						//regular char
						prev_blank = false;
					}
					bytes++;
				}
			} while (BUFFER_SIZE == filled);

			ss.width(10);
			ss << lines;
			sum_lines += lines;
			text += ss.str();
			ss.str("");
			ss.width(10);
			ss << words;
			sum_words += words;
			text += ss.str();
			ss.str("");
			ss.width(10);
			ss << bytes;
			sum_bytes += bytes;
			text += ss.str();
			ss.str("");
			ss.width(20);
			ss << path << "\n";
			text += ss.str();
			ss.str("");
			size = text.length();
			success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
			/*Handle not all has been written*/
			if (check_write("WC", STDERR, success, written, size) != S_OK) {
				return (size_t)1;
			}
		}
		text = "";
		ss.width(10);
		ss << sum_lines;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << sum_words;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << sum_bytes;
		text += ss.str();
		ss.str("");
		ss.width(20);
		ss << "Total" << "\n";
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		/*Handle not all has been written*/
		if (check_write("WC", STDERR, success, written, size) != S_OK) {
			return (size_t)1;
		}

	}
	return (size_t)0;
}