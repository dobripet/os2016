#pragma once

#include <Windows.h>
#include <vector>

const int ERR_PROCESS_CREATE = 0x101;
const int ERR_IO_PATH_NOEXIST = 0x201;
const int ERR_IO_FILE_ISFILE = 0x202;
const int ERR_IO_FILE_ISFOLDER = 0x203;
const int ERR_IO_FILE_ISOPENED = 0x204;
const int ERR_IO_DIR_EXIST = 0x205;
const int ERR_IO_DIR_NOTEMPTY = 0x206;

typedef HANDLE THandle;
typedef int FDHandle;

typedef size_t (__stdcall *TEntryPoint)(const CONTEXT syscall);		//vstupni bod uzivatelskeho programu
typedef void (__stdcall *TSysCall)(CONTEXT &context);			//prototyp funkce, ktera realizuje syscall


/*

   Cisla funkci OS:
	 AH - major cislo aka id skupiny fci
	 AL - minor cislo aka cisle konkretni fce

	 je po ovlani nastavena vlajka carry, tj. CONTEXT::Eflags & CF != 0, pak Rax je kod chyby

      AH == 1 : IO operace
		AL: cislo IO operace	//konzole je take jenom soubor
			1 - otevrit soubor				 IN: rdx je pointer na null-terminated string udavajici file_name; rcx jsou flags k otevreni souboru
											OUT: rax je handle nove otevreneho souboru
			2 - zapis do souboru			 IN: rdx je handle souboru, rdi je pointer na buffer, rcx je pocet bytu v bufferu k zapsani
											OUT: rax je pocet zapsanych bytu
			3 - cti ze souboru
			4 - nastav pozici v souboru
			5 - zavri soubor				 IN: rdx  je handle souboru k zavreni
											OUT: rax != 0 je uspech, jinak chyba


												

   Dalsi cisla si doplnte dle potreby


*/


const __int64 CarryFlag = 1;
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
//const __int8 scCreateFile = 1;
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

/*structure for passing params to crate process*/
typedef struct create_process_params {

	std::vector <FDHandle> handles; //0=STDIN, 1=STDOUT, 2=STDERR, 3=SLOZKA kde se proces nachazi
									//FDHandle STDOUT, STDIN, STDERR;
									//char * current_path; //pouze kvuli informaci pro uzivatele, jinak neni na nic potreba
	char * switches;
	char ** argv;
	int argc;
	const char * name;
	//	bool waitForProcess; 

} command_params;


/*IO constants*/
const int F_MODE_READ = 7;
const int F_MODE_WRITE = 8;
const int F_MODE_BOTH = 9;
const int F_MODE_CLEAR_WRITE = 10;
const int F_MODE_CLEAR_BOTH = 11;

const int F_TYPE_STD = 1;
const int F_TYPE_FILE = 2;
const int F_TYPE_PIPE = 3;