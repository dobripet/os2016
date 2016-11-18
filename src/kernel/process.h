#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "filesystem.h"
#include "..\common\api.h"

const size_t PROCESS_TABLE_SIZE = 1024;

void HandleProcess(CONTEXT &regs);

typedef struct process_control_block {

	std::thread thr;
	const char * name;
	std::vector<FDHandle> IO_descriptors; //tabulka souboru daneho procesu; 0 = stdin, 1 = stdout, 2 = stderr, 3 = current folder
	std::string currentPath;

} PCB;


int joinProcess(int pid); 
int getProcesses(std::vector<process_info*> *all_info);
int createProcess(command_params * par, int *pid);

extern PCB *process_table[PROCESS_TABLE_SIZE];
extern std::unordered_map< std::thread::id, int> TIDtoPID;