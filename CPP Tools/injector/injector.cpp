#include "../stdafx/stdafx.h"
#include "../mem/mem.h"
#include "../errorhandling/ErrorHandling.h"
#include "injector.h"


void injector::LocAInject(const char* dllPath, HANDLE hProc)
{
	LPVOID libAAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	mem::PatchEx((BYTE *)loc, (BYTE*)dllPath, (unsigned int)(strlen(dllPath) + 1), hProc);

	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)libAAddr, loc, 0, 0);

	if (hThread != NULL) {
		CloseHandle(hThread);
	}
	else {
        ErrorHandling::ErrorExit((LPTSTR)(L"CreateRemoteThread"));
	}
}