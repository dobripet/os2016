#include "rtl.h"
#include "c_md.h"
#include <iostream>

size_t __stdcall md(const CONTEXT &regs) {

	std::cout << "DEBUG:md volano s poctem parametru: " << regs.Rcx << "\n";
	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:md param " << i << ": "<< ((char**)regs.Rdx)[i] << "\n";
	}
	/*Calling with wrong number of params*/
	if ((int)regs.Rcx != 1) {
		return (size_t)1;
	}
	char * path = ((char**)regs.Rdx)[0];
	bool mkdir = Make_Dir(path);
	if (mkdir == 0) {
		/*handle error*/
		std::cout << "DEBUG:md error\n";
		return (size_t)1;
	}
	else {
		std::cout << "DEBUG:md successful\n";
	}
	return (size_t)0;
}