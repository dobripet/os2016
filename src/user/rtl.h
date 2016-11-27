#pragma once

#include "..\common\api.h"

#include <vector>
#include <string>

//vrati posledni chybu, ktera nastala v kernelu
size_t Get_Last_Error();

//do "out" souboru vypise chybovou hlasku podle LastErroru. Prilepi libovolny "prefix" pred tuto hlasku.
void Print_Last_Error(FDHandle out, std::string prefix);
void Print_Last_Error(FDHandle out);

//pro parsovani prikazu - implementace v souboru rtl_parser.cpp
bool splitByPipes(std::string line, std::vector<std::string> * commandsStr);
bool parseCommandRedirects(std::string commandStr, struct Parsed_command * parsedCommand);
bool parseCommandParams(std::string command, std::string *switches, std::vector<std::string> *args);
std::string get_error_message();

typedef struct Parsed_command {
	std::string com;
	bool redirectstdout;
	bool appendstdout;
	bool redirectstdin;
	std::string stdoutpath;
	std::string stdinpath;
	std::string arg;
} parsed_command;

//konec parsovani


/*********************************************************************************************************
IO
*/

//duplikuje old_handle do new_handle
bool Duplicate_File(FDHandle old_handle, FDHandle * new_handle); 

//otevre soubor v nasem FS se zaslanou cestou fname  
//return true, pokud vse OK
//do handle ulozi handle na soubor
//pokud soubor existuje, nesmaze data (vychozi chovani)
bool Open_File(FDHandle * handle, const char * fname, int mode);
//bool rewrite - prepsat data ci nikoliv 
bool Open_File(FDHandle * handle, const char * fname, int mode, bool rewrite);

//vyrobi rouru
//return true, pokud vse OK
//do writeHandle ulozi handle na zapisovaci konec roury, do readHandle cteci konec
bool Open_Pipe(FDHandle * writeHandle, FDHandle * readHandle);

//uzavre soubor identifikovany pomoci deskriptoru
//return true, pokud vse OK
bool Close_File(FDHandle file_handle);

//nahlednuti (neblokujici!) do souboru, kolik je toho v nem ke cteni
//funguje pro rouru a FS, ne pro konzoli
//return true, pokud vse OK
bool Peek_File(FDHandle handle, size_t *available);

//cteni souboru
//len je delka buferu, do ktereho bude zapsano
//do filled bude ulozen pocet, kolik se zapsalo
//return true, pokud vse OK
bool Read_File(FDHandle handle, size_t len, char * buf, size_t *filled);

//zapise do souboru identifikovaneho deskriptorem data z bufferu o velikosti buffer_size
//return true, pokud vse OK
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size);
//navic ulozi pocet zapsanych dat ve written
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size, size_t *written);

//vytvori slozku podle dane absolutni nebo relativni cesty path
//return true, pokud vse OK
bool Make_Dir(char *path);

//zmeni volajicmu procesu aktualni slozku (prikaz CD)
//return true, pokud vse OK
bool Change_Dir(char *path);

//smaze slozku se zaslanou relativni nebo absolutni cestou
//return true, pokud vse OK
bool Remove_Dir(char *path);

//smaze file se zaslanou relativni nebo absolutni cestou
//return true, pokud vse OK
bool Remove_File(char *path);

//zapise seznam nodu pro path do all_info
//return true, pokud vse OK
bool Get_Dir_Nodes(std::vector<node_info*> *all_info, char *path);

//konec IO

/*********************************************************************************************************
PROCES
*/

//vytvori a spusti ve vlakne novy proces, specifikovany zaslanymi parametry
//return true, pokud vse OK
bool Create_Process(command_params * par, int * pid);

//pocka na dokonceni procesu a uklidi po nem
bool Join_and_Delete_Process(int pid);

//zapise seznam procesu do all_info
//return true, pokud vse OK
bool Get_Processes(std::vector<process_info*> *all_info);

//konec PROCES