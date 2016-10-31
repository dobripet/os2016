#include "process.h"
#include "kernel.h"
#include <iostream>
#include <thread>
#include <Windows.h>

std::mutex process_mtx; //mutex for process table
PCB* process_table[PROCESS_TABLE_SIZE] = { nullptr };//process table with max 1024 processes


//void runProcess(std::unique_ptr<TEntryPoint> func, std::unique_ptr<CONTEXT> regs, std::unique_ptr<int> pid) {
void runProcess(/*TEntryPoint func, */CONTEXT regs/*, int pid*/) {

	int pid = (int)(regs.R8);
	TEntryPoint func = (TEntryPoint)(regs.Rdx);
	size_t returnValue = func(regs);

	//uklid:
	process_mtx.lock();
	/*
		ZAPSAT EOF NA STDOUT
		ZAVRIT OTEVRENE SOUBORY
	*/
	process_table[pid] = nullptr;
	//delete regs;
	process_mtx.unlock();
}

int createProcess(/*TEntryPoint * func,*/ command_params * par)
{
	PCB newProc;
	newProc.IO_decriptors.push_back(par->STDIN);
	newProc.IO_decriptors.push_back(par->STDOUT);
	newProc.IO_decriptors.push_back(par->STDERR);
	newProc.name = par->name;
	//newProc.current_dir = ...
	//newProc.root_dir = ...

	process_mtx.lock();
//	std::unique_ptr<int> pid = std::make_unique<int>();
	//*pid = -1;
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
	else {

		TEntryPoint func = (TEntryPoint)GetProcAddress(User_Programs, par->name.c_str());
		if (!func) {
			//TODO return error
			SetLastError(CREATE_PROCESS_ERROR);
			return pid;
		}

		//std::unique_ptr<CONTEXT> regs = std::make_unique<CONTEXT>();
		CONTEXT regs;
		//CONTEXT *regs = new CONTEXT();
		//regs->Rbx = (decltype(regs->Rbx))&newProc;
		//regs->Rcx = (decltype(regs->Rcx))&(par->params);
		regs.Rbx = (decltype(regs.Rbx))&newProc;
		//std::vector<std::string> stringpar = std::move(par->params);
		regs.Rcx = (decltype(regs.Rcx))&(par->params);
		regs.Rdx = (decltype(regs.Rdx))func;
		regs.R8 = pid;
		//std::unique_ptr<TEntryPoint> f(std::move(func));
		//std::thread t(runProcess, std::move(f), std::move(regs) , std::move(pid));
		std::thread t(runProcess, regs);
		//runProcess(*func, regs, pid);
		//t.join();

		if (par->wait) {
			t.join();
		}
		else {
			t.detach();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));  //bez cekani se to  zatim posere
		}

	}
	return pid;
}


void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {
	case scCreateProcess:
		regs.Rax = (decltype(regs.Rax))createProcess(/*(TEntryPoint *) regs.Rbx,*/ (command_params *)regs.Rcx);
		//regs.Rax = (decltype(regs.Rax))createProcess((runFunctionPointer) regs.Rbx, (run_params*)regs.Rcx);
		//regs.Rax = (decltype(regs.Rax))createProcess((int(*)(int, char*[])) regs.Rbx, regs.Rcx, (char**) regs.Rdx, (char*)regs.R8, (char*)regs.R9, (char*)  regs.R10, (THandle)regs.R11, (THandle)regs.R12, (THandle)regs.R13);
		//regs.Rax = (decltype(regs.Rax))createProcess((TEntryPoint *)regs.Rbx, regs.Rcx, (char**)regs.Rdx, (char*)regs.R8, (char*)regs.R9, (char*)regs.R10, (THandle)regs.R11, (THandle)regs.R12, (THandle)regs.R13);
		Set_Error(regs.Rax == -1, regs);
		break;
	}
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