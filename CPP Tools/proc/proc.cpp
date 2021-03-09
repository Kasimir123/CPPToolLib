#include "../stdafx/stdafx.h"
#include "../sigscan/sigscan.h"
#include "../mem/mem.h"
#include "proc.h"

DWORD proc::GetProcId(const wchar_t* procName)
{
    // Assign to 0 for error handling
    DWORD procId = 0;
    // Takes snapshot of the processes
    HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    // Check if snapshot exists and didn't error out
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        // Set entry size
        procEntry.dwSize = sizeof(procEntry);
        // Grabs first process in the snapshot and stores in procEntry
        if (Process32First(hSnap, &procEntry)) {
            // Loops through all processes
            do
            {
                // Checks if the process name is our process name
                if (!_wcsicmp(procEntry.szExeFile, procName)) {
                    // When found it saves the id and breaks out of the loop
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    // Closes Handle
    CloseHandle(hSnap);
    // Returns process id
    return procId;
}

DWORD proc::GetProcIdByContains(const wchar_t* procName)
{
    // Assign to 0 for error handling
    DWORD procId = 0;
    // Takes snapshot of the processes
    HANDLE hSnap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    // Check if snapshot exists and didn't error out
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        // Set entry size
        procEntry.dwSize = sizeof(procEntry);
        // Grabs first process in the snapshot and stores in procEntry
        if (Process32First(hSnap, &procEntry)) {
            // Loops through all processes
            do
            {
                // Checks if the process name contains our process name
                if (wcsstr(procEntry.szExeFile, procName)) {
                    // When found it saves the id and breaks out of the loop
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    // Closes Handle
    CloseHandle(hSnap);
    // Returns process id
    return procId;
}

uintptr_t proc::GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    // Assign to 0 for error handling
    uintptr_t modBaseAddr = 0;
    // Takes snapshot of the processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    // Check if snapshot exists and didn't error out
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        // Set entry size
        modEntry.dwSize = sizeof(modEntry);
        // Grabs first module in the snapshot and stores in modEntry
        if (Module32First(hSnap, &modEntry)) {
            do
            {
                // Checks if the module name is our module name
                if (!_wcsicmp(modEntry.szModule, modName)) {
                    // When found it saves the address and breaks out of the loop
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    // Closes Handle
    CloseHandle(hSnap);
    // Returns module address
    return modBaseAddr;
}

MODULEENTRY32 proc::GetModule(DWORD procId, wchar_t * modName)
{
    // Assign to 0 for error handling
    MODULEENTRY32 modEntry = { 0 };
    // Takes snapshot of the processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    // Check if snapshot exists and didn't error out
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 curr = { 0 };
        // Set entry size
        curr.dwSize = sizeof(MODULEENTRY32);
        // Grabs first module in the snapshot and stores in curr
        if (Module32First(hSnap, &curr))
        {
            do
            {
                // Checks if the module name is the module name
                if (!wcscmp(curr.szModule, modName))
                {
                    // When found it saves the address and breaks out of the loop
                    modEntry = curr;
                    break;
                }
            } while (Module32Next(hSnap, &curr));
        }
        // Closes Handle
        CloseHandle(hSnap);
    }
    return modEntry;
}

// Dynamic Memory Allocation Address
uintptr_t proc::FindDMAAddyEx(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    // Loops through offsets to get to address of the actual value
    for (unsigned int i = 0; i < offsets.size(); ++i) {
        ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

uintptr_t proc::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    // Loops through offsets to get to address of the actual value
    for (unsigned int i = 0; i < offsets.size(); ++i) {
        addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}

uintptr_t proc::getBaseOffset(HANDLE hProcess, wchar_t* processName, char* signature, char* signatureMask, int readStartOffset)
{
    // Gets the base offset address from the signature
    void* baseOffsetAddress = sigscan::PatternScanExModule(hProcess, processName, processName, signature, signatureMask);

    // If the signature isn't nothing
    if (baseOffsetAddress != nullptr) {
        // Dword to read base offset into
        DWORD read;
        // Add the read start offset (e.g. if the read starts X amount in a signature) 
        baseOffsetAddress = (void*)((uintptr_t)baseOffsetAddress + (sizeof(char) * readStartOffset));
        // Read the base offset
        mem::ReadEx((BYTE*)baseOffsetAddress, (BYTE*)&read, sizeof(read), hProcess);
        // Return Base Offset
        return read;
    }

    // Return 0 if the ptr is nothing
    return (uintptr_t)0;
}
