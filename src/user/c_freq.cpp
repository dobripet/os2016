#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>
#include <algorithm>
/*Count frequency of bytes and print it*/

const int FREQ_SIZE = 256;
const int BUFFER_SIZE = 1024;

size_t __stdcall freq(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	size_t written; 
	size_t size;
	bool success;
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "*Count frequency of input bytes and print it.\n\n  FREQ\n\0";
		size = strlen(msg);
		success = Write_File(STDOUT, msg, size, &written);
	}
	/*No params*/
	else if ((int)regs.Rcx == 0) {
		/*Read from stdin until EOF*/
		std::string text = "";
		int freq[FREQ_SIZE] = { 0 };
		char buffer[BUFFER_SIZE];
		size_t filled;
		while (true) {
			Read_File(STDIN, BUFFER_SIZE, buffer, &filled);
			for (int i = 0; i < filled; i++) {
				if (buffer[i] >= 0 || buffer[i] < FREQ_SIZE) {
					freq[buffer[i]]++;
				}
			}
			if (buffer[filled] == EOF) { //goodbye
				break;
			}

		}
		for (int i = 0; i < FREQ_SIZE; i++) {
			if (freq[i] > 0) {
				char buf[128];
				size_t n = sprintf_s(buf, "0x%hhx : %d\n", i, freq[i]);
				text += ((std::string)buf).substr(0, n);
			}
		}		
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);	
	}
	else {
		/*wrong number of params*/
		char * msg = "The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	/*Handle not all has been written*/
	if (!success || written != size) {
		char * msg = "FREQ: error - not all data written(possibly closed file handle)\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	return (size_t)0;
}
