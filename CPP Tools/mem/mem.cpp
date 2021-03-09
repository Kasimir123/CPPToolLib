#include "../stdafx/stdafx.h"
#include "mem.h"

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);

    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

void mem::PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess)
{
    DWORD oldProtect;
    VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    WriteProcessMemory(hProcess, dst, src, size, nullptr);
    VirtualProtectEx(hProcess, dst, size, oldProtect, &oldProtect);
}

void mem::ReadEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess) 
{
    DWORD oldProtect;
    VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    ReadProcessMemory(hProcess, dst, src, size, nullptr);
    VirtualProtectEx(hProcess, dst, size, oldProtect, &oldProtect);
}

void mem::Nop(BYTE* dst, unsigned int size)
{
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memset(dst, 0x90, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}


void mem::NopEx(BYTE* dst, unsigned int size, HANDLE hProcess)
{
    BYTE* nopArray = new BYTE[size];
    // 0x90 is the NOP code
    memset(nopArray, 0x90, size);

    PatchEx(dst, nopArray, size, hProcess);
    delete[] nopArray;
}