#include "rtl.h"
#include "c_rd.h"
#include <string>
#include <iostream>
/*Removes file*/
size_t __stdcall del(const CONTEXT &regs) {
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	std::cout << "DEBUG:del volano s poctem parametru: " << regs.Rcx << "\n";


	for (int i = 0; i < (int)regs.Rcx; i++) {
		std::cout << "DEBUG:del param " << i << ": " << ((char**)regs.Rdx)[i] << "\n";
	}
	/*Flag handling*/
	if (!strcmp((char *)regs.R12, "h\0")) {
		char * msg = "Deletes one or more files.\n\n  DEL names\n\nnames Specifies a list of one or more files.\n\0";
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
			bool rmdir = Remove_File(path);
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