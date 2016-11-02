#include "process.h"
#include "kernel.h"
#include <iostream>
#include <thread>
#include <Windows.h>

std::mutex process_table_mtx; //mutex for process table
PCB * process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes


void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess:
		regs.Rax = (decltype(regs.Rax))createProcess((command_params *)regs.Rcx);
		Set_Error(regs.Rax == -1, regs);
		break;
	}
}

void runProcess(TEntryPoint func, int pid, int argc, char ** argv, char * switches) {

	CONTEXT regs;
	regs.R8 = (decltype(regs.R8))process_table[pid]->IO_decriptors[0]; //stdit
	regs.R9 = (decltype(regs.R9))process_table[pid]->IO_decriptors[1]; //stdout
	regs.R10 = (decltype(regs.R10))process_table[pid]->IO_decriptors[2]; //stder
	regs.R11 = (decltype(regs.R11))switches;
	regs.Rax = (decltype(regs.Rax))process_table[pid]->name;
	regs.Rbx = (decltype(regs.Rbx))pid;
	regs.Rcx = (decltype(regs.Rcx))argc;
	regs.Rdx = (decltype(regs.Rdx))argv;
		
	size_t ret = func(regs);

	//uklid:
	/*
		ZAPSAT EOF NA STDOUT
		ZAVRIT OTEVRENE SOUBORY
	*/
	std::lock_guard<std::mutex> lock(process_table_mtx);
	delete process_table[pid];
	process_table[pid] = nullptr;
}

int createProcess(command_params * par)
{
	int pid = -1;
	{
		std::lock_guard<std::mutex> lock(process_table_mtx);
		for (int i = 0; i < PROCESS_TABLE_SIZE; i++) {
			if (process_table[i] == nullptr) {
				process_table[i] = new PCB();
				pid = i;
				break;
			}
		}
	}

	if (pid == -1) {
		SetLastError(CREATE_PROCESS_ERROR);
		return pid;
	}

	process_table[pid]->pid = pid;
	process_table[pid]->IO_decriptors.push_back(par->STDIN);
	process_table[pid]->IO_decriptors.push_back(par->STDOUT);
	process_table[pid]->IO_decriptors.push_back(par->STDERR);
	process_table[pid]->name = par->name;
	//newProc.current_dir = ...
	//newProc.root_dir = ...
	
	TEntryPoint func = (TEntryPoint)GetProcAddress(User_Programs, par->name);
	if (!func) {
		std::lock_guard<std::mutex> lock(process_table_mtx);
		delete process_table[pid];
		process_table[pid] = nullptr;
		SetLastError(CREATE_PROCESS_ERROR);
		return pid;
	}

	std::thread t(runProcess, func, pid, par->argc, par->argv, par->switches);

	if (par->waitForProcess) {
		t.join();
	}
	else {
		t.detach();
	}
	return pid;
}