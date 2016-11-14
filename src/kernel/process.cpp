#include "process.h"
#include "kernel.h"
#include "io.h"
#include <iostream>
#include <thread>
#include <Windows.h>
#include <unordered_map>


std::mutex process_table_mtx; //mutex for process table
PCB * process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes
std::unordered_map< std::thread::id, int> TIDtoPID;


void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess:
		//int pid = -1;
		regs.Rax = (decltype(regs.Rax))createProcess((command_params *)regs.Rcx/*, &pid*/);
		//regs.Rbx = (decltype(regs.Rax))pid;
		Set_Error(regs.Rax != 0, regs);
		break;
	//default:
		//gtfo
	}
}

void runProcess(TEntryPoint func, int pid, int argc, char ** argv, char * switches) {

	TIDtoPID[std::this_thread::get_id()] = pid;

	CONTEXT regs;
	regs.R8 = (decltype(regs.R8))process_table[pid]->IO_descriptors[0]; //stdit
	regs.R9 = (decltype(regs.R9))process_table[pid]->IO_descriptors[1]; //stdout
	regs.R10 = (decltype(regs.R10))process_table[pid]->IO_descriptors[2]; //stderr
	regs.R11 = (decltype(regs.R11))process_table[pid]->IO_descriptors[3]; //current dir
	regs.R12 = (decltype(regs.R12))switches;
	regs.Rax = (decltype(regs.Rax))process_table[pid]->name;
	//regs.Rbx = (decltype(regs.Rbx))pid; //?
	regs.Rcx = (decltype(regs.Rcx))argc;
	regs.Rdx = (decltype(regs.Rdx))argv;

	size_t ret = func(regs);
	//TODO delat neco s navratovou hodnotou


	for (auto &handle : process_table[pid]->IO_descriptors) {
		int ret = close_file(handle);
		//ret? nemusi se povest zavrit? 
	}

	std::lock_guard<std::mutex> lock(process_table_mtx);
	delete process_table[pid];
	process_table[pid] = nullptr;
}

int createProcess(command_params * par/*, int *proc_pid*/)
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
		//v tabulce neni misto
		SetLastError(CREATE_PROCESS_ERROR);
		return 1;
	}

	process_table[pid]->pid = pid;
	for (auto &handle : par->handles) {
		process_table[pid]->IO_descriptors.push_back(handle);
	}
	process_table[pid]->name = par->name;
	
	TEntryPoint func = (TEntryPoint)GetProcAddress(User_Programs, par->name);
	if (!func) {
		//vstupni bod se nepovedlo nalezt v uzivatelskych programech
		std::lock_guard<std::mutex> lock(process_table_mtx);
		delete process_table[pid];
		process_table[pid] = nullptr;
		SetLastError(CREATE_PROCESS_ERROR);
		return 1;
	}

	std::thread t(runProcess, func, pid, par->argc, par->argv, par->switches); 

	if (par->waitForProcess) {
		t.join();
	}
	else {
		t.detach();
	}

	//*proc_pid = pid;
	return 0;
}