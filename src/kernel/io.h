#pragma once

#include "filesystem.h"
#include "pipe.h"
#include "..\common\api.h"

void initIO();

void HandleIO(CONTEXT &regs);

#define OPEN_FILES_TABLE_SIZE 2056
/*
Flags
 0 - stdin celeho os
 1 - stdout celeho os
 2 - stderr celeho os
 3 - pipe vstup
 4 - pipe vystup
 5 - soubor
 TODO - rozdelit soubor na vic typu asi
*/
#define OF_FLAGS_OS_STDIN 0
#define OF_FLAGS_OS_STDOUT 1
#define OF_FLAGS_OS_STDERR 2
#define OF_FLAGS_PIPE_IN 3
#define OF_FLAGS_PIPE_OUT 4
#define OF_FLAGS_FILE 5
typedef struct open_file {
	unsigned int count; // pocet otevreni
	unsigned int flags; 
	node *node; //node pokud je to file v systemu
	pipe *pipe; //pipe pokud je to konec pipy
	THandle std; //windows handle pokud je to std io
} open_file;//kernel v sobe bude drzet tuto tabulku

typedef struct write_params {
	unsigned int pid;
	THandle handle;
	const void *buffer;
	unsigned long size;
	unsigned long *written;
} write_params;

unsigned long write_file(write_params *par);