#pragma once

#include "kernel.h"
#include "io.h"
#include "filesystem.h"
#include <iostream>
#include <string>
#include <vector>

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
}

void Shutdown_Kernel() {
	FreeLibrary(User_Programs);
}

void SysCall(CONTEXT &regs) {

	switch (Get_AH((__int16) regs.Rax)) {
		case scIO:		HandleIO(regs);
	}

}

void Run_VM() {
	Initialize_Kernel();

	//spustime shell - v realnem OS bychom ovsem spousteli login
	TEntryPoint shell = (TEntryPoint)GetProcAddress(User_Programs, "shell");
	if (shell) {
		CONTEXT regs;  //ted je regs jenom nejak vyplneno kvuli preakladci
		GetThreadContext(GetCurrentThread(), &regs);  //ale pak bude jeden z registru ukazovat na nejaky startup info blok
		shell(regs);
	}

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
	
	std::cin.get();
	Shutdown_Kernel();
}