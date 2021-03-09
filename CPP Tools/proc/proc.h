#pragma once

namespace proc 
{
    /// <summary>
    /// Gets the process id
    /// </summary>
    /// <param name="procName">The Process Name</param>
    /// <returns>Returns the process id</returns>
    DWORD GetProcId(const wchar_t* procName);

    /// <summary>
    /// Gets the process id for a process that contains the given string
    /// </summary>
    /// <param name="procName">string in process name</param>
    /// <returns>Returns the process id</returns>
    DWORD GetProcIdByContains(const wchar_t* procName);
    
    /// <summary>
    /// Gets the module base address
    /// </summary>
    /// <param name="procId">Process ID</param>
    /// <param name="modName">The name of the module</param>
    /// <returns>Returns the module base address</returns>
    uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

    /// <summary>
    /// Gets the Module
    /// </summary>
    /// <param name="procId">Process ID</param>
    /// <param name="modName">The name of the module</param>
    /// <returns>Returns the module</returns>
    MODULEENTRY32 GetModule(DWORD procId, wchar_t * modName);

    /// <summary>
    /// Iterates through offsets to find an address
    /// </summary>
    /// <param name="ptr">Original address in memory</param>
    /// <param name="offsets">Offsets to be added to ptr</param>
    /// <returns>Returns new address with the offsets added</returns>
    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);

    /// <summary>
    /// Iterates through offsets to find an address
    /// </summary>
    /// <param name="hProc">Process Handle</param>
    /// <param name="ptr">Original address in memory</param>
    /// <param name="offsets">Offsets to be added to ptr</param>
    /// <returns>Returns new address with the offsets added</returns>
    uintptr_t FindDMAAddyEx(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);

    /// <summary>
    /// Gets the base offset of a process when passed the signature
    /// </summary>
    /// <param name="hProcess">Process Handle</param>
    /// <param name="processName">Process Name</param>
    /// <param name="signature">Signature to find the base offset</param>
    /// <param name="signatureMask">Signature Mask</param>
    /// <param name="readStartOffset">Offset for start read</param>
    /// <returns>Returns the base offset or 0 if nothing</returns>
    uintptr_t getBaseOffset(HANDLE hProcess, wchar_t* processName, char* signature, char* signatureMask, int readStartOffset);

}
