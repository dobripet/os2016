#include "rtl.h"

#include <atomic>

extern "C" __declspec(dllimport) void __stdcall SysCall(CONTEXT &context);


std::atomic<size_t> LastError;

size_t Get_Last_Error() {
	return LastError;
}

CONTEXT Prepare_SysCall_Context(__int8 major, __int8 minor) {
	CONTEXT regs;
	regs.Rax = Compose_AX(major, minor);
	return regs;
}

bool Do_SysCall(CONTEXT &regs) {
	SysCall(regs);

	const bool failed = test_cf(regs.EFlags);
	if (failed) LastError = regs.Rax;
		else LastError = 0;

	return !failed;
}



THandle Create_File(const char* file_name, size_t flags) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scCreateFile);
	regs.Rdx = (decltype(regs.Rdx)) file_name;
	regs.Rcx = flags;
	Do_SysCall(regs);
	return (THandle) regs.Rax;
}

bool Write_File(const THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scWriteFile);
	regs.Rdx = (decltype(regs.Rdx)) file_handle;
	regs.Rdi = (decltype(regs.Rdi)) buffer;
	regs.Rcx = buffer_size;	

	const bool result = Do_SysCall(regs);
	written = regs.Rax;
	return result;
}

bool Close_File(const THandle file_handle) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scCloseFile);
	regs.Rdx = (decltype(regs.Rdx))file_handle;
	return Do_SysCall(regs);
}