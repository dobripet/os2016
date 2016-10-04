#include <Windows.h>
#include <iostream>

typedef void(__stdcall *TRun_VM)();

int main() {

	std::cout << "boot test";
	
	HMODULE kernel = LoadLibrary(L"kernel.dll");
	TRun_VM vm = (TRun_VM)GetProcAddress(kernel, "Run_VM");
	if (vm) vm();
	FreeLibrary(kernel);
	return 0;
}