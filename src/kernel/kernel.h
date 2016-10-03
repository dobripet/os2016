#pragma once

#include "..\common\api.h"

extern HMODULE User_Programs;

void Set_Error(const bool failed, CONTEXT &regs);

extern "C" void __stdcall SysCall(CONTEXT &regs);
extern "C" void __stdcall Run_VM();



