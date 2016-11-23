#include <iostream>
#include <sstream>
#include <random>
#include "rtl.h"
#include "c_rgen.h"

size_t __stdcall rgen(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

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
				Print_Last_Error(STDERR, "Rgen: writing to stdout failed.");
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