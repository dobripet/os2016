#pragma once

#include "kernel.h"
#include "io.h"
#include "process.h"
#include "filesystem.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <io.h>
#include "..\common\api.h"

HMODULE User_Programs;

void Set_Error(const bool failed, CONTEXT &regs) {
	if (failed) {
		regs.EFlags = stc(regs.EFlags);
		regs.Rax = GetLastError();
	}
	else
		regs.EFlags = clc(regs.EFlags);
}

void Initialize_Kernel() {
	User_Programs = LoadLibrary(L"user.dll");	
	initSystemIO();//vytvori handly 0,1,2 pro standardni io
}

void Shutdown_Kernel() {
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

	/*
	//testovaci slozky a soubory
	struct node *a, *b, *c, *d;
	mkdir(&a, "zcu", getRoot());
	mkdir(&b, "zcu/prvak", getRoot());
	openFile(&c, "aaa", true, true, b);
	mkdir(&b, "zcu/druhak", getRoot());
	setData(&c, "nejaky text\0", strlen("nejaky text\0"));
	char *buffer = (char*)malloc(sizeof(char) * (strlen("nejaky text\0") + 1));
	buffer[strlen("nejaky text\0")] = '\0';
	size_t filled;
	getData(&c, 0, strlen("nejaky text\0"), &buffer, &filled);
	openFile(&d, "C://zcu/prvak/bbb", true, true, a);
	setData(&d, buffer, filled);
	*/

	//run shell
	command_params par;
	par.handles.push_back((FDHandle)0); 
	par.handles.push_back((FDHandle)1); 
	par.handles.push_back((FDHandle)2); 
	par.name = "shell";
	par.handles.push_back((FDHandle)3);
	if (_isatty(_fileno(stdin))) {
		par.stdinIsConsole = true;
	} else {
		par.stdinIsConsole = false;
	}
	int pid;
	std::cout << "Boot successful." << std::endl;
	if (createProcess(&par, &pid) == S_FALSE) {
		std::cerr << "It was not possible to start shell. Shutting down..." << std::endl;
	}
	else {
		joinProcess(pid);
	}
	
	Shutdown_Kernel();
}