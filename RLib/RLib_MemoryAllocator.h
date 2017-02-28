/********************************************************************
	Created:	2014/06/30  20:07
	Filename: 	RLib_MemoryAllocator.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib.h"
#ifndef _USE_MEMORY_ALLOCATOR
#define _USE_MEMORY_ALLOCATOR
// AccessProtection
#define RLIB_PAGE_NOACCESS          0x01     
#define RLIB_PAGE_READONLY          0x02     
#define RLIB_PAGE_READWRITE         0x04     
#define RLIB_PAGE_WRITECOPY         0x08     
#define RLIB_PAGE_EXECUTE           0x10     
#define RLIB_PAGE_EXECUTE_READ      0x20     
#define RLIB_PAGE_EXECUTE_READWRITE 0x40     
#define RLIB_PAGE_EXECUTE_WRITECOPY 0x80     
#define RLIB_PAGE_GUARD            0x100     
#define RLIB_PAGE_NOCACHE          0x200     
#define RLIB_PAGE_WRITECOMBINE     0x400     
// AllocationType
#define RLIB_MEM_COMMIT           0x1000     
#define RLIB_MEM_RESERVE          0x2000     
#define RLIB_MEM_DECOMMIT         0x4000     
#define RLIB_MEM_RELEASE          0x8000     
#define RLIB_MEM_FREE            0x10000     
#define RLIB_MEM_PRIVATE         0x20000     
#define RLIB_MEM_MAPPED          0x40000     
#define RLIB_MEM_RESET           0x80000     
#define RLIB_MEM_TOP_DOWN       0x100000     
#define RLIB_MEM_WRITE_WATCH    0x200000     
#define RLIB_MEM_PHYSICAL       0x400000     
#define RLIB_MEM_ROTATE         0x800000     
#define RLIB_MEM_LARGE_PAGES  0x20000000     
#define RLIB_MEM_4MB_PAGES    0x80000000     
#define RLIB_MEM_IMAGE        0x1000000     
#define RLIB_VALIDBYTE        0xCCU

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		class RLIB_API RLIB_THREAD_SAFE MemoryAllocator
		{
		public:
			static RLIB_RESTRICT_RETURN LPVOID Allocate(HANDLE ProcessHandle, 
														PVOID BaseAddress, SIZE_T RegionSize, 
														ULONG AllocationType, ULONG Protect);
			static LPVOID Free(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T RegionSize, ULONG FreeType);
			/// <summary>
			/// changes the access protection on a region of committed pages in the virtual address space of the calling process
			/// return OldAccessProtection, and zero if failed
			/// </summary>
			static ULONG Protect(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T RegionSize, ULONG NewAccessProtection);
			/// <summary>
			/// retrieves parameters of queried memory block(thread-unsafe) and returns nullptr if failed
			/// </summary>
			static struct MEMORY_INFORMATION *Query(HANDLE ProcessHandle, PVOID BaseAddress);
			static size_t GetPageSize();
			static size_t GetPhysicalPages();
		};
		typedef struct MEMORY_INFORMATION
		{
			PVOID  BaseAddress;
			PVOID  AllocationBase;
			DWORD  AllocationProtect;
			SIZE_T RegionSize;
			DWORD  State; // Unknown
			DWORD  Protect;
			DWORD  Type;  // Unknown
		} *PMEMORY_INFORMATION;
	}
}
#endif // _USE_MEMORY_ALLOCATOR