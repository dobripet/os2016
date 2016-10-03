#include <Windows.h>
#include <stdio.h>

typedef void(__stdcall *TRun_VM)();

int main() {
	printf("boot test\n");
	HMODULE kernel = LoadLibrary(L"kernel.dll");
	TRun_VM vm = (TRun_VM)GetProcAddress(kernel, "Run_VM");
	if (vm) vm();
	FreeLibrary(kernel);
	return 0;
}