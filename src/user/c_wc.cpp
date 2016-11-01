
#include <iostream>
#include <mutex>
#include "c_wc.h"
#include "..\kernel\process.h"

/*
int wc(int argc, char * argv[]) {
	std::cout << "bezi program wc s parametry: " << argc << "\n";
	return 0;
}
*/
int s = 500;

std::mutex aaa;


size_t __stdcall wc(const CONTEXT &regs) {

	THandle STDIN = (THandle)regs.R8;
	THandle STDOUT = (THandle)regs.R9;
	THandle STDERR = (THandle)regs.R10;
	char * myName = (char *)regs.Rax;
	int myPid = (int)regs.Rbx;
	int argc = (int)regs.Rcx;
	char ** argv = (char**)regs.Rdx;
	char *switches = (char *)regs.R11;

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
	std::cout << std::endl;
	std::cout.flush();
	aaa.unlock();
	
	return (size_t)0;
}