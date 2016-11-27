#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>

typedef HANDLE THandle; 
typedef int FDHandle; //handle na nas soubor

typedef size_t(__stdcall *TEntryPoint)(const CONTEXT syscall);		//vstupni bod uzivatelskeho programu
typedef void(__stdcall *TSysCall)(CONTEXT &context);			//prototyp funkce, ktera realizuje syscall

const __int64 CarryFlag = 1;

//Error codes
const int ERR_NOERROR = 0x0; //zadny error
const int ERR_PROCESS_CREATE = 0x101; //proces nejde vyrobit
const int ERR_PROCESS_NOTFOUND = 0x102; //nenalezen vstupni bod procesu
const int ERR_IO_FILE_CREATE = 0x205; //soubor nejde vyrobit
const int ERR_IO_PATH_NOEXIST = 0x201; //cesta neexistuje
const int ERR_IO_FILE_ISNOTFOLDER = 0x202; //node je soubor (kdyz chceme neco delat se slozkou)
const int ERR_IO_FILE_ISNOTFILE = 0x203; //node je slozka (kdyz chceme neco delat se souborem)
const int ERR_IO_FILE_ISOPENED = 0x204; //node nekdo pouziva (nejde smazat)
const int ERR_IO_FILE_NOTEMPTY = 0x206; //node neni prazdnej (slozka nejde smazat)
const int ERR_IO_WRITE_STD = 0x207; //neslo zapsat na STDOUT (konzole).. neni nase chyba
const int ERR_IO_PIPE_READCLOSED = 0x208; //roura je zavrena pro cteni (tj. nelze zapisovat)
const int ERR_IO_READ_STD = 0x209; //neslo cist ze STDIN (konzole).. neni nase chyba
const int ERR_IO_FOLDER_EXISTS = 0x210; //slozka uz existuje (prikaz md)
const int ERR_IO_FILE_READ_ONLY = 0x211; //soubor otevren pouze pro cteni
const int ERR_IO_FILE_WRITE_ONLY = 0x212; //soubor otevren pouze pro zapis


//IO konstanty
const int F_MODE_READ = 7;
const int F_MODE_WRITE = 8;
const int F_MODE_BOTH = 9;
const bool F_OPEN_REWRITE = true;
const bool F_OPEN_APPEND = false;

const int TYPE_DIRECTORY = 0;
const int TYPE_FILE = 1;

//struktura pro predani vlastnosti procesu
typedef struct get_process_info {
	size_t pid; // pid procesu
	std::thread::id threadID; //id threadu
	std::string name; //jmeno programu
	std::string workingDir; //slozka kde je proces spusten
} process_info;

//struktura pro predani vlastnosti uzlu
typedef struct get_node_info {
	std::string name; //jmeno nodu
	size_t size; // velikost nodu
	int type; //typ nodu
	std::string path;//absolutni cesta
	std::string pathParent;//absolutni cesta rodice
} node_info;

//struktura pro predani parametru spousteni procesu
typedef struct create_process_params {
	std::vector <FDHandle> handles; //0=STDIN, 1=STDOUT, 2=STDERR, 3=SLOZKA kde se proces nachazi
	char * arg; //argumenty a prepinace (jako jeden string)
	const char * name; //jmeno programu
} command_params;


constexpr DWORD clc(const DWORD flags) {
	return flags & (~CarryFlag);
}

constexpr DWORD stc(const DWORD flags) {
	return flags | CarryFlag;
}

constexpr bool test_cf(const DWORD flags) {
	return flags & CarryFlag;
}

//ah hodnoty 
const __int8 scIO = 1;		//IO operace
const __int8 scProcess = 2;		//Process operace

//al hodnoty pro scIO 
const __int8 scWriteFile = 2;
const __int8 scOpenFile = 3;
const __int8 scCreatePipe = 4;
const __int8 scCloseFile = 5;
const __int8 scReadFile = 6;
const __int8 scDuplicateHandle = 7;
const __int8 scPeekFile = 8;
const __int8 scMakeDir = 9;
const __int8 scChangeDir = 10;
const __int8 scRemoveDir = 11;
const __int8 scRemoveFile = 12;
const __int8 scGetDirNodes = 13;

//al hodnoty pro scProcess
const __int8 scCreateProcess = 1;
const __int8 scJoinProcess = 2;
const __int8 scGetProcesses = 3;

constexpr __int16 Compose_AX(const __int8 ah, const __int8 al) {
	return (ah << 8) | al;
}

constexpr __int8  Get_AH(const __int16 ax) {
	return ax >> 8;
}

constexpr __int8  Get_AL(const __int16 ax) {
	return ax & 0xFF;
}

