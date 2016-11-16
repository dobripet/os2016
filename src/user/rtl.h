#pragma once

#include "..\common\api.h"

#include <vector>
#include <string>

size_t Get_Last_Error();


/*********************************************************************************************************
IO
*/

bool Open_File(FDHandle old_handle, FDHandle * new_handle);

//otevre soubor v nasem FS se zaslanou cestou fname  
//return true kdyz vse OK
//do handle ulozi handle na soubor
bool Open_File(FDHandle * handle, const char * fname, int mode); //jeste chybi typ esi slozka nebo soubor
//a jeste mozna bude potreba posilat current_node

//vyrobi rouru
//return true kdyz vse OK
//do writeHandle ulozi handle na zapisovaci konec roury, do readHandle cteci konec
bool Open_Pipe(FDHandle * writeHandle, FDHandle * readHandle);

//uzavre soubor identifikovany pomoci deskriptoru
//vraci true, kdyz vse OK
bool Close_File(FDHandle file_handle);

bool Peek_File(FDHandle handle, size_t *available);

bool Read_File(FDHandle handle, size_t len, char * buf, size_t *filled);

//THandle Create_File(const char* file_name, size_t flags);
		//podle flags otevre, vytvori soubor a vrati jeho deskriptor
		//vraci nenulovy handle, kdyz vse OK

bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size, size_t *written);
bool Write_File(FDHandle file_handle, char *buffer, size_t buffer_size);
		//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve writtent
		//vraci true, kdyz vse OK
//bool Close_File(const THandle file_handle);
		//uzavre soubor identifikovany pomoci deskriptoru
		//vraci true, kdyz vse OK

/*Vytvori slozku podle dane absolutni nebo relativni cesty*/
bool Make_Dir(char *path);
bool Change_Dir(char *path);
bool Remove_Dir(char *path);


/*********************************************************************************************************
PROCES
*/

//vytvori a spusti ve vlakne novy proces, specifikovany zaslanymi parametry
//vraci true, kdyz vse OK
bool Create_Process(command_params * par, int * pid);
bool Join_and_Delete_Process(int pid);
