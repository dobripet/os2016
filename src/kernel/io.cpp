#include "io.h"
#include "kernel.h"

void HandleIO(CONTEXT &regs) {

	//V ostre verzi pochopitelne do switche dejte volani funkci a ne primo vykonny kod

	switch (Get_AL((__int16) regs.Rax)) {
		case scCreateFile: {
				regs.Rax = (decltype(regs.Rax)) CreateFileA((char*)regs.Rdx, GENERIC_READ | GENERIC_WRITE , (DWORD) regs.Rcx, 0, OPEN_EXISTING, 0, 0);
							//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
				Set_Error(regs.Rax == 0, regs);				
			}
			break;	//scCreateFile


		case scWriteFile: {
				DWORD written;
				const bool failed = !WriteFile((HANDLE)regs.Rdx, (void*)regs.Rdi, (DWORD)regs.Rcx, &written, NULL);
				Set_Error(failed, regs);
				if (!failed) regs.Rax = written;
			}
			break; //scWriteFile


		case scCloseFile: {
			Set_Error(!CloseHandle((HANDLE)regs.Rdx), regs);			
			}
			break;	//CloseFile
	}
}