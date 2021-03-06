#include "rtl.h"

#include <atomic>
extern "C" __declspec(dllimport) void __stdcall SysCall(CONTEXT &context);

std::atomic<size_t> LastError;

size_t Get_Last_Error() {
	return LastError;
}

void Print_Last_Error(FDHandle out) {
	Print_Last_Error(out, "");
}
void Print_Last_Error(FDHandle out, std::string prefix) {
	std::string err = prefix;
	if (!((prefix.length() > 0 && prefix[prefix.length() - 1] == '\n') ||
		(prefix.length() > 1 && prefix[prefix.length() - 2] == '\n'))) {
		err += "\n";
	}
	switch (Get_Last_Error()) {
	case ERR_PROCESS_CREATE: {
		err += "It was not possible to create new process.\n";
		break;
	}
	case ERR_PROCESS_NOTFOUND: {
		err += "Process entry point was not found.\n";
		break;
	}
	case ERR_IO_FILE_CREATE: {
		err += "It was not possible to create new file.\n";
		break;
	}
	case ERR_IO_PATH_NOEXIST: {
		err += "Path was not found.\n";
		break;
	}
	case ERR_IO_FILE_ISNOTFOLDER: {
		err += "Specified file is NOT a folder.\n";
		break;
	}
	case ERR_IO_FILE_ISNOTFILE: {
		err += "Specified file is a folder.\n";
		break;
	}
	case ERR_IO_FILE_ISOPENED: {
		err += "Specified file is used by another process.\n";
		break;
	}
	case ERR_IO_FILE_NOTEMPTY: {
		err += "Specified folder is not empty.\n";
		break;	
	}
	case ERR_IO_WRITE_STD: {
		err += "Windows error: it was not possible to write to console STDOUT.\n";
		break;
	}
	case ERR_IO_READ_STD: {
		err += "Windows error: it was not possible to read from console STDIN.\n";
		break;
	}
	case ERR_IO_PIPE_READCLOSED: {
		err += "Can't write to pipe. It is closed for reading.\n";
		break;
	}
	case ERR_IO_FOLDER_EXISTS: {
		err += "File of given name already exists.\n";
		break;
	}
	default: {
		err += "Unspecified error occured.\n";
	}
	} //end switch
	Write_File(out, (char*)err.c_str(), err.length());
}



CONTEXT Prepare_SysCall_Context(__int8 major, __int8 minor) {
	CONTEXT regs;
	regs.Rax = Compose_AX(major, minor);
	return regs;
}

//return true pokud syscall probehl bez chyb
bool Do_SysCall(CONTEXT &regs) {
	SysCall(regs);
	const bool failed = test_cf(regs.EFlags);
	if (failed) {
		LastError = regs.Rax;
	}
	else {
		LastError = ERR_NOERROR;
	}
	return !failed;
}

bool Duplicate_File(FDHandle old_handle, FDHandle * new_handle) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scDuplicateHandle);
	regs.Rcx = (decltype(regs.Rcx))old_handle;
	bool success = Do_SysCall(regs);
	*new_handle = (FDHandle)regs.Rbx;
	return success;
}
bool Open_File(FDHandle * handle, const char * fname, int mode) {
	return Open_File(handle, fname, mode, false); 
}
bool Open_File(FDHandle * handle, const char * fname, int mode, bool rewrite) {

	CONTEXT regs = Prepare_SysCall_Context(scIO, scOpenFile);
	regs.Rdx = (decltype(regs.Rdx))fname;
	regs.Rcx = (decltype(regs.Rcx))mode;
	regs.Rbx = (decltype(regs.Rbx))&rewrite;
	bool success = Do_SysCall(regs);
	*handle = (FDHandle)regs.Rbx;
	return success;
}
bool Open_Pipe(FDHandle * writeHandle, FDHandle * readHandle) {

	CONTEXT regs = Prepare_SysCall_Context(scIO, scCreatePipe);
	bool success = Do_SysCall(regs);
	*writeHandle = (FDHandle)regs.Rbx;
	*readHandle = (FDHandle)regs.Rcx;
	return success;
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
	bool success = Do_SysCall(regs);
	*available = (size_t)regs.Rbx;
	return success;
}

bool Read_File(FDHandle handle, size_t len, char * buf, size_t * filled) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scReadFile);
	regs.Rdx = (decltype(regs.Rdx))handle;
	regs.Rcx = (decltype(regs.Rcx))len;
	regs.Rbx = (decltype(regs.Rbx))buf;
	bool success = Do_SysCall(regs);
	*filled = (size_t)regs.Rbx;
	return success;
}
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size, size_t *written) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scWriteFile);
	regs.Rbx = (decltype(regs.Rbx))file_handle;
	regs.Rcx = (decltype(regs.Rcx))buffer;
	regs.Rdx = (decltype(regs.Rdx))buffer_size;
	const bool success = Do_SysCall(regs);
	*written = (size_t)regs.Rbx;
	return success;
}
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scWriteFile);
	regs.Rbx = (decltype(regs.Rbx))file_handle;
	regs.Rcx = (decltype(regs.Rcx))buffer;
	regs.Rdx = (decltype(regs.Rdx))buffer_size;
	return Do_SysCall(regs);
}
bool Create_Process(command_params * par, int * pid)
{
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scCreateProcess);
	regs.Rcx = (decltype(regs.Rcx))par;
	bool success = Do_SysCall(regs);
	*pid = (int)regs.Rdx;
	return success;
}
bool Join_and_Delete_Process(int pid) {
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scJoinProcess);
	regs.Rbx = (decltype(regs.Rbx))pid;
	return Do_SysCall(regs);
}
bool Make_Dir(char *path) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scMakeDir);
	regs.Rbx = (decltype(regs.Rbx))path;
	return Do_SysCall(regs);
}
bool Change_Dir(char *path) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scChangeDir);
	regs.Rbx = (decltype(regs.Rbx))path;
	return Do_SysCall(regs);
}
bool Remove_Dir(char *path) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scRemoveDir);
	regs.Rbx = (decltype(regs.Rbx))path;
	return Do_SysCall(regs);
}
bool Remove_File(char *path) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scRemoveFile);
	regs.Rbx = (decltype(regs.Rbx))path;
	return Do_SysCall(regs);
}
bool Get_Processes(std::vector<process_info*> *all_info) {
	CONTEXT regs = Prepare_SysCall_Context(scProcess, scGetProcesses);
	regs.Rbx = (decltype(regs.Rbx))all_info;
	return Do_SysCall(regs);
}
bool Get_Dir_Nodes(std::vector<node_info*> *all_info, char *path) {
	CONTEXT regs = Prepare_SysCall_Context(scIO, scGetDirNodes);
	regs.Rbx = (decltype(regs.Rbx))all_info;
	regs.Rcx = (decltype(regs.Rcx))path;
	return Do_SysCall(regs);
}