
#include <iostream>
#include <mutex>
#include "rtl.h"
#include "c_wc.h"
//#include "..\kernel\process.h"

#include <atomic>

//int s = 500;
std::atomic<int> q = 0;
std::atomic<bool> first = true;
//std::mutex aaa;


size_t __stdcall wc(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * myName = (char *)regs.Rax;
	int myPid = (int)regs.Rbx;
	int argc = (int)regs.Rcx;
	char ** argv = (char**)regs.Rdx;
	char *switches = (char *)regs.R11;

	/*
	q += 10;
	//if (q == 30)
		//std::this_thread::sleep_for(std::chrono::milliseconds(500)); else 
	std::this_thread::sleep_for(std::chrono::milliseconds(q));
	*/


	/*
	aaa.lock();
	if (s == 200) {
		s = 600;
	}
	std::cout << "Jsem proces \"" << myName << "\" s pid " << myPid << " a budu spat " << (s - 100) << " ms" << std::endl;
	std::cout.flush();
	s -= 100;
	aaa.unlock();

	//simulujem praci
	std::this_thread::sleep_for(std::chrono::milliseconds(s));

	aaa.lock();
	std::cout << "Jsem proces s pid " << myPid << " a probudil jsem. Mam " << argc << " params a " << std::strlen(switches) << " switches" << std::endl << "Jsou to tyto: ";
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << ", ";
	}
	std::cout << std::endl;
	for (int i = 0; i < std::strlen(switches); i++) {
		std::cout << switches[i] << ", ";
	}
	if (std::strlen(switches) > 0) {
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout.flush();
	aaa.unlock();
	*/

	if (first) {
		first = !first;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		while (true) {
			char * buf = (char*)malloc(20 * sizeof(char));
			int filled;
			buf[19] = '\0';
			Read_File(STDIN, 20, buf, &filled);
			std::cout << "pid=" << myPid << ", read=" <<  filled << ", text=" << buf << std::endl;
			Write_File(STDOUT, buf, 20);
		}
	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		while (true) {
			char * buf = (char*)malloc(20 * sizeof(char));
			int filled;
			buf[19] = '\0';
			Read_File(STDIN, 20, buf, &filled);

			std::cout << "pid=" << myPid << ", read=" << filled << ", text=" << buf << std::endl;
			Write_File(STDOUT, buf, 20);
		}
	}


	/*
	while (true) {
		char * buf = (char*)malloc(20 * sizeof(char));
		int filled;
		buf[19] = '\0';
		Read_File(STDIN, 20, buf, &filled);

		//char * buf2 = (char*)malloc((filled + 1) * sizeof(char));
		//strncpy_s(buf2, filled+1, buf, filled);
		//free(buf);
		//buf2[filled] = '\0';
		std::cout << filled << ", HERE " << buf << std::endl;
		Write_File(STDOUT, buf, 20);
	}*/
	

	/*
	if (!first) {
		//std::cout << "ZDE" << std::endl;
		char * buf = "tohleJeBlbostLidi\n\0";
		Write_File(STDOUT, buf, 20);
		first = false;

	}


	else {
		//std::cout << "LICH" << std::endl;
		char * buf =(char*)malloc(20*sizeof(char));
		Read_File(STDIN, 20, buf);
		std::cout << "HERE " << buf << std::endl;
		Write_File(STDOUT, buf, 20);
		//free(buf);
	}*/

	//char* hello = "WC Test zapisu: Hello world!\0";
	//size_t written;
	

	
	return (size_t)0;
}