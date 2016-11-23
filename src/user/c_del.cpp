#include "rtl.h"
#include "c_del.h"
#include <string>
#include <iostream>
/*Removes file*/
size_t __stdcall del(const CONTEXT &regs) {
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

//	std::cout << "DEBUG:del volano s poctem parametru: " << regs.Rcx << "\n";


	/*for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:del param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
	}*/
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Deletes one or more files.\n\n  DEL names\n\nnames Specifies a list of one or more files.\n";
			/*

DEL [/P] [/F] [/S] [/Q] [/A[[:]attributes]] names
ERASE [/P] [/F] [/S] [/Q] [/A[[:]attributes]] names

  names         Specifies a list of one or more files or directories.
                Wildcards may be used to delete multiple files. If a
                directory is specified, all files within the directory
                will be deleted.

  /P            Prompts for confirmation before deleting each file.
  /F            Force deleting of read-only files.
  /S            Delete specified files from all subdirectories.
  /Q            Quiet mode, do not ask if ok to delete on global wildcard
  /A            Selects files to delete based on attributes
  attributes    R  Read-only files            S  System files
                H  Hidden files               A  Files ready for archiving
                I  Not content indexed Files  L  Reparse Points
                -  Prefix meaning not

If Command Extensions are enabled DEL and ERASE change as follows:

The display semantics of the /S switch are reversed in that it shows
you only the files that are deleted, not the ones it could not find.
*/
		if (!Write_File(STDOUT, (char *)msg, strlen(msg))) {
			if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
				std::string msg = "An error occurred while writing to STDOUT\n";
				Print_Last_Error(STDERR, msg);
				return (size_t)1;
			}
			return (size_t)0;
		}
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
			bool del = Remove_File(path);
			/*handle error*/
			if (del == false) {
				std::string msg = "An error occurred while deleting: " + (std::string)path + "\n";
				Print_Last_Error(STDERR, msg);
			}
		}
	}
	return (size_t)0;
}