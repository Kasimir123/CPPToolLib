#include "../stdafx/stdafx.h"
#include "sigscan.h"
#include "../mem/mem.h"
#include "../proc/proc.h"

//Pattern Scan
void * sigscan::PatternScan(char* base, size_t size, char* pattern, char* mask)
{
    size_t patternLength = strlen(mask);

    for (unsigned int i = 0; i < size - patternLength; i++)
    {
        bool found = true;
        for (unsigned int j = 0; j < patternLength; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(base + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (void*)(base + i);
        }
    }
    return nullptr;
}

//External Wrapper
void * sigscan::PatternScanEx(HANDLE hProcess, uintptr_t begin, uintptr_t end, char* pattern, char*  mask)
{
    uintptr_t currentChunk = begin;
    SIZE_T bytesRead;

    while (currentChunk < end)
    {
        char buffer[4096];

        DWORD oldprotect;
        VirtualProtectEx(hProcess, (void*)currentChunk, sizeof(buffer), PAGE_EXECUTE_READWRITE, &oldprotect);
        ReadProcessMemory(hProcess, (void*)currentChunk, &buffer, sizeof(buffer), &bytesRead);
        VirtualProtectEx(hProcess, (void*)currentChunk, sizeof(buffer), oldprotect, &oldprotect);

        if (bytesRead == 0)
        {
            return nullptr;
        }

        void* internalAddress = PatternScan((char*)&buffer, bytesRead, pattern, mask);

        if (internalAddress != nullptr)
        {
            //calculate from internal to external
            uintptr_t offsetFromBuffer = (uintptr_t)internalAddress - (uintptr_t)&buffer;
            return (void*)(currentChunk + offsetFromBuffer);
        }
        else
        {
            //advance to next chunk
            currentChunk = currentChunk + bytesRead;
        }
    }
    return nullptr;
}

//Module wrapper for external pattern scan
void * sigscan::PatternScanExModule(HANDLE hProcess, wchar_t * exeName, wchar_t* module, char* pattern, char* mask)
{
    DWORD processID = proc::GetProcId((const wchar_t*)exeName);
    MODULEENTRY32 modEntry = proc::GetModule(processID, module);

    if (!modEntry.th32ModuleID)
    {
        return nullptr;
    }

    uintptr_t begin = (uintptr_t)modEntry.modBaseAddr;
    uintptr_t end = begin + modEntry.modBaseSize;
    return PatternScanEx(hProcess, begin, end, pattern, mask);
}

char* sigscan::PatternScanInternal(char* begin, intptr_t size, char* pattern, char* mask)
{
    char* match{ nullptr };
    MEMORY_BASIC_INFORMATION mbi{};

    for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
    {
            if (!VirtualQuery(curr, &mbi, sizeof(mbi)) || !(mbi.State & MEM_COMMIT) || mbi.Protect == PAGE_NOACCESS) continue;

        __try {
            match = (char*)sigscan::PatternScan(curr, mbi.RegionSize, pattern, mask);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // Continue if memory was freed or there is a page fault
        }

        if (match != nullptr)
        {
            break;
        }
    }
    return match;
}

char* sigscan::PatternScanInternalFull(char* begin, char* pattern, char* mask) 
{
    return sigscan::PatternScanInternal(begin, (DWORD)0x7FFFFFFF, pattern, mask);
}