#pragma once

#include "..\common\api.h"

#include <vector>
#include <string>

size_t Get_Last_Error();

/*********************************************************************************************************
IO
*/

//duplikuje old_handle do new_handle
bool Duplicate_File(FDHandle old_handle, FDHandle * new_handle); 

//otevre soubor v nasem FS se zaslanou cestou fname  
//return true kdyz vse OK
//do handle ulozi handle na soubor
//pokud soubor existuje, nesmaze data
bool Open_File(FDHandle * handle, const char * fname, int mode); //jeste chybi typ esi slozka nebo soubor
//rewrite - prepsat data ci nikoliv
bool Open_File(FDHandle * handle, const char * fname, int mode, bool rewrite);

//vyrobi rouru
//return true kdyz vse OK
//do writeHandle ulozi handle na zapisovaci konec roury, do readHandle cteci konec
bool Open_Pipe(FDHandle * writeHandle, FDHandle * readHandle);

//uzavre soubor identifikovany pomoci deskriptoru
//vraci true, kdyz vse OK
bool Close_File(FDHandle file_handle);

//bool Peek_File(FDHandle handle, size_t *available);

//cteni souboru
//len je delka buferu, do ktereho bude zapsano
//do filled bude ulozen pocet, kolik se zapsalo
bool Read_File(FDHandle handle, size_t len, char * buf, size_t *filled);

//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
//vraci true, kdyz vse OK
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size, size_t *written);
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size);

/*Vytvori slozku podle dane absolutni nebo relativni cesty*/
bool Make_Dir(char *path);

/*zmeni volajicmu procesu aktualni slozku (prikaz CD)*/
bool Change_Dir(char *path);

/*smaze slozku se zaslanou relativni nebo absolutni cestou*/
bool Remove_Dir(char *path);
/*smaze file se zaslanou relativni nebo absolutni cestou*/
bool Remove_File(char *path);


/*********************************************************************************************************
PROCES
*/

//vytvori a spusti ve vlakne novy proces, specifikovany zaslanymi parametry
//vraci true, kdyz vse OK
bool Create_Process(command_params * par, int * pid);

//pocka na dokonceni procesu a uklidi po nem
bool Join_and_Delete_Process(int pid);
//Vrati seznam procesu
bool Get_Processes(std::vector<process_info*> *all_info);

//Vrati seznam nodu
bool Get_Dir_Nodes(std::vector<node_info*> *all_info, char *path);