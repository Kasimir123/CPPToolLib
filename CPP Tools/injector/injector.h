#pragma once

namespace injector 
{
	/// <summary>
	/// Injects a dll
	/// </summary>
	/// <param name="dllPath"></param>
	/// <param name="hProc"></param>
	void LocAInject(const char* dllPath, HANDLE hProc);
}

