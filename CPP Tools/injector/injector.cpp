#include "../stdafx/stdafx.h"
#include "../mem/mem.h"
#include "../errorhandling/ErrorHandling.h"
#include "injector.h"

void injector::LocAInject(const char* dllPath, HANDLE hProc)
{
	LPVOID libAAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	mem::PatchEx((BYTE*)loc, (BYTE*)dllPath, (unsigned int)(strlen(dllPath) + 1), hProc);

	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)libAAddr, loc, 0, 0);

	if (hThread != NULL) {
		CloseHandle(hThread);
	}
	else {
		ErrorHandling::ErrorExit((LPTSTR)(L"CreateRemoteThread"));
	}
}


// Gets the size of the section
size_t GetSectionSize(PIMAGE_NT_HEADERS headers, PIMAGE_SECTION_HEADER section) {
	// Set the size to the size of the raw data
	DWORD size = section->SizeOfRawData;

	// If the section has no raw data then set the size to the size of the data
	if (size == 0) {
		if (section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
			size = headers->OptionalHeader.SizeOfInitializedData;
		}
		else if (section->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
			size = headers->OptionalHeader.SizeOfUninitializedData;
		}
	}

	// Return the size
	return (size_t)size;
}

#define GetPointerOffset(data, offset) (void*)((uintptr_t)data + offset)

// LoadLibrary function 
typedef HMODULE(WINAPI tLoadLibrary)(LPCSTR);

// GetProcAddress function 
typedef FARPROC(WINAPI tGetProcAddress)(HMODULE, LPCSTR);

// Manually mapped data struct
typedef struct {
	unsigned char* code;
	tLoadLibrary* loadLibrary;
	tGetProcAddress* getProcAddress;
} MAN_MAP_DATA;

