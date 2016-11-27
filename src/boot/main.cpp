#include <Windows.h>

/* takhle jsme kod mainu dostali, takze jsme to nememenili - nepokouseli se warningy odstranit, pouze je vypnuli */
// C4191 = 'type cast': unsafe conversion from 'FARPROC' to 'TRun_VM' 
// C4007 = 'main': must be '__cdecl'
#pragma warning ( push )
#pragma warning( disable : 4191 4007 )


typedef void(__stdcall *TRun_VM)();

int main() {
	HMODULE kernel = LoadLibrary(L"kernel.dll");
	if (!kernel) return 1;
	TRun_VM vm = (TRun_VM)GetProcAddress(kernel, "Run_VM");
	if (vm) vm();
	FreeLibrary(kernel);
	return 0;
}
#pragma warning ( pop )