#include "rtl.h"

#include <atomic>

#include <iostream>
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

bool Open_File(FDHandle old_handle, FDHandle * new_handle) {

	CONTEXT regs = Prepare_SysCall_Context(scIO, scDuplicateHandle);
	regs.Rcx = (decltype(regs.Rcx))old_handle;
	bool fail = Do_SysCall(regs);
	*new_handle = (FDHandle)regs.Rbx;
	return fail;
}


bool Open_File(FDHandle * handle, const char * fname, int mode) {

	CONTEXT regs = Prepare_SysCall_Context(scIO, scOpenFile);
	regs.Rdx = (decltype(regs.Rdx))fname;
	regs.Rcx = (decltype(regs.Rcx))mode;
	bool fail = Do_SysCall(regs);
	*handle =  (FDHandle)regs.Rbx;
	return fail;
}

bool Open_Pipe(FDHandle * writeHandle, FDHandle * readHandle) {

	CONTEXT regs = Prepare_SysCall_Context(scIO, scCreatePipe);
	bool fail = Do_SysCall(regs);
	*writeHandle = (FDHandle)regs.Rbx;
	*readHandle = (FDHandle)regs.Rcx;
	return fail;
}

bool Close_File(FDHandle file_handle) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scCloseFile);
	regs.Rdx = (decltype(regs.Rdx))file_handle;
	return Do_SysCall(regs);
}

bool Peek_File(FDHandle handle, size_t * available)
{
	CONTEXT regs = Prepare_SysCall_Context(scIO, scPeekFile);
	regs.Rdx = (decltype(regs.Rdx))handle;
	bool ret = Do_SysCall(regs);
	*available = (size_t)regs.Rbx;
	return ret;
}

bool Read_File(FDHandle handle, size_t len, char * buf, size_t * filled) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scReadFile);
	regs.Rdx = (decltype(regs.Rdx))handle;
	regs.Rcx = (decltype(regs.Rcx))len;
	regs.Rbx = (decltype(regs.Rbx))buf;
	bool ret = Do_SysCall(regs);
	*filled = (size_t)regs.Rax;
	return ret;
}


/*
THandle Create_File(const char* file_name, size_t flags) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scCreateFile);
	regs.Rdx = (decltype(regs.Rdx)) file_name;
	regs.Rcx = flags;
	Do_SysCall(regs);
	return (THandle) regs.Rax;
}*/

bool Write_File( FDHandle file_handle, char *buffer, size_t buffer_size, size_t *written) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scWriteFile);
/*	write_params *par = new write_params();
	par->buffer = buffer;
	par->HANDLE = file_handle;
	par->size = buffer_size;
	regs.Rcx = (decltype(regs.Rdx)) par;
	*/
	regs.Rbx = (decltype(regs.Rbx))file_handle;
	regs.Rcx = (decltype(regs.Rcx))buffer;
	regs.Rdx = (decltype(regs.Rdx))buffer_size;

	const bool result = Do_SysCall(regs);
	*written = regs.Rax;
	return result;
}
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scWriteFile);
	regs.Rbx = (decltype(regs.Rbx))file_handle;
	regs.Rcx = (decltype(regs.Rcx))buffer;
	regs.Rdx = (decltype(regs.Rdx))buffer_size;

	const bool result = Do_SysCall(regs);
	return result;
}


bool Create_Process(/*TEntryPoint * func,*/ command_params * par)
{
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scCreateProcess);
	//regs.Rbx = (decltype(regs.Rbx))func;
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