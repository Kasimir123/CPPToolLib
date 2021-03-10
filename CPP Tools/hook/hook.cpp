#include "../stdafx/stdafx.h"
#include "../mem/mem.h"
#include "hook.h"

/// <summary>
/// Basic Hook Function
/// - Make sure to return to the next bytes after function finishes
/// </summary>
/// <param name="toHook">Address to the beginning of the bytes you want to nop and jump around</param>
/// <param name="myFunct">Pointer to our function</param>
/// <param name="len">Length of the bytes to overwrite, must be at least 5 to have enough space for the jump</param>
/// <returns>true if successful</returns>
bool Detour32(void* toHook, void* myFunct, int len) {
	// Make sure we have enough length to write jump
	if (len < 5) return false;

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	// Nop all the opcodes we don't want in there
	memset(toHook, 0x90, len);

	// Get relative address to our function
	DWORD relativeAddress = ((DWORD)myFunct - (DWORD)toHook) - 5;

	// Write jump to our function
	// First opcode in jump
	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;

	DWORD temp;

	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}

/// <summary>
/// Reads bytes of signed integer in the correct endian order
/// </summary>
/// <param name="x">src to start of integer</param>
/// <returns>signed int read from bytes</returns>
signed int read_bytes(BYTE* x)
{
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		signed int u;
		unsigned char u8[sizeof(x)];
	} fin;

	int i = 0;
	for (size_t k = 0; k < sizeof(signed int); k++)
	{
		fin.u8[k] = (int)*(x + (sizeof(BYTE) * i));
		i++;
	}

	return fin.u;
}

char* TrampolineHook32(char* src, char* dst, const intptr_t len) {
	// Make sure we have enough length to write jump
	if (len < 5) return 0;

	// Creates the gateway, length of overwritten bytes plus 5 bytes for the jump back
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// Copy original bytes to the gateway
	memcpy(gateway, src, len);

	// Check if the first byte is E8, if it is then fix the relative offset for the copied bytes.
	// if this is not done then the hooked original jump will not work as it points to the wrong spot.
	if ((int)*(src) == -24)
	{
		// Reads the 4 bytes after E8 to see original offset
		signed int x = read_bytes((BYTE*)(src + sizeof(BYTE)));

		// Calculate new offset
		// src + (x+5)/4 gets the address the offset was pointing to
		// - gateway * 4) - 5 gets the new offset
		signed int newX = ((((int*)src + ((x + 5) / 4)) - (int*)gateway) * 4) - 5;

		// Copy new offset to the hooked bytes
		*(gateway + 1) = newX;
		*(gateway + 2) = (int)*((BYTE*)&newX + 1 * sizeof(BYTE));
		*(gateway + 3) = (int)*((BYTE*)&newX + 2 * sizeof(BYTE));
		*(gateway + 4) = (int)*((BYTE*)&newX + 3 * sizeof(BYTE));
	}

	// Get the destination address
	intptr_t gatewayRelAddr = ((intptr_t)src - (intptr_t)gateway) - 5;
	
	// Add the jmp opcode to the end of the gateway
	*(gateway + len) = 0xE9;

	// Add the calculated address to the jump
	*(uintptr_t*)((intptr_t)gateway + len + 1) = gatewayRelAddr;

	// Perform Hook
	Detour32(src, dst, len);

	return (char*)gateway;
}

char* TrampolineHook(char* src, char* dst, const intptr_t len)
{
	// Make sure we have enough length to write jump
	if (len < 14) return 0;

	BYTE jmp[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ptr [$+6]
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // ptr
	};

	// Creates the gateway, length of overwritten bytes plus 5 bytes for the jump back
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len + 14 + sizeof(jmp), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	DWORD64 retto = (DWORD64)src + len;

	memcpy(jmp + 6, &retto, 8);
	memcpy((void*)((DWORD_PTR)gateway), src, len);
	memcpy((void*)((DWORD_PTR)gateway + len), jmp, sizeof(jmp));

	memcpy(jmp + 6, &dst, 8);
	memcpy(src, jmp, sizeof(jmp));

	for (int i = 14; i < len; i++)
	{
		*(BYTE*)((DWORD_PTR)src + i) = 0x90;
	}

	VirtualProtect(src, len, curProtection, &curProtection);


	return (char*)((DWORD_PTR)gateway);
}

Hook::Hook(BYTE* src, BYTE* dst, BYTE* ptrToGatewayFunctionPtr, uintptr_t len)
{
	this->src = src;
	this->dst = dst;
	this->len = len;
	this->ptrToGatewayFunctionPtr = ptrToGatewayFunctionPtr;
}

Hook::Hook(const char* exportName, const char* modName, BYTE* dst, BYTE* ptrToGatewayFunctionPtr, uintptr_t len)
{
	HMODULE hMod = GetModuleHandleA(modName);

	this->src = (BYTE*)GetProcAddress(hMod, exportName);
	this->dst = dst;
	this->len = len;
	this->ptrToGatewayFunctionPtr = ptrToGatewayFunctionPtr;
}

void Hook::Enable32() 
{
	memcpy(originalBytes, src, len);
	*(uintptr_t*)ptrToGatewayFunctionPtr = (uintptr_t)TrampolineHook32((char*)src, (char*)dst, len);
	bStatus = true;
}

void Hook::Disable32()
{
	mem::Patch(src, originalBytes, len);
	bStatus = false;
}

void Hook::Enable()
{
	memcpy(originalBytes, src, len);
	*(uintptr_t*)ptrToGatewayFunctionPtr = (uintptr_t)TrampolineHook((char*)src, (char*)dst, len);
	bStatus = true;
}

void Hook::Disable()
{
	mem::Patch(src, originalBytes, len);
	bStatus = false;
}

void Hook::Toogle() 
{
	if (bStatus) Enable();
	else Disable();
}