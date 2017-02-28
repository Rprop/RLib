/********************************************************************
	Created:	2014/06/30  20:19
	Filename: 	RLib_MemoryAllocator.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"
#include "RLib_MemoryAllocator.h"
#include "native/RLib_Native.h"
#if RLIB_DISABLE_NATIVE_API
#include <memoryapi.h>
#endif // RLIB_DISABLE_NATIVE_API

using namespace System::IO;

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN LPVOID MemoryAllocator::Allocate(HANDLE ProcessHandle, 
													  PVOID BaseAddress, SIZE_T RegionSize,
													  ULONG AllocationType, ULONG Protect)
{
#if RLIB_DISABLE_NATIVE_API
	if ((BaseAddress = VirtualAllocEx(ProcessHandle, BaseAddress, RegionSize, 
									  AllocationType, Protect)) == NULL)
#else
	if (NtAllocateVirtualMemory(ProcessHandle, &BaseAddress, NULL, &RegionSize, 
								AllocationType, Protect) != STATUS_SUCCESS)
#endif // RLIB_DISABLE_NATIVE_API
	{
		trace(!"alloc failed");
		return nullptr;
	} //if
	return BaseAddress;
}

//-------------------------------------------------------------------------

LPVOID MemoryAllocator::Free(HANDLE ProcessHandle, PVOID BaseAddress,
							 SIZE_T RegionSize, ULONG FreeType)
{
	// must be zero
	assert(FreeType != RLIB_MEM_RELEASE || RegionSize == 0);

#if RLIB_DISABLE_NATIVE_API
	// If the dwFreeType parameter is MEM_RELEASE, dwSize must be 0 (zero)
	if (VirtualFreeEx(ProcessHandle, BaseAddress, RegionSize, FreeType) == FALSE)
#else
	if (NtFreeVirtualMemory(ProcessHandle, &BaseAddress, &RegionSize, FreeType) != STATUS_SUCCESS)
#endif // RLIB_DISABLE_NATIVE_API
	{
		trace(!"free failed");
		return nullptr;
	} //if
	return BaseAddress;
}

//-------------------------------------------------------------------------

ULONG MemoryAllocator::Protect(HANDLE ProcessHandle, PVOID BaseAddress, 
							   SIZE_T RegionSize, ULONG NewAccessProtection)
{
#if RLIB_DISABLE_NATIVE_API
	if (VirtualProtectEx(ProcessHandle, BaseAddress, RegionSize, 
		NewAccessProtection, &NewAccessProtection) == FALSE)
#else
	if (NtProtectVirtualMemory(ProcessHandle, &BaseAddress, &RegionSize, 
		NewAccessProtection, &NewAccessProtection) != STATUS_SUCCESS)
#endif // RLIB_DISABLE_NATIVE_API
	{
		trace(!"protect failed");
		return NULL;
	} //if
	return NewAccessProtection;
}

//-------------------------------------------------------------------------

PMEMORY_INFORMATION MemoryAllocator::Query(HANDLE ProcessHandle, PVOID BaseAddress)
{
	static MEMORY_BASIC_INFORMATION memoryBasicInfo;
#if RLIB_DISABLE_NATIVE_API
	if (VirtualQueryEx(ProcessHandle, BaseAddress, &memoryBasicInfo, sizeof(memoryBasicInfo)) == FALSE)
#else
	if (NtQueryVirtualMemory(ProcessHandle, BaseAddress, MemoryBasicInformation, 
							 &memoryBasicInfo, sizeof(memoryBasicInfo), nullptr) != STATUS_SUCCESS)
#endif // RLIB_DISABLE_NATIVE_API
	{
		trace(!"query failed");
		memset(&memoryBasicInfo, 0, sizeof(memoryBasicInfo));
	} //if
	return reinterpret_cast<PMEMORY_INFORMATION>(&memoryBasicInfo);
}

//-------------------------------------------------------------------------

size_t MemoryAllocator::GetPageSize()
{
	return System::AppBase::GetSystemInfo()->
#if RLIB_DISABLE_NATIVE_API
		dwPageSize;
#else
		PhysicalPageSize;
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

size_t MemoryAllocator::GetPhysicalPages()
{
#if RLIB_DISABLE_NATIVE_API
	trace(!"not supported");
	return 0;
#else
	return System::AppBase::GetSystemInfo()->NumberOfPhysicalPages;
#endif // RLIB_DISABLE_NATIVE_API
}
