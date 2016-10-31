
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
	int s = params.size();
	std::string q = "\nJsem proces \"" + thisProcess.name + "\" s pid = " + std::to_string(thisProcess.pid) + " a mam " + std::to_string(s) + " parametru.\n";
	
	//std::string q = *((std::string*) regs.Rbx);
	std::cout << q;
	return (size_t)0;
}