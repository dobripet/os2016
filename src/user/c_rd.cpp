#include "rtl.h"
#include "c_rd.h"
#include <string>
#include <iostream>
/*Removes directory*/
size_t __stdcall rd(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR= (FDHandle)regs.R10;
	size_t total; 
	size_t size;
	std::cout << "DEBUG:md volano s poctem parametru: " << regs.Rcx << "\n";
	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:md param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
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