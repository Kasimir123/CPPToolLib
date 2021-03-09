#pragma once

/// <summary>
/// Hook Class
/// </summary>
struct Hook {

	/// <summary>
	/// Whether the hook is active or not
	/// </summary>
	bool bStatus{ false };

	/// <summary>
	/// pointer to original function
	/// </summary>
	BYTE* src{ nullptr };
	
	/// <summary>
	/// pointer to our function
	/// </summary>
	BYTE* dst{ nullptr };

	/// <summary>
	/// pointer to function gateway
	/// </summary>
	BYTE* ptrToGatewayFunctionPtr{ nullptr };

	/// <summary>
	/// len of bytes to overwrite
	/// </summary>
	int len{ 0 };

	/// <summary>
	/// Array to store original bytes, currently set at 20 but can be adjusted,
	/// 20 is more than enough for most cases, 64 bit included.
	/// </summary>
	BYTE originalBytes[20]{ 0 };

	Hook() {};

	/// <summary>
	/// Hook Constructor
	/// </summary>
	/// <param name="src">pointer to original function</param>
	/// <param name="dst">pointer to our function</param>
	/// <param name="ptrToGatewayFunctionPtr">pointer to function gateway</param>
	/// <param name="len">len of bytes to overwrite</param>
	Hook(BYTE* src, BYTE* dst, BYTE* ptrToGatewayFunctionPtr, uintptr_t len);

	/// <summary>
	/// Hook an exported function
	/// </summary>
	/// <param name="exportName">Name of exported function</param>
	/// <param name="modName">Name of module to hook</param>
	/// <param name="dst">pointer to our function</param>
	/// <param name="ptrToGatewayFunctionPtr">pointer to function gateway</param>
	/// <param name="len">len of bytes to overwrite</param>
	Hook(const char* exportName, const char* modName, BYTE* dst, BYTE* ptrToGatewayFunctionPtr, uintptr_t len);

	/// <summary>
	/// Enable 64 bit hook
	/// </summary>
	void Enable();

	/// <summary>
	/// Disable 64 bit hook
	/// </summary>
	void Disable();

	/// <summary>
	/// Toggle Hook
	/// - currently only 64 bit
	/// </summary>
	void Toogle();

	/// <summary>
	/// Enable 32 bit hook
	/// </summary>
	void Enable32();

	/// <summary>
	/// Disable 32 bit hook
	/// </summary>
	void Disable32();

};

