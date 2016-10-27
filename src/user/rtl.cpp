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


bool Create_Process(TEntryPoint * func, command_params * par)
{
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scCreateProcess);
	regs.Rbx = (decltype(regs.Rbx))func;
	regs.Rcx = (decltype(regs.Rcx))par;
	return Do_SysCall(regs);
}

/*
bool Create_Process(void(*func)(PCB * pcb, std::vector<std::string> argv), run_params * params)
{
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scCreateProcess);
	regs.Rbx = (decltype(regs.Rbx))func;
	regs.Rcx = (decltype(regs.Rcx))params;
	return Do_SysCall(regs);
}*/



/*
bool Create_Process(TEntryPoint * func, int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err) {
//bool Create_Process(int(*func)(int argc, char *argv[]), int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err){
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scCreateProcess);
	regs.Rbx = (decltype(regs.Rbx)) func;
	regs.Rcx = (decltype(regs.Rcx)) argc;
	regs.Rdx = (decltype(regs.Rdx)) argv;

	regs.R8 = (decltype(regs.R8)) name;
	regs.R9 = (decltype(regs.R9)) current_dir;
	regs.R10 = (decltype(regs.R9)) root_dir;
	regs.R11 = (decltype(regs.R10)) in; 
	regs.R12 = (decltype(regs.R11)) out;
	regs.R13 = (decltype(regs.R12)) err;
	return Do_SysCall(regs);
}
*/