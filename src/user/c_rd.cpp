#include "rtl.h"
#include "c_rd.h"
#include <string>
#include <iostream>
/*Removes directory*/
size_t __stdcall rd(const CONTEXT &regs) {
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	std::cout << "DEBUG:rd volano s poctem parametru: " << regs.Rcx << "\n";


	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:rd param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
	}
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Removes (deletes) a directory.\n\n  RD [drive:]path\n\0";
			/*RD[/ S][/ Q][drive:]path

			/ S      Removes all directories and files in the specified directory
			in addition to the directory itself.Used to remove a directory
			tree.

			/ Q      Quiet mode, do not ask if ok to remove a directory tree with / S*/
		Write_File(STDOUT, (char *)msg, strlen(msg));
	}
	/*Calling with zero params*/
	else if ((int)regs.Rcx == 0) {
		char * msg = "The syntax of the command is incorrect.\n\0";
		Write_File(STDERR, msg, strlen(msg));
		return (size_t)1;
	}
	else {
		for (int i = 0; i < (int)regs.Rcx; i++) {
			char * path = ((char**)regs.Rdx)[i];
			bool rmdir = Remove_Dir(path);
			/*handle error*/
			if (rmdir == false) {
				switch (Get_Last_Error()) {
					case ERR_IO_FILE_NOTEMPTY: {
						char *msg = "The directory is not empty.\n\0";
						Write_File(STDERR, msg, strlen(msg));
						break;
					}
					case (size_t)ERR_IO_FILE_ISOPENED: {
						char *msg = "The directory is opened in another process.\n\0";
						Write_File(STDERR, msg, strlen(msg));
						break;
					}
					case (size_t)ERR_IO_FILE_ISFILE: {
						char *msg = "The directory name is invalid.\n\0";
						Write_File(STDERR, msg, strlen(msg));
						break;
					}
					case (size_t)ERR_IO_PATH_NOEXIST: {
						char *msg = "The system cannot find the file specified.\n\0";
						Write_File(STDERR, msg, strlen(msg));
						break;
					}

				}
			}
		}
	}
	return (size_t)0;
}