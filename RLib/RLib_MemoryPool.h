/********************************************************************
	Created:	2012/04/22  8:45
	Filename: 	RLib_MemoryPool.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_MEMORYPOOL
#define _USE_MEMORYPOOL
#include "RLib_MemoryPage.h"
#include "RLib_Exception.h"
#include "RLib_IntrinsicLock.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// A nonblocking thread-safe memory-pool implementation
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE MemoryPool
		{
		public:
			typedef struct MEMORY_PAGE
			{
				MemoryPage            Page;
				Threading::AtomicLock SyncRoot; // lock object
#ifdef _DEBUG
				MEMORY_PAGE          *pNext;    // for debug view
				volatile intptr_t     Holders;  // for debug
#endif // _DEBUG
			} *PMEMPAGE;

		private:
			LPVOID page_try_alloc(intptr_t index, intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM);
			LPVOID page_alloc(intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM);
			LPVOID rand_alloc(intptr_t low, intptr_t up, intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM);

		public:
			PMEMPAGE MemoryList;
			intptr_t MemoryCount;

		public:	
			MemoryPool(intptr_t memory_count = RLIB_PAGECOUNT);
			~MemoryPool();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// Allocates memory, with debug information if provided [RLIB_INTERNAL_DEBUG_PARAM_HERE]
			/// </summary>
			RLIB_RESTRICT_RETURN void *AllocByte(intptr_t size
												 RLIB_INTERNAL_DEBUG_PARAM);
			/// <summary>
			/// Releases memory
			/// </summary>
			void Collect(void *lptr);
			/// <summary>
			/// Trys to change the size of memory allocated,
			/// if succeed and the new size is greater than the old size,
			/// the data in the expanded memory is undefined.
			/// </summary>
			bool ReSize(void *lptr, intptr_t new_size);
			/// <summary>
			/// Gets the size of specified memory,
			/// @warning in DEBUG version the return value is original unaligned size,
			/// while in RELEASE version is aligned.
			/// </summary>
			intptr_t GetSize(void *lptr);
			/// <summary>
			/// Reallocates memory, copys and releases original memory if succeed
			/// </summary>
			RLIB_RESTRICT_RETURN void *ReAlloc(void *lptr, intptr_t size);
			/// <summary>
			/// First, trys reallocate memory on the same page, 
			/// if failed, the function behaves like ReAlloc
			/// </summary>
			RLIB_RESTRICT_RETURN void *TryReAlloc(void *lptr, intptr_t size);		
			/// <summary>
			/// Gets the total size of allocated memory, no guarantee of accuracy
			/// </summary>
			intptr_t GetUsage();
			/// <summary>
			/// Gets the total size of committed memory, no guarantee of accuracy
			/// </summary>
			intptr_t GetMemorySize();
			/// <summary>
			/// Trys to collect unused memory and return to system if possible
			/// </summary>
			intptr_t TryGCCollect();
			/// <summary>
			/// Forces to collect unused memory and return to system,
			/// and waits until all collect tasks are completed
			/// </summary>
			intptr_t WaitForFullGCComplete();
			/// <summary>
			/// Gets the associated MemoryPage object by index
			/// @warning no grantee of thread-safe, uses SyncRoot to sync
			/// </summary>
			MEMORY_PAGE &operator [] (intptr_t index) {
				assert(index >= 0 && index < this->MemoryCount);
				return this->MemoryList[index];
			}

		public:
			/// <summary>
			/// Sets a breakpoint on a specified object allocation order number(DEBUG only),
			/// which allows you to perform memory leak detection by breaking at a specific point of memory allocation,
			/// and tracing back to the origin of the request
			/// </summary>
			static void SetBreakAlloc(__int64 lBreakAlloc);
		};
	}
}

#endif // _USE_MEMORYPOOL