// Shellcode we inject into the function
HINSTANCE __stdcall ManMap(MAN_MAP_DATA * data)
{

	// Get dos header
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(data->code);

	// Get the pointer to the allocated headers
	PIMAGE_NT_HEADERS allocatedHeaders = (PIMAGE_NT_HEADERS)(data->code + dosHeader->e_lfanew);

	// Get the entry import directory
	PIMAGE_DATA_DIRECTORY directory = &(allocatedHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);

	// Get the first import descriptor
	PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(data->code + directory->VirtualAddress);

	auto loadLibrary = data->loadLibrary;
	auto getProcAddress = data->getProcAddress;

	// While the descriptor has a name keep looping through them all
	while (importDesc->Name)
	{
		// Declare our two reference pointers
		uintptr_t* thunkRef;
		FARPROC* funcRef;

		// Get the handle to the import (using LoadLibrary since we don't want to have to resolve all of the relative paths)
		HMODULE handle = loadLibrary((LPCSTR)(data->code + importDesc->Name));


		// If original first think then use that virtual address, otherwise use first thunk address
		if (importDesc->OriginalFirstThunk)
			thunkRef = (uintptr_t*)(data->code + importDesc->OriginalFirstThunk);
		else
			thunkRef = (uintptr_t*)(data->code + importDesc->FirstThunk);

		// Get function reference
		funcRef = (FARPROC*)(data->code + importDesc->FirstThunk);

		// Loop through all thunks and set the value of the function reference
		for (; *thunkRef; thunkRef++, funcRef++)
		{
			if (IMAGE_SNAP_BY_ORDINAL(*thunkRef))
			{
				*funcRef = getProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(data->code + (*thunkRef));
				*funcRef = getProcAddress(handle, (LPCSTR)&thunkData->Name);
			}
		}

		// increment the descriptor
		importDesc++;
	}

	 //If we have a TLS directory
	if (allocatedHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		// Get the directory
		IMAGE_TLS_DIRECTORY* tlsDirectory = (IMAGE_TLS_DIRECTORY*)(data->code + allocatedHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

		// Get the first callback
		PIMAGE_TLS_CALLBACK* pCallback = (PIMAGE_TLS_CALLBACK*)(tlsDirectory->AddressOfCallBacks);

		// Loop through all callbacks
		for (; pCallback && (*pCallback); ++pCallback)
		{
			// Attach the dll
			PIMAGE_TLS_CALLBACK Callback = *pCallback;
			Callback(data->code, DLL_PROCESS_ATTACH, nullptr);
		}
	}

	// ****************************************************************************************************************************************************

	// typedef the DLL entry function
	typedef bool(WINAPI* DllEntryProc)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

	// Get the address for the entry 
	DllEntryProc DllEntry = (DllEntryProc)(LPVOID)(data->code + allocatedHeaders->OptionalHeader.AddressOfEntryPoint);

	// Call the entrypoint
	(*DllEntry)((HINSTANCE)data->code, DLL_PROCESS_ATTACH, 0);

	// ****************************************************************************************************************************************************

	// Return the base address of our loaded DLL
	return (HINSTANCE)data->code;

}

// End position of shellcode, used to dynamically determine size of the function
DWORD ManMapEnd() { return 1; }


void injector::ManMapInject(const char* dllPath, HANDLE hProc)
{

	// Open file in binary mode
	std::ifstream File(dllPath, std::ios::binary | std::ios::ate);

	// Get the size of the program
	size_t fileSize = File.tellg();

	// Allocate space for the data
	BYTE* data = new BYTE[fileSize];

	// Reset position to the start of the file
	File.seekg(0, std::ios::beg);

	// Read all of the file into our data
	File.read((char*)data, fileSize);

	// Close the handle to the file
	File.close();

	// Get dos header
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(data);

	// Get nt headers
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(data + dosHeader->e_lfanew);

	// ****************************************************************************************************************************************************

	// Declare the variable where we will store the system information
	SYSTEM_INFO sysInfo;

	// Get the native system information
	GetNativeSystemInfo(&sysInfo);

	// Get the image size aligned to the next largest multiple of the page size
	size_t imageSize = (ntHeaders->OptionalHeader.SizeOfImage + sysInfo.dwPageSize - 1) & ~(sysInfo.dwPageSize - 1);

	// Attempt to allocate memory at the image base
	unsigned char* code = (unsigned char*)VirtualAllocEx(hProc, (LPVOID)(ntHeaders->OptionalHeader.ImageBase), imageSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// If the result was null then let the system decide where to allocate
	if (code == NULL)
		code = (unsigned char*)VirtualAllocEx(hProc, NULL, imageSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// Allocate space in the process for the data
	unsigned char* localCode = (unsigned char*)VirtualAlloc(NULL, imageSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// Copy the headers from the data into the allocated memory
	memcpy(localCode, data, ntHeaders->OptionalHeader.SizeOfHeaders);

	// Get the pointer to the allocated headers
	PIMAGE_NT_HEADERS allocatedHeaders = (PIMAGE_NT_HEADERS)(localCode + dosHeader->e_lfanew);

	// ****************************************************************************************************************************************************

	// Get first section header
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(allocatedHeaders);

	// Loop through all sections
	for (int i = 0; i < allocatedHeaders->FileHeader.NumberOfSections; i++, section++)
	{
		// Get the physical address of the section in memory
		unsigned char* segmentAddress = localCode + section->VirtualAddress;

		// If the size of the data is 0 then we are "allocating" memory for a data section
		if (!(section->SizeOfRawData))
		{
			// Check if the SectionAlignment is greater than 0, semi redundent check but better safe than sorry
			if (ntHeaders->OptionalHeader.SectionAlignment > 0)
				// Set the SectionAlignment number of bytes to 0
				memset(segmentAddress, 0, ntHeaders->OptionalHeader.SectionAlignment);
		}
		// If not a data section then we need to copy all of the data over to it
		else
			// Copy the raw data into memory
			memcpy(segmentAddress, data + section->PointerToRawData, section->SizeOfRawData);

		// Update the address in the section header to point to our loaded section
		section->Misc.PhysicalAddress = (DWORD)((uintptr_t)segmentAddress & 0xffffffff);
	}

	// Get the first section
	section = IMAGE_FIRST_SECTION(allocatedHeaders);

	// Get the physical address of the first section
	LPVOID gAddress = (LPVOID)section->Misc.PhysicalAddress;

	// Get the address of the first section aligned with the page size
	LPVOID gAlignedAddress = (LPVOID)((uintptr_t)gAddress & ~(sysInfo.dwPageSize - 1));

	// Get the section size
	size_t gSize = GetSectionSize(allocatedHeaders, section);

	// Get the section characteristics 
	DWORD gCharacteristics = section->Characteristics;

	// Set isLast to false
	bool isLast = false;

	// Loop through all sections
	for (int i = 0; i < allocatedHeaders->FileHeader.NumberOfSections; i++, section++) {

		// Get all of the information for the current section
		LPVOID sectionAddress = (LPVOID)((uintptr_t)section->Misc.PhysicalAddress);
		LPVOID alignedAddress = (LPVOID)((uintptr_t)sectionAddress & ~(sysInfo.dwPageSize - 1));
		SIZE_T sectionSize = GetSectionSize(allocatedHeaders, section);

		// Check if the current section is on the same page as the previous section
		if (gAlignedAddress == alignedAddress || (uintptr_t)gAddress + gSize > (uintptr_t) alignedAddress) {
			// If it is then update the characteristics with those of the current section
			if ((section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) == 0 || (gCharacteristics & IMAGE_SCN_MEM_DISCARDABLE) == 0) {
				gCharacteristics = (gCharacteristics | section->Characteristics) & ~IMAGE_SCN_MEM_DISCARDABLE;
			}
			else {
				gCharacteristics |= section->Characteristics;
			}

			// Get the size from the start of the first section on the page up till the end of the most recent section
			gSize = (((uintptr_t)sectionAddress) + ((uintptr_t)sectionSize)) - (uintptr_t)gAddress;

			// Skip the rest of the loop
			continue;
		}

		// If the section can be discarded then free the memory
		if (gCharacteristics & IMAGE_SCN_MEM_DISCARDABLE) {
			// Check that a whole page is getting freed
			if (gAddress == gAlignedAddress &&
				(isLast ||
					allocatedHeaders->OptionalHeader.SectionAlignment == sysInfo.dwPageSize ||
					(gSize % sysInfo.dwPageSize) == 0)
				) {
				VirtualFreeEx(hProc, gAddress, gSize, MEM_DECOMMIT);
			}
		}

		// Check if the section is executable and or readable, we ignore writeable since we are going to need
		// the sections to be writable when we do the imports once the dll is injected. The imports need to be 
		// done in the injected process while we want to do this in our injector so we need to make some exceptions.
		bool executable = (gCharacteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		bool readable = (gCharacteristics & IMAGE_SCN_MEM_READ) != 0;

		// Array of allowed protections
		int ProtectionFlags[2][2] = { {PAGE_WRITECOPY, PAGE_READWRITE, }, {PAGE_EXECUTE_WRITECOPY, PAGE_EXECUTE_READWRITE,} };

		// Get the protection flag to use
		DWORD protection = ProtectionFlags[executable][readable];

		// check if we need to add the no cache flag to the protection
		if (gCharacteristics & IMAGE_SCN_MEM_NOT_CACHED) {
			protection |= PAGE_NOCACHE;
		}

		// Declare old protection
		DWORD oldProtection;

		// Change the protection of the section
		VirtualProtectEx(hProc, gAddress, gSize, protection, &oldProtection);

		// Set the new values
		gAddress = sectionAddress;
		gAlignedAddress = alignedAddress;
		gSize = sectionSize;
		gCharacteristics = section->Characteristics;

		// If this is the last section then set isLast to true
		if (i == allocatedHeaders->FileHeader.NumberOfSections - 1)
			isLast = true;
	}

	// ****************************************************************************************************************************************************

	// Get the difference between the two ImageBases
	ptrdiff_t ptrDiff = (ptrdiff_t)(code - ntHeaders->OptionalHeader.ImageBase);

	// Check if we need to perform any relocations
	if (ptrDiff)
	{
		// Get data directory
		PIMAGE_DATA_DIRECTORY directory = &(allocatedHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);

		// Get first base relocation
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(localCode + directory->VirtualAddress);

		// Loop through all relocations, check virtual address to see when to quit
		while (relocation->VirtualAddress > 0)
		{
			// Get the physical address of the relocation
			unsigned char* physicalAddress = localCode + relocation->VirtualAddress;

			// Get the first relocation information for the relocation
			unsigned short* relInfo = (unsigned short*)GetPointerOffset(relocation, sizeof(IMAGE_BASE_RELOCATION));

			// Loop through all relocation info
			for (int i = 0; i < ((relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2); i++, relInfo++)
			{
				// Get the type of relocation
				int type = *relInfo >> 12;

				// Get the relocation offset
				int offset = *relInfo & 0xfff;

				// If the type is IMAGE_REL_BASED_HIGHLOW then perform our patch
				if (type == IMAGE_REL_BASED_HIGHLOW)
				{
					// Get the reference
					DWORD* patch = (DWORD*)(physicalAddress + offset);

					// Adjust the reference based on the difference between the ImageBases
					*patch += (DWORD)ptrDiff;
				}
			}

			// Get the next relocation
			relocation = (PIMAGE_BASE_RELOCATION)GetPointerOffset(relocation, relocation->SizeOfBlock);
		}
	}

	// ****************************************************************************************************************************************************

	// Copy the local code to the other process
	WriteProcessMemory(hProc, code, localCode, imageSize, NULL);

	// Initialize manually mapped data struct
	MAN_MAP_DATA* manMapData = new MAN_MAP_DATA;

	// Set the pointer of the code
	manMapData->code = code;

	// Get the pointer to load library
	manMapData->loadLibrary = (tLoadLibrary*)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	// Get the pointer to get proc address
	manMapData->getProcAddress = (tGetProcAddress*)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetProcAddress");

	// Allocate space for the manually mapped data
	LPVOID man_map_data = VirtualAllocEx(hProc, NULL, sizeof(MAN_MAP_DATA), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// Allocate space for the shellcode
	BYTE* man_map = (BYTE*)VirtualAllocEx(hProc, NULL, (DWORD)((ULONG_PTR)ManMapEnd - (ULONG_PTR)ManMap), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// Copy the data to the other function
	WriteProcessMemory(hProc, man_map_data, manMapData, sizeof(MAN_MAP_DATA), NULL);

	// Copy the shellcode to the other function
	WriteProcessMemory(hProc, man_map, ManMap, (DWORD)((ULONG_PTR)ManMapEnd - (ULONG_PTR)ManMap), NULL);

	// Create the thread in the other process
	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)man_map, man_map_data, 0, 0);

	if (hThread != NULL) {
		CloseHandle(hThread);
	}
	else {
		ErrorHandling::ErrorExit((LPTSTR)(L"CreateRemoteThread"));
	}

}
