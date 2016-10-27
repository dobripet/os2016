#include "process.h"
#include "kernel.h"
#include <iostream>
#include <thread>


std::mutex process_mtx; //mutex for process table
PCB* process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes


void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess:
		regs.Rax = (decltype(regs.Rax))createProcess((TEntryPoint *) regs.Rbx, (command_params *)regs.Rcx);
		//regs.Rax = (decltype(regs.Rax))createProcess((runFunctionPointer) regs.Rbx, (run_params*)regs.Rcx);
		//regs.Rax = (decltype(regs.Rax))createProcess((int(*)(int, char*[])) regs.Rbx, regs.Rcx, (char**) regs.Rdx, (char*)regs.R8, (char*)regs.R9, (char*)  regs.R10, (THandle)regs.R11, (THandle)regs.R12, (THandle)regs.R13);
		//regs.Rax = (decltype(regs.Rax))createProcess((TEntryPoint *)regs.Rbx, regs.Rcx, (char**)regs.Rdx, (char*)regs.R8, (char*)regs.R9, (char*)regs.R10, (THandle)regs.R11, (THandle)regs.R12, (THandle)regs.R13);
		Set_Error(regs.Rax == -1, regs);
		break;
	}
}

int createProcess(TEntryPoint * func, command_params * par)
{
	PCB newProc;
	newProc.IO_decriptors.push_back(par->STDIN);
	newProc.IO_decriptors.push_back(par->STDOUT);
	newProc.IO_decriptors.push_back(par->STDERR);
	newProc.name = par->name;
	//newProc.current_dir = ...
	//newProc.root_dir = ...

	process_mtx.lock();
	int pid = -1;
	for (int i = 0; i < PROCESS_TABLE_SIZE; i++) {
		if (process_table[i] == nullptr) {
			newProc.pid = i;
			process_table[i] = &newProc;
			pid = i;
			break;
		} //pri ruseni procesu se pak zas musi dat table[pid] = nullptr
	}
	process_mtx.unlock();
	if (pid == -1) {
		SetLastError(CREATE_PROCESS_ERROR);
	}
	else {
		newProc.pid = pid;
		CONTEXT regs;
		regs.Rbx = (decltype(regs.Rbx)) &newProc;
		regs.Rcx = (decltype(regs.Rcx)) &(par->params);
		(*func)(regs); //TODO ve vlakne tohlencto
	}

	return pid;

}

/*
int createProcess(void(*func)(PCB * pcb, std::vector<std::string> argv), run_params *params) {

	PCB newProc;
	newProc.files.push_back(params->STDIN);
	newProc.files.push_back(params->STDOUT);
	newProc.files.push_back(params->STDERR);
	newProc.name = params->name;
	newProc.current_dir = params->current_dir;

	//std::thread proc(func, newProc, params->ARGV);

	process_mtx.lock();
	int pid = -1;
	for (int i = 0; i < PROCESS_TABLE_SIZE; i++) {
		if (process_table[i] == nullptr) {
			newProc.pid = i;
			process_table[i] = &newProc;
			pid = i;
			break;
		}
	}
	process_mtx.unlock();
	if (pid == -1) {
		SetLastError(CREATE_PROCESS_ERROR);
	}
	return pid;


}*/

/*
int createProcess() {
	CONTEXT regs;
	TEntryPoint e = (TEntryPoint)GetProcAddress(User_Programs, "wc");
	GetThreadContext(GetCurrentThread(), &regs);
	e(regs);
	return 0;
}*/

/*Vytvoreni noveho procesu */
/*
int  createProcess(TEntryPoint * func, int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err) {
	//int  createProcess(int(*function)(int, char*[]), int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err) {
	int iy = (int)out;
	std::cout << "tvorim proces v jadre" << name << " " << iy << "\n";
	CONTEXT regs;

	GetThreadContext(GetCurrentThread(), &regs);
	(*func)(regs);
	PCB *new_process = new PCB;
	new_process->name = name;
	new_process->current_dir = nullptr; //CURRENT DIR
	new_process->files.push_back(in);
	new_process->files.push_back(out);
	new_process->files.push_back(err);
	process_mtx.lock();
	int pid = -1;
	for (int i = 0; i < PROCESS_TABLE_SIZE; i++) {
		if (process_table[i] == nullptr) {
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

	//createProcess();

	return pid;
}
*/