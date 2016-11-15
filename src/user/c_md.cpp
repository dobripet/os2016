#include "rtl.h"
#include "c_md.h"
#include <iostream>

size_t __stdcall md(const CONTEXT &regs) {

	std::cout << "DEBUG:md volano s poctem parametru: " << regs.Rcx << "\n";
	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:md param " << i << ": "<< ((char**)regs.Rdx)[i] << "\n";
	}
	char * path = ((char**)regs.Rdx)[0];
	bool mkdir = Make_Dir(path);
	if (mkdir == 0) {
		/*handle error*/
		std::cout << "DEBUG:md error\n";
	}
	else {
		std::cout << "DEBUG:md successful\n";
	}
	return (size_t)0;
}