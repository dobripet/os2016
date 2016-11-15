#include "rtl.h"
#include "c_echo.h"
#include <string>
#include <iostream>
/*Param 0 is text, param 1 is size of text, param 2 target filehandle*/
size_t __stdcall echo(const CONTEXT &regs) {

	FDHandle STDOUT = (FDHandle)regs.R9;

	std::cout << "DEBUG:echo volano s poctem parametru: " << regs.Rcx << "\n";
	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:echo param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
	}
	std::string text = "";
	for (int i = 0; i < (int)regs.Rcx; i++) {
		text += (std::string)((char**)regs.Rdx)[i] + " ";
	}
	text.replace(text.length() - 1, 1, "\0");
	size_t size = text.length();
	size_t written;
	std::cout << "DEBUG:echo text je " << text << "\n";
	bool success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
	size_t total = written;
	while (success && total != size) {
		std::string rest = text.substr(total);
		success = Write_File(STDOUT, (char *)rest.c_str(), rest.length(), &written);
		total += written;
	}
	/*Handle not all has been written*/
	if (total != size) {
		std::cout << "DEBUG:echo error\n";
		return (size_t)1;
	}
	else {
		std::cout << "DEBUG:echo successful\n";
	}
	return (size_t)0;
}