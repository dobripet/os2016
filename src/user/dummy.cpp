#include "rtl.h"
#include "dummy.h"
#include <chrono>
#include <iostream>

size_t __stdcall dummy(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	
	size_t filled = 0;
	char * buf = new char[1001];

	while (true) {
		//porad dokola cteme a kdyz narazime na EOF, tak ho preposleme dale..  a konec.
		bool ok = Read_File(STDIN, 1000, buf, &filled);
		if (!ok && Get_Last_Error() == ERR_IO_PIPE_READCLOSED) {
			break;
		}
		if (buf[filled] == EOF) {
			buf[0] = EOF;
			Write_File(STDOUT, buf, 1);
			break;
		}
		//nebudeme to delat zbytecne moc casto..
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	delete[] buf;
	return (size_t)0;
}