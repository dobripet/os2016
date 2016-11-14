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
	par.handles.push_back((FDHandle)0); //realnej soubor pokud bude presmerovani vstupu do naseho programu?
	par.handles.push_back((FDHandle)1); //realnej soubor pokud bude presmerovani vystup z naseho programu?
	par.handles.push_back((FDHandle)2); //realnej soubor pokud bude presmerovani err z naseho programu?

	std::cout << std::endl;
	struct node *a = openFile(TYPE_DIRECTORY, "zcu", true, getCecko());
	struct node *b = openFile(TYPE_DIRECTORY, "zcu/prvak", true, getCecko());
	struct node *c = openFile(TYPE_FILE, "aaa.txt", true, b);
	setData(&c, 0, 10, "asdfghjklq");
	char *buffer = (char*)malloc(sizeof(char) * 10);
	getData(&c, 0, 10, &buffer);
	setData(&a, 0, 10, buffer);
	//struct node *d = openFile(TYPE_DIRECTORY, "C://zcu/prvak/bbb.txt", true, a);

	par.name = "shell";
	par.current_path = (char *)getCecko()->name.c_str();
	par.handles.push_back((FDHandle)3);
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
	struct node *newFile = openFile(0, "zcu/prvak", true, getCecko());
	
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