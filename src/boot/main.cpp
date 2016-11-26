#include <Windows.h>

//C4191: 'type cast': unsafe conversion from 'FARPROC' to 'TRun_VM' 
#pragma warning( disable : 4191 )

typedef void(__stdcall *TRun_VM)();

int main() {

	HMODULE kernel = LoadLibrary(L"kernel.dll");
	if (!kernel) {
		return 1;
	}
	TRun_VM vm = (TRun_VM)GetProcAddress(kernel, "Run_VM");
	if (vm) vm();
	FreeLibrary(kernel);

	return 0;
}