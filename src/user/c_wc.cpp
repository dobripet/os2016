
#include <iostream>
#include "c_wc.h"

/*
int wc(int argc, char * argv[]) {
	std::cout << "bezi program wc s parametry: " << argc << "\n";
	return 0;
}
*/

size_t __stdcall wc(const CONTEXT &regs) {

	PCB thisProcess = *((PCB *)regs.Rbx);
	std::vector<std::string> params = *((std::vector<std::string> *)regs.Rcx);
	std::cout << "Jsem proces \"" << thisProcess.name << "\" s pid = " << thisProcess.pid << " a mam " << params.size() << " parametru."<< std::endl;
	return 0;
}