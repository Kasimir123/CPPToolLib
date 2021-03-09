#pragma once
namespace sigscan 
{
    /// <summary>
    /// 
    /// </summary>
    /// <param name="base"></param>
    /// <param name="size"></param>
    /// <param name="pattern"></param>
    /// <param name="mask"></param>
    /// <returns></returns>
    void * PatternScan(char* base, size_t size, char* pattern, char* mask);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="hProcess"></param>
    /// <param name="begin"></param>
    /// <param name="end"></param>
    /// <param name="pattern"></param>
    /// <param name="mask"></param>
    /// <returns></returns>
    void * PatternScanEx(HANDLE hProcess, uintptr_t begin, uintptr_t end, char* pattern, char*  mask);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="hProcess"></param>
    /// <param name="exeName"></param>
    /// <param name="module"></param>
    /// <param name="pattern"></param>
    /// <param name="mask"></param>
    /// <returns></returns>
    void * PatternScanExModule(HANDLE hProcess, wchar_t * exeName, wchar_t* module, char* pattern, char* mask);

    /// <summary>
    /// Scans the process internally
    /// </summary>
    /// <param name="begin">start of the memory you want to scan</param>
    /// <param name="size">size of memory you want to scan</param>
    /// <param name="pattern">the pattern you want to scan for</param>
    /// <param name="mask">the mask for the pattern</param>
    /// <returns>returns address if found, null pointer if not found</returns>
    char * PatternScanInternal(char* begin, intptr_t size, char* pattern, char* mask);

    /// <summary>
    /// Scans internally but scans as much memory as it can
    /// </summary>
    /// <param name="begin">start of the memory you want to scan</param>
    /// <param name="pattern">the pattern you want to scan for</param>
    /// <param name="mask">the mask for the pattern</param>
    /// <returns>returns address if found, null pointer if not found</returns>
    char * PatternScanInternalFull(char* begin, char* pattern, char* mask);
}