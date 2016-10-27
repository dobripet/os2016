#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "filesystem.h"
#include "..\common\api.h"

const size_t PROCESS_TABLE_SIZE = 1024;
const size_t CREATE_PROCESS_ERROR = 211;

void HandleProcess(CONTEXT &regs);

/*
struct open_file {
	unsigned int count;
	unsigned int flags;
	node *node;
};//kernel v sobe bude drzet tuto tabulku
*/

typedef struct process_control_block {

	unsigned int pid;
	std::string name;
	node *current_dir, *root_dir;//mozna node asi
	std::vector<THandle> IO_decriptors; //tabulka souboru daneho procesu index by mel byt file descriptor
	//0 = stdin, 1 = stdout, 2 = stderr

} PCB;//kazdej proces bude mit tuto strukturu

typedef struct create_process_params {

	THandle STDOUT, STDIN, STDERR;
	THandle *current_node, *root_node;
	std::vector<std::string> params;
	std::string name;

} command_params;


//int createProcess(void(*func)(PCB * pcb, std::vector<std::string> argv), run_params* params);
//int createProcess(TEntryPoint * func, int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err);
int createProcess(TEntryPoint * func, command_params * par);
