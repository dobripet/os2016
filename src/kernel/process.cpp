#include "process.h"
#include "kernel.h"
#include <iostream>

std::mutex process_mtx; //mutex for process table
struct pcb* process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes

int createProcess(int(*function)(int, char*[]), int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err);

void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess: 
		regs.Rax = (decltype(regs.Rax))createProcess((int(*)(int, char*[])) regs.Rbx, regs.Rcx, (char**) regs.Rdx, (char*)regs.R8, (char*)regs.R9, (char*)  regs.R10, (THandle)regs.R11, (THandle)regs.R12, (THandle)regs.R13);
		Set_Error(regs.Rax == -1, regs);
		break;
	}
}
/*Vytvoreni noveho procesu */
int  createProcess(int(*function)(int, char*[]), int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err) {
	int iy = (int) out;
	std::cout << "tvorim proces v jadre" << name << " " << iy << "\n";
	function(argc, argv);
	pcb *new_process = new pcb; 
	new_process->name = name;
	new_process->current_dir = current_dir;
	new_process->files.push_back(in);
	new_process->files.push_back(out);
	new_process->files.push_back(err);
	process_mtx.lock(); 
	int pid = -1;
	for (int i = 0; i < PROCESS_TABLE_SIZE; i++) {
		if (process_table[i] == nullptr){			
			new_process->pid = i;
			process_table[i] = new_process;
			pid = i;
			break;
		}
	}
	process_mtx.unlock();
	if (pid == -1) {
		SetLastError(CREATE_PROCESS_ERROR);
	}
	return pid;
}
