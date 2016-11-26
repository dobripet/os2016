#include <iostream>
#include <sstream>
#include <random>

#include "rtl.h"
#include "c_rgen.h"

//prints random float numbers to stdout
size_t __stdcall rgen(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	char * arg = (char*)regs.Rcx;

	//parse arg
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("RGEN: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//switches
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Continuously prints random float numbers in range <0,1) to STDOUT.\n\n  RGEN\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "RGEN: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("RGEN: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}

	if (args.size() > 0) {
		std::string msg("Error: RGEN does not take parameters.\n");
		Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
		return (size_t)1;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> rnd(0.0f, 1.0f);

	size_t avail = 0;

	while (true) {
		//generuje to moc, trochu to pozdrzime //pozdeji asi smazat
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		std::string s = std::to_string(rnd(gen)) + " ";
		bool ok = Write_File(STDOUT, (char*)s.c_str(), s.length());
		if (!ok) {
			if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
				Print_Last_Error(STDERR, "RGEN: writing to stdout failed.");
				return (size_t)1;
			}
			break;
		}
		Peek_File(STDIN, &avail);
		if (avail > 0) {
			size_t filled = 0;
			char * buf = new char[avail + 1];
			Read_File(STDIN, avail, buf, &filled);
			if (buf[filled] == EOF) {
				delete[] buf;
				break;
			}
			delete[] buf;
		}
	}
	return (size_t)0;
}