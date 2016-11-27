#pragma once

#include "filesystem.h"
#include "pipe.h"
#include "..\common\api.h"

//otevre prvni 4 handly pro shell
void initSystemIO();
//roztridi syscally podle AL(podle typu scOpenFile atd..)
void HandleIO(CONTEXT &regs);
//velikosti tabulek
const int OPEN_FILES_TABLE_SIZE = 2056;
const int OPEN_INSTANCES_TABLE_SIZE = 4* OPEN_FILES_TABLE_SIZE;
//typy otevrenych souboru
const int F_TYPE_STD = 1;
const int F_TYPE_FILE = 2;
const int F_TYPE_PIPE = 3;
//struktura otevreneho souboru v systemu
typedef struct opened_file {

	unsigned int openCount = 0; //kolikrat je tento soubor otevren
	unsigned int FILE_TYPE; // std / pipe / node
	node *node = nullptr; //node pokud je to file v systemu
	pipe *pipe = nullptr; //pipe pokud je to konec pipy
	THandle std = 0; //windows handle pokud je to std io

} FD;
//struktura otevrene instance otevreneho souboru v systemu
typedef struct opened_file_instance {

	size_t pos = 0; //pozice pri cteni
	int mode = 0; //write / read / both
	FDHandle file; //index do tabulky otevrenych souboru

} FD_instance;

//zmena aktualni slozky volajiciho procesu (prikaz CD)
HRESULT change_dir(char * path);
//smaze slozku podle cesty
HRESULT remove_dir(char * path);
//vytvori slozku podle cesty
HRESULT mkdir(char *path);

//zduplikuje handle na soubor otevreny pod orig_handle
HRESULT duplicate_handle(FDHandle orig_handle, FDHandle * duplicated_handle);
//otevre rouru, vraci zapisovaci a cteci konec
HRESULT open_pipe(FDHandle * whandle, FDHandle * rhandle);
//otevre soubor, handle je soucasna slozka volajiciho procesu, path je relativni (muze byt i absolutni) cesta k teto slozce, rewrite jestli se maji data prepsat
HRESULT open_file(char *path, int MODE, bool rewrite, FDHandle * handle);
//zavre soubour otevreny pod handle
HRESULT close_file(FDHandle handle);
//smaze soubor podle cesty v path
HRESULT remove_file(char * path);

HRESULT peek_file(FDHandle handle, size_t *available);
//cteni ze souboru(handle jaky, kolik znaku, buffer k naplneni a naplni kolik bylo uspesne naplneno)
HRESULT read_file(FDHandle handle, size_t howMuch, char * buf, size_t * read);
//zapis do souboru(handle jaky, kolik znaku, buffer se znaky a naplni kolik bylo uspesne zapsano)
HRESULT write_file(FDHandle handle, size_t howMuch, char * buf, size_t *written);
//listovani dane slozky do all_info, aktualni adresar vyzdy na pozici 0
HRESULT getDirNodes(std::vector<node_info*> *all_info, char *path);
//tabulka otevrenych souboru 
extern opened_file * opened_files_table[OPEN_FILES_TABLE_SIZE];
//tabulka otevrenych instanci
extern opened_file_instance * opened_files_table_instances[OPEN_INSTANCES_TABLE_SIZE];