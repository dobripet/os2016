#pragma once

#include "..\common\api.h"

extern "C" size_t __stdcall shell(const CONTEXT &regs);


//nasledujici funkce si dejte do vlastnich souboru
extern "C" size_t __stdcall type(const CONTEXT &regs) { return 0; };
extern "C" size_t __stdcall mk(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall rd(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall echo(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall wc(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall sort(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall rgen(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall freq(const CONTEXT &regs) { return 0; }
extern "C" size_t __stdcall ps(const CONTEXT &regs) { return 0; }
