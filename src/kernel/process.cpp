#include "process.h"
#include "kernel.h"
#include "io.h"
#include <iostream>
#include <thread>
#include <Windows.h>


std::mutex process_table_mtx; //mutex for process table
PCB * process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes
std::unordered_map< std::thread::id, int> TIDtoPID;


void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess: {
		//std::thread t;
		int pid = -1;
		regs.Rax = (decltype(regs.Rax))createProcess((command_params *)regs.Rcx, &pid);
		regs.Rdx = (decltype(regs.Rdx))pid;
		Set_Error(regs.Rax != 0, regs);
		break;
	} 
	case scJoinProcess: {
		regs.Rax = (decltype(regs.Rax))joinProcess((int)regs.Rbx);
		Set_Error(regs.Rax != 0, regs);
		break;
	}
	//default:
		//gtfo
	}
}


void runProcess(TEntryPoint func, int pid, int argc, char ** argv, char * switches) {

	{
		std::lock_guard<std::mutex> lock(process_table_mtx);
		TIDtoPID[std::this_thread::get_id()] = pid;
	}

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
	regs.R13 = (decltype(regs.R13))&(process_table[pid]->currentPath);

	size_t ret = func(regs);

	for (auto &handle : process_table[pid]->IO_descriptors) {
		int ret = close_file(handle);
		//ret? nemusi se povest zavrit? 
	}

	//std::lock_guard<std::mutex> lock(process_table_mtx);
	//TIDtoPID.erase(std::this_thread::get_id());
//	delete process_table[pid];
//	process_table[pid] = nullptr;
}

int createProcess(command_params * par, int *proc_pid)
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
		return -1;
	}

	//process_table[pid]->pid = pid;
	for (auto &handle : par->handles) {
		process_table[pid]->IO_descriptors.push_back(handle);
	}
	process_table[pid]->name = par->name;
	//TODO getPathFromNode() misto node->name
	process_table[pid]->currentPath = opened_files_table[opened_files_table_instances[process_table[pid]->IO_descriptors[3]]->file]->node->name;

	TEntryPoint func = (TEntryPoint)GetProcAddress(User_Programs, par->name);
	if (!func) {
		//vstupni bod se nepovedlo nalezt v uzivatelskych programech
		std::lock_guard<std::mutex> lock(process_table_mtx);
		delete process_table[pid];
		process_table[pid] = nullptr;
		SetLastError(CREATE_PROCESS_ERROR);
		return -1;
	}

	//std::thread t(runProcess, func, pid, par->argc, par->argv, par->switches);
	process_table[pid]->thr = std::thread(runProcess, func, pid, par->argc, par->argv, par->switches);
	*proc_pid = pid;

	//*t = std::thread(runProcess, func, pid, par->argc, par->argv, par->switches);

	/*
	if (par->waitForProcess) {
		process_table[pid]->thr.join();
	}
	else {
		process_table[pid]->thr.detach();
	}
	*/
	return 0;
}


int joinProcess(int pid) {
	process_table[pid]->thr.join();
	std::lock_guard<std::mutex> lock(process_table_mtx);
	TIDtoPID.erase(std::this_thread::get_id());
	delete process_table[pid];
	process_table[pid] = nullptr;
	return 0;
}