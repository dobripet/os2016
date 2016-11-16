#include "rtl.h"
#include "c_md.h"
#include <iostream>

size_t __stdcall md(const CONTEXT &regs) {
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	std::cout << "DEBUG:md volano s poctem parametru: " << regs.Rcx << "\n";
	
	
	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:md param " << i << ": "<< ((char**)regs.Rdx)[i] << "\n";
	}
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Creates a directory.\n\n  MD[drive:]path\n\0";
			/*MD creates any intermediate directories in the path, if needed.
			For example, assume \a does not exist then :

		mkdir \a\b\c\d

			is the same as :

		mkdir \a
			chdir \a
			mkdir b
			chdir b
			mkdir c
			chdir c
			mkdir d

			which is what you would have to type if extensions were disabled.*/
			
		Write_File(STDOUT, (char *)msg, strlen(msg));
	}
	/*Calling with zero params*/
	else if ((int)regs.Rcx == 0) {
		char * msg = "The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg)); 
		return (size_t)1;
	}
	else {
		std::string text = "";
		for (int i = 0; i < (int)regs.Rcx; i++) {
			char * path = ((char**)regs.Rdx)[i];
			bool mkdir = Make_Dir(path);
			/*handle error*/
			if (mkdir == false) {
				std::string msg = "A subdirectory or file test already exists.\nError occurred while processing: " + (std::string)path + "\n";
				Write_File(STDERR, (char *)msg.c_str(), msg.length());
			}
		}
	}		
	return (size_t)0;
}