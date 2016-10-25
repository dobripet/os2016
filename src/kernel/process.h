#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "filesystem.h"
#include "..\common\api.h"

#define PROCESS_TABLE_SIZE 1024
#define CREATE_PROCESS_ERROR 211

void HandleProcess(CONTEXT &regs);

struct open_file {
	unsigned int count;
	unsigned int flags;
	node *node;
};//kernel v sobe bude drzet tuto tabulku

struct pcb{
	unsigned int pid;
	std::string name;

	std::string current_dir;//mozna node asi
	std::vector<THandle> files;//tabulka souboru daneho procesu index by mel byt file descriptor
};//kazdej proces bude mit tuto strukturu
