#pragma once

#include <Windows.h>
#include <vector>

typedef HANDLE THandle; 
typedef int FDHandle; //handle na nas soubor

typedef size_t(__stdcall *TEntryPoint)(const CONTEXT syscall);		//vstupni bod uzivatelskeho programu
typedef void(__stdcall *TSysCall)(CONTEXT &context);			//prototyp funkce, ktera realizuje syscall

const __int64 CarryFlag = 1;

/*Error codes*/
const int ERR_NOERROR = 0x0; //zadny error
const int ERR_PROCESS_CREATE = 0x101; //proces nejde vyrobit
const int ERR_PROCESS_NOTFOUND = 0x102; //nenalezen vstupni bod procesu
const int ERR_IO_FILE_CREATE = 0x205; //soubor nejde vyrobit
const int ERR_IO_PATH_NOEXIST = 0x201; //cesta neexistuje
const int ERR_IO_FILE_ISFILE = 0x202; //node je soubor (kdyz chceme neco delat se slozkou)
const int ERR_IO_FILE_ISFOLDER = 0x203; //node je slozka (kdyz chceme neco delat se souborem)
const int ERR_IO_FILE_ISOPENED = 0x204; //node nekdo pouziva (nejde smazat)
const int ERR_IO_FILE_NOTEMPTY = 0x206; //node neni prazdnej (slozka nejde smazat)

/*IO konstanty*/
const int F_MODE_READ = 7;
const int F_MODE_WRITE = 8;
const int F_MODE_BOTH = 9;
const int F_MODE_CLEAR_WRITE = 10;
const int F_MODE_CLEAR_BOTH = 11;

/*struktura pro predani parametru spousteni procesu*/
typedef struct create_process_params {
	std::vector <FDHandle> handles; //0=STDIN, 1=STDOUT, 2=STDERR, 3=SLOZKA kde se proces nachazi
	char * switches; //prepinace programu
	char ** argv; //argumenty programu
	int argc; //pocet argumentu
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

//al hodnoty pro scProcess
const __int8 scCreateProcess = 1;
const __int8 scJoinProcess = 2;

constexpr __int16 Compose_AX(const __int8 ah, const __int8 al) {
	return (ah << 8) | al;
}

constexpr __int8  Get_AH(const __int16 ax) {
	return ax >> 8;
}

constexpr __int8  Get_AL(const __int16 ax) {
	return ax & 0xFF;
}

