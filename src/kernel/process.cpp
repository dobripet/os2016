#include "process.h"
#include "kernel.h"
#include "io.h"
#include <thread>
#include <Windows.h>


std::mutex process_table_mtx; //mutex pro tabulky procesu
PCB * process_table[PROCESS_TABLE_SIZE] = { nullptr };//tabulka procesu pro max 1024 procesu
std::unordered_map< std::thread::id, int> TIDtoPID; //mapovani std::thread::id na nas pid

void HandleProcess(CONTEXT & regs) {
	switch (Get_AL((__int16)regs.Rax)) {

	case scCreateProcess: {
		int pid = -1;
		regs.Rax = (decltype(regs.Rax))createProcess((command_params *)regs.Rcx, &pid);
		regs.Rdx = (decltype(regs.Rdx))pid;
		Set_Error(regs.Rax == S_FALSE, regs);
		break;
	} 
	case scJoinProcess: {
		regs.Rax = (decltype(regs.Rax))joinProcess((int)regs.Rbx);
		Set_Error(regs.Rax == S_FALSE, regs);
		break;
	}
	case scGetProcesses: {
		regs.Rax = (decltype(regs.Rax))getProcesses((std::vector<process_info*>*)regs.Rbx);
		Set_Error(regs.Rax == S_FALSE, regs);
		break;
	}
	//default:
		//gtfo
	}
}


void runProcess(TEntryPoint func, int pid, char * arg) {

	{
		std::lock_guard<std::mutex> lock(process_table_mtx);
		//namapujeme id tohoto vlakna na pid noveho procesu
		TIDtoPID[std::this_thread::get_id()] = pid;
	}

	CONTEXT regs;
	regs.R8 = (decltype(regs.R8))process_table[pid]->IO_descriptors[0]; //stdit
	regs.R9 = (decltype(regs.R9))process_table[pid]->IO_descriptors[1]; //stdout
	regs.R10 = (decltype(regs.R10))process_table[pid]->IO_descriptors[2]; //stderr
	regs.R11 = (decltype(regs.R11))process_table[pid]->IO_descriptors[3]; //current dir
	regs.Rax = (decltype(regs.Rax))process_table[pid]->name;
	regs.Rcx = (decltype(regs.Rcx))arg;
	regs.R13 = (decltype(regs.R13))&(process_table[pid]->currentPath);

	//zavolame vstupni bod kodu procesu
	size_t ret = func(regs);

	//Zavreme stdin/out/err, soucasnou slozku a dalsi soubory, ktere zlobivy proces otevrel, ale nezavrel.
	for (int cnt = (int)process_table[pid]->IO_descriptors.size() - 1; cnt >= 0; cnt--) {
		auto &handle = process_table[pid]->IO_descriptors[cnt];
		int ret = close_file(handle);
		//ret? nemusi se povest zavrit? 
	}
}

HRESULT createProcess(command_params * par, int *proc_pid)
{
	int pid = -1;
	//najde prvni misto volne v tabulce procesu a vytvori novy PCB
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
		SetLastError(ERR_PROCESS_CREATE);
		return S_FALSE;
	}

	{
		std::lock_guard<std::mutex> lock(process_table_mtx);
		//predame procesu handly na soubory
		for (auto &handle : par->handles) {
			process_table[pid]->IO_descriptors.push_back(handle);
		}
		//Kernel neni samostatny proces (nema zaznam v PCB), takze kdyz je vytvaren prvni proces shellu,
		//je velikost process_table (a TIDtoPID) 0, a lze tam tezko nejaky PCB najit, abychom odtud odstranily handly.
		if (TIDtoPID.size() > 0) {
			//odstranime predane handly z PCB procesu (shell), ktery pro nove vytvareny proces vyrabel soubory.
			std::vector<FDHandle> &handles = process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors;
			for (auto &handle : par->handles) {
				handles.erase(std::remove(handles.begin(), handles.end(), handle), handles.end());
			}
		}
	}
	process_table[pid]->name = par->name;
	//nastavime procesu jeho kontext - slozku, ve ktere byl spusten
	getPathFromNode(opened_files_table[opened_files_table_instances[process_table[pid]->IO_descriptors[3]]->file]->node, &(process_table[pid]->currentPath));

	//hledame v uzivatelskych programech vstupni bod
	TEntryPoint func = (TEntryPoint)GetProcAddress(User_Programs, par->name);
	if (!func) {
		//vstupni bod se nepovedlo nalezt 
		std::lock_guard<std::mutex> lock(process_table_mtx);
		delete process_table[pid];
		process_table[pid] = nullptr;
		SetLastError(ERR_PROCESS_NOTFOUND);
		return S_FALSE;
	}

	//konecne spustime proces
	process_table[pid]->thr = std::thread(runProcess, func, pid, par->arg);
	*proc_pid = pid;
	return S_OK;
}

HRESULT joinProcess(int pid) {
	std::thread::id tid = process_table[pid]->thr.get_id();
	//pockame si na ukonceni procesu
	process_table[pid]->thr.join();
	std::lock_guard<std::mutex> lock(process_table_mtx);
	//smazeme jej z tabulek
	TIDtoPID.erase(tid);
	delete process_table[pid];
	process_table[pid] = nullptr;
	return S_OK;
}
HRESULT getProcesses(std::vector<process_info*> *all_info) {
	std::lock_guard<std::mutex> lock(process_table_mtx);
	//projde tabulku procesu
	for (size_t i = 0; i < PROCESS_TABLE_SIZE; i++) {
		if (process_table[i] != nullptr) {
			//pro kazdy existujici proces vytvori info a plni jej o vektoru
			process_info *info = new process_info;
			info->name = process_table[i]->name;
			info->pid = i;
			info->threadID = process_table[i]->thr.get_id();
			info->workingDir = process_table[i]->currentPath;
			all_info->push_back(info);
		}
	}
	return S_OK;
}