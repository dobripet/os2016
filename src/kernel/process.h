#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "filesystem.h"
#include "..\common\api.h"


const size_t PROCESS_TABLE_SIZE = 1024;

const size_t CREATE_PROCESS_ERROR = 211;
const size_t CREATE_PROCESS_OK = 0;

void HandleProcess(CONTEXT &regs);

typedef struct process_control_block {

	std::thread thr;
	//unsigned int pid;
	const char * name;
	//node *current_dir, *root_dir;//mozna node asi
	std::vector<FDHandle> IO_descriptors; //tabulka souboru daneho procesu index by mel byt file descriptor
	//0 = stdin, 1 = stdout, 2 = stderr, 3 = current folder
	std::string currentPath;



} PCB;//kazdej proces bude mit tuto strukturu



int joinProcess(int pid);
int createProcess(command_params * par, int *pid /*, std::thread * t*/);

extern PCB *process_table[PROCESS_TABLE_SIZE];
extern std::unordered_map< std::thread::id, int> TIDtoPID;