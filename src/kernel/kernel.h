#pragma once

#include "..\common\api.h"

extern HMODULE User_Programs;

void Set_Error(const bool failed, CONTEXT &regs);

extern "C" void __stdcall SysCall(CONTEXT &regs);
extern "C" void __stdcall Run_VM();

typedef int RESULT;
const RESULT R_OK = 0;
const RESULT R_ERR = 1;
const bool R_FAILED(const RESULT R) {
	return R != R_OK;
}


