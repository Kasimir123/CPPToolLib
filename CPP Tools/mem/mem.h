#pragma once
namespace mem
{
    /// <summary>
    /// Internal Patch Function
    /// </summary>
    /// <param name="dst">Pointer to the memory which will be overwritten</param>
    /// <param name="src">Address of the variable that will be used in patch</param>
    /// <param name="size">Size of patch</param>
    void Patch(BYTE* dst, BYTE* src, unsigned int size);

    /// <summary>
    /// Patches a section of memory
    /// </summary>
    /// <param name="dst">Pointer to the memory which will be overwritten</param>
    /// <param name="src">Address of the variable that will be used in patch</param>
    /// <param name="size">Size of patch</param>
    /// <param name="hProcess">Process Handle</param>
    void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);

    /// <summary>
    /// Reads a designated section of memory into the given variable
    /// </summary>
    /// <param name="dst">Pointer to the memory to be read</param>
    /// <param name="src">Address of the variable to be read to</param>
    /// <param name="size">Size of the variable to be read to</param>
    /// <param name="hProcess">Process Handle</param>
    void ReadEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);

    /// <summary>
    /// Internal NOP function
    /// </summary>
    /// <param name="dst">Start point for the NOP</param>
    /// <param name="size">Number of Bytes to NOP</param>
    void Nop(BYTE* dst, unsigned int size);

    /// <summary>
    /// NOP's a certain number of bytes
    /// </summary>
    /// <param name="dst">Start point for the NOP</param>
    /// <param name="size">Number of Bytes to NOP</param>
    /// <param name="hProcess">Process Handle</param>
    void NopEx(BYTE* dst, unsigned int size, HANDLE hProcess);
}
