#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "filesystem.h"
#include "..\common\api.h"


const size_t PROCESS_TABLE_SIZE = 1024;
const size_t CREATE_PROCESS_ERROR = 211;
const size_t CREATE_PROCESS_OK = 0;

void HandleProcess(CONTEXT &regs);

typedef struct process_control_block {

	unsigned int pid;
	const char * name;
	node *current_dir, *root_dir;//mozna node asi
	std::vector<THandle> IO_decriptors; //tabulka souboru daneho procesu index by mel byt file descriptor
	//0 = stdin, 1 = stdout, 2 = stderr

} PCB;//kazdej proces bude mit tuto strukturu

typedef struct create_process_params {

	THandle STDOUT, STDIN, STDERR;
	THandle *current_node, *root_node;
	char * switches;
	char ** argv;
	int argc;
	const char * name;
	bool waitForProcess; 

} command_params;


int createProcess(command_params * par);

extern PCB *process_table[1024];