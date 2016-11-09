#pragma once

#include "filesystem.h"
#include "pipe.h"
#include "..\common\api.h"


void initSystemIO();
void closeSystemIO();

void HandleIO(CONTEXT &regs);

const int OPEN_FILES_TABLE_SIZE = 2056;
const int OPEN_INSTANCES_TABLE_SIZE = 4* OPEN_FILES_TABLE_SIZE;

/*
#define OF_FLAGS_OS_STDIN 0
#define OF_FLAGS_OS_STDOUT 1
#define OF_FLAGS_OS_STDERR 2
#define OF_FLAGS_PIPE_IN 3
#define OF_FLAGS_PIPE_OUT 4
#define OF_FLAGS_FILE 5
*/

const int F_MODE_READ = 7;
const int F_MODE_WRITE = 8;
const int F_MODE_BOTH = 9;

const int F_TYPE_STD = 1;
const int F_TYPE_FILE = 2;
const int F_TYPE_PIPE = 3;

/*
const int FILE_TYPE_OS_STDERR = 2;
const int FILE_TYPE_PIPE_IN = 3;
const int FILE_TYPE_PIPE_OUT = 4;
const int FILE_TYPE_FS = 5;
*/

typedef struct opened_file {

	unsigned int openCount = 0; //kolikrat je tento soubor otevren
	unsigned int FILE_TYPE; // std / pipe / node
	node *node = nullptr; //node pokud je to file v systemu
	pipe *pipe = nullptr; //pipe pokud je to konec pipy
	THandle std = 0; //windows handle pokud je to std io

} FD;

typedef struct opened_file_instance {

	int pos = 0; //pozice pri cteni
	int mode = 0; //write / read
	FDHandle file; //index of underlying opened file

} FD_instance;

/*
typedef struct write_params {
	unsigned int caller_pid;
	FDHandle HANDLE;
	const void *buffer;
	unsigned long size;
	unsigned long *written;
} write_params;
*/

/*
typedef struct open_params {

	int OPEN_MODE; //read/write/both
	char * path;
	
} open_params;
*/

int open_pipe(FDHandle * whandle, FDHandle * rhandle);
int open_file(char *path, int MODE, FDHandle * handle);
int close_file(FDHandle handle);
unsigned long read_file(FDHandle handle, int howMuch, char * buf);
unsigned long write_file(FDHandle handle, int howMuch, char * buf);