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
	//closeSystemIO();//uvolni handly 0,1,2 pro standardni io
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


	std::cout << std::endl;
	struct node *a, *b, *c, *d;
	mkdir(&a, "zcu", getRoot());
	mkdir(&b, "zcu/prvak", getRoot());
	openFile(&c, "aaa.txt", true, true, b);
	setData(&c, 0, 10, "asdfghjklq");
	char *buffer = (char*)malloc(sizeof(char) * 11);
	size_t filled;
	getData(&c, 0, 11, &buffer, &filled);
	setData(&a, 0, filled, buffer);
	openFile(&d, "C://zcu/prvak/bbb.txt", true, true, a);


	//run shell
	command_params par;
	par.handles.push_back((FDHandle)0); 
	par.handles.push_back((FDHandle)1); 
	par.handles.push_back((FDHandle)2); 
	par.name = "shell";
//	par.current_path = (char *)getRoot()->name.c_str();
	par.handles.push_back((FDHandle)3);
//	par.waitForProcess = true; //musime na nej pockat
	int pid;
	createProcess(&par/*, &t*/, &pid);
	if (pid == - 1) {
		std::cout << " nenalezen";
		//oznamit error , ze nesel spustit shell
	}
	else {
		joinProcess(pid);
	}

	Shutdown_Kernel();
}