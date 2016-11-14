
#include <iostream>
#include <mutex>
#include "rtl.h"
#include "c_wc.h"

#include <atomic>

std::mutex aaa;


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

	while (true) {	

		char * buf = (char*)malloc(21 * sizeof(char));
		size_t filled;
		Read_File(STDIN, 20, buf, &filled);	
		if (buf[filled - 1] == EOF) {
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
}