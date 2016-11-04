#pragma once

#include "..\common\api.h"

#include <vector>
#include <string>
#include "..\kernel\filesystem.h"
#include "..\kernel\process.h"
#include "..\kernel\io.h"

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

/*
typedef struct process_params {

	THandle STDIN, STDOUT, STDERR;
	std::vector<std::string> ARGV;
	std::string name; 
	node *current_dir, *root_dir;
	
} run_params;
*/

size_t Get_Last_Error();

THandle Create_File(const char* file_name, size_t flags);
		//podle flags otevre, vytvori soubor a vrati jeho deskriptor
		//vraci nenulovy handle, kdyz vse OK
bool Write_File(const THandle file_handle, const void *buffer, const size_t buffer_size, size_t &written);
		//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve writtent
		//vraci true, kdyz vse OK
bool Close_File(const THandle file_handle);
		//uzavre soubor identifikovany pomoci deskriptoru
		//vraci true, kdyz vse OK

//bool Create_Process(void(*func)(PCB * pcb, std::vector<std::string> argv), run_params * params);
//bool Create_Process(int(*function)(int argc, char* argv[]), int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err);
//bool Create_Process(TEntryPoint * func, int argc, char *argv[], const char*  name, const char*  current_dir, const char*  root_dir, THandle in, THandle out, THandle err);
bool Create_Process(/*TEntryPoint * func,*/ command_params * par);
		//TODO slozky predelat
		//vytvori novy a spusti novy process
		//vraci true, kdyz vse OK
