#pragma once
#include "..\common\api.h"
#include "filesystem.h"
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

const size_t PROCESS_TABLE_SIZE = 1024;
//roztridi syscally podle AL(podle typu scCreateProcess atd..)
void HandleProcess(CONTEXT &regs);
//struktura predstavujici process v systemu, handly souboru ukazuji do tabulky instanci
typedef struct process_control_block {
	std::thread thr; //vlakno procesu
	const char * name;	//jmeno procesu
	std::vector<FDHandle> IO_descriptors; //tabulka souboru daneho procesu; 0 = stdin, 1 = stdout, 2 = stderr, 3 = current folder
	std::string currentPath; //pracovni adresar procesu

} PCB;


HRESULT joinProcess(int pid);
//listovani bezicich procesu do all_info
HRESULT getProcesses(std::vector<process_info*> *all_info);
//vytvori novy proces z predanych parametru a vrati pid
HRESULT createProcess(command_params * par, int *pid);
//tabulka procesu
extern PCB *process_table[PROCESS_TABLE_SIZE];
//mapovani threadu na proces
extern std::unordered_map< std::thread::id, int> TIDtoPID;