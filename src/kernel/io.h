#pragma once

#include "filesystem.h"
#include "pipe.h"
#include "..\common\api.h"

void initSystemIO();
//void closeSystemIO();

void HandleIO(CONTEXT &regs);

const int OPEN_FILES_TABLE_SIZE = 2056;
const int OPEN_INSTANCES_TABLE_SIZE = 4* OPEN_FILES_TABLE_SIZE;

const int F_MODE_READ = 7;
const int F_MODE_WRITE = 8;
const int F_MODE_BOTH = 9;
const int F_MODE_CLEAR_WRITE = 10;
const int F_MODE_CLEAR_BOTH = 11;

const int F_TYPE_STD = 1;
const int F_TYPE_FILE = 2;
const int F_TYPE_PIPE = 3;


typedef struct opened_file {

	unsigned int openCount = 0; //kolikrat je tento soubor otevren
	unsigned int FILE_TYPE; // std / pipe / node
	node *node = nullptr; //node pokud je to file v systemu
	pipe *pipe = nullptr; //pipe pokud je to konec pipy
	THandle std = 0; //windows handle pokud je to std io

} FD;

typedef struct opened_file_instance {

	int pos = 0; //pozice pri cteni
	int mode = 0; //write / read / both
	FDHandle file; //index of underlying opened file

} FD_instance;


//TODO peek_file(handle, *kolik_tam_je int) (kvuli tomu, abych se nezablokoval na roure, kdyz jenom chci vedet, esi nahodou nemam uz EOF - napr. pro RGEN)
//TODO change where handle points to (kvuli CD)
int peek_file(FDHandle handle, size_t *available);

int change_dir(char * path);

int duplicate_handle(FDHandle orig_handle, FDHandle * duplicated_handle);
int open_pipe(FDHandle * whandle, FDHandle * rhandle);
int open_file(char *path, int MODE, FDHandle * handle);
int close_file(FDHandle handle);
size_t read_file(FDHandle handle, size_t howMuch, char * buf);
size_t write_file(FDHandle handle, size_t howMuch, char * buf);
HRESULT mkdir(char *path);

extern opened_file * opened_files_table[OPEN_FILES_TABLE_SIZE];
extern opened_file_instance * opened_files_table_instances[OPEN_INSTANCES_TABLE_SIZE];