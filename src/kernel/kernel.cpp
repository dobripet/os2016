#pragma once

#include "kernel.h"
#include "io.h"
#include "process.h"
#include "filesystem.h"
#include <iostream>
#include <string>
#include <vector>
#include "..\common\api.h"

HMODULE User_Programs;

void Set_Error(const bool failed, CONTEXT &regs) {
	if (failed) {
		stc(regs.EFlags);
		regs.Rax = GetLastError();
	}
	else
		clc(regs.EFlags);
}

void Initialize_Kernel() {
	User_Programs = LoadLibrary(L"user.dll");	
	initSystemIO();//vytvori handly 0,1,2 pro standardni io
}

void Shutdown_Kernel() {
	closeSystemIO();//uvolni handly 0,1,2 pro standardni io
	FreeLibrary(User_Programs);
}

void __stdcall SysCall(CONTEXT &regs) {

	switch (Get_AH((__int16) regs.Rax)) {
		case scIO:	
			HandleIO(regs); 
			break;
		case scProcess:		
			HandleProcess(regs);
			break;
	}
}

void __stdcall Run_VM() {

	Initialize_Kernel();

	command_params par;
	par.STDIN = (FDHandle) 0; //realnej soubor pokud bude presmerovani vstupu do naseho programu?
	par.STDOUT = (FDHandle) 1; //realnej soubor pokud bude presmerovani vystup z naseho programu?
	par.STDERR = (FDHandle) 2;

	par.name = "shell";
	//par.current_node = zde ROOT
	par.waitForProcess = true; //musime na nej pockat

	int pid = createProcess(&par);
	if (pid == - 1) {
		//oznamit error , ze nesel spustit shell
		return;
	}


	/*
	std::cout << std::endl;
	createFile(TYPE_FILE, "C://", "aaa.txt", "lorem ipsum");
	createFile(TYPE_FILE, "C://", "C://bbb.txt", "lorem ipsum");
	createFile(TYPE_DIRECTORY, "C://", "zcu", "lorem ipsum");
	createFile(TYPE_FILE, "C://zcu", "ccc.txt", "lorem ipsum");
	createFile(TYPE_DIRECTORY, "C://zcu", "/prvak", "lorem ipsum");
	createFile(TYPE_FILE, "C://zcu", "prvak/..///prvak/ddd.txt", "lorem ipsum");

	deleteFile("C://zcu", "prvak");
	deleteFile("C://", "zcu");
	deleteFile("C://", "zcu/ccc.txt");
	
	deleteFile("C://", "aaa.txt");
	deleteFile("C://", "bbb.txt");
	deleteFile("C://", "eee.txt");
	*/
	

	/*
	TEntryPoint shell = (TEntryPoint)GetProcAddress(User_Programs, "shell");
	if (shell) {
	CONTEXT regs;  //ted je regs jenom nejak vyplneno kvuli preakladci
	GetThreadContext(GetCurrentThread(), &regs);  //ale pak bude jeden z registru ukazovat na nejaky startup info blok
	shell(regs);
	}
	*/

	Shutdown_Kernel();
}