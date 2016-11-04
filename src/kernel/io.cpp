#include "io.h"
#include "kernel.h"
#include "process.h"


open_file * open_files_table[OPEN_FILES_TABLE_SIZE] = { nullptr };//process table with max 1024 processes

void initIO() {
	open_files_table[0] = new open_file();
	open_files_table[0]->count = 1;
	open_files_table[0]->flags = OF_FLAGS_OS_STDIN;
	open_files_table[0]->node = nullptr;
	open_files_table[0]->pipe = nullptr;
	open_files_table[0]->std = GetStdHandle(STD_INPUT_HANDLE);
	open_files_table[1] = new open_file();
	open_files_table[0]->count = 1;
	open_files_table[0]->flags = OF_FLAGS_OS_STDOUT;
	open_files_table[0]->node = nullptr;
	open_files_table[0]->pipe = nullptr;
	open_files_table[0]->std = GetStdHandle(STD_OUTPUT_HANDLE);
	open_files_table[2] = new open_file();
	open_files_table[0]->count = 1;
	open_files_table[0]->flags = OF_FLAGS_OS_STDERR;
	open_files_table[0]->node = nullptr;
	open_files_table[0]->pipe = nullptr;
	open_files_table[0]->std = GetStdHandle(STD_ERROR_HANDLE);
}
void freeIO() {
	delete open_files_table[0];
	delete open_files_table[1];
	delete open_files_table[2];
}

void HandleIO(CONTEXT &regs) {

	//V ostre verzi pochopitelne do switche dejte volani funkci a ne primo vykonny kod

	switch (Get_AL((__int16) regs.Rax)) {
		case scCreateFile: {
				regs.Rax = (decltype(regs.Rax)) CreateFileA((char*)regs.Rdx, GENERIC_READ | GENERIC_WRITE , (DWORD) regs.Rcx, 0, OPEN_EXISTING, 0, 0);
							//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
				Set_Error(regs.Rax == 0, regs);				
			}
			break;	//scCreateFile

		case scWriteFile:
			regs.Rax = (decltype(regs.Rax))write_file((write_params *)regs.Rcx);
			Set_Error(regs.Rax == 0, regs);
			break;
		/*
		case scWriteFile: {
				DWORD written;
				const bool failed = !WriteFile((HANDLE)regs.Rdx, (void*)regs.Rdi, (DWORD)regs.Rcx, &written, NULL);
				Set_Error(failed, regs);
				if (!failed) regs.Rax = written;
			}
			break; //scWriteFile
		*/

		case scCloseFile: {
			Set_Error(!CloseHandle((HANDLE)regs.Rdx), regs);			
			}
			break;	//CloseFile
	}


}


unsigned long write_file(write_params *par) {
	int handle = (int) process_table[par->pid]->IO_decriptors[(int) par->handle];
	open_file *file = open_files_table[handle];
	unsigned long written;
	bool success = true;
	switch (file->flags) {
	case OF_FLAGS_OS_STDIN://let windows handle stdio
		success = WriteFile(file->std, par->buffer, par->size, &written, nullptr);
		break;
	case OF_FLAGS_OS_STDOUT:
		success = WriteFile(file->std, par->buffer, par->size, &written, nullptr);
		break;
	case OF_FLAGS_OS_STDERR:
		success = WriteFile(file->std, par->buffer, par->size, &written, nullptr);
		break;
	}
	if (success) return written; 
	return 0;
}