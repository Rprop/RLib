/********************************************************************
	Created:	2011/08/14  20:33
	Filename: 	RLib_MemoryPage.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_MEMORY
#define _USE_MEMORY
#include "RLib_AppBase.h"
#include "RLib_MemoryAllocator.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// The namespace contains types that allow reading and writing to files and data streams, 
	/// and types that provide basic file and directory support
	/// </summary>
	namespace IO
	{
		/// <summary>
		/// Provides a Memory Page class base on Double-Linked List,
		/// which is the core of MemoryPool
		/// </summary>
		class RLIB_API MemoryPage
		{
			friend class MemoryPool;

		protected:
			// pointer validation status
			typedef enum
			{
				PTR_IN = 0,
				PTR_HIGH,
				PTR_LOW
			} PTR_STATUS;
			// allocation block info structure
			typedef struct BLOCK_INFO *PBLOCK_INFO;

		protected:		
			PBLOCK_INFO m_base_ptr; 
			PBLOCK_INFO m_first_ptr;		
			PBLOCK_INFO m_last_ptr; 
			intptr_t    m_now;  // committed size
			intptr_t    m_used; // allocated size(using)	
			intptr_t    m_reserved;
#ifdef _DEBUG			
			intptr_t    m_succeed_count; 	
			intptr_t    m_failed_count;
			void      (*m_memoryleak_hook)(void *, intptr_t, LPCTSTR func, LPCTSTR file);
#endif // _DEBUG

		public:
			static intptr_t PageSize; // default page size

		protected:
			PBLOCK_INFO find(intptr_t);
			LPVOID      alloc_block(intptr_t);
			PBLOCK_INFO alloc_tail(PBLOCK_INFO, intptr_t, PBLOCK_INFO);
			bool extend(intptr_t);                   
			void init(intptr_t, intptr_t);
			void free_block(PBLOCK_INFO); // releases block without initiating to zero

		public:
			MemoryPage();
			MemoryPage(intptr_t commitSize, intptr_t reserveSize);
			~MemoryPage();
			RLIB_DECLARE_DYNCREATE;

		public:
#ifdef _DEBUG
			void SetMemoryLeakHook(RLIB_TYPE(MemoryPage::m_memoryleak_hook) lphook) {
				this->m_memoryleak_hook = lphook;
			}
#endif // _DEBUG
			/// <summary>
			/// Gets a value indicating whether the page has been created successfully
			/// </summary>
			bool IsAvailable() const;
			/// <summary>
			/// Allocates a block of memory from a memory page
			/// @warning the allocated memory is no guarantee to be initialized to zero
			/// </summary>
			RLIB_RESTRICT_RETURN void *AllocByte(intptr_t 
												 RLIB_INTERNAL_DEBUG_PARAM);
#ifdef _DEBUG
			/// <summary>
			/// Allocates a block of memory from a memory page
			/// @warning the allocated memory is no guarantee to be initialized to zero
			/// </summary>
			RLIB_RESTRICT_RETURN void *AllocByte(intptr_t size) {
				return this->AllocByte(size RLIB_INTERNAL_DEBUG_PARAM_HERE);
			}
#endif // _DEBUG
			/// <summary>
			/// Frees a memory block allocated from a memory page
			/// </summary>
			void Collect(LPVOID);    
			/// <summary>
			/// Resizes a block of memory without moving, which enables you to resize a memory block
			/// @warning the allocated memory is no guarantee to be initialized to zero
			/// </summary>
			bool ReSize(LPVOID, intptr_t);
			/// <summary>
			/// Reallocates a block of memory, which enables you to resize a memory block and move it.
			/// If new_size equals -1, the original size is kept
			/// @warning the allocated memory is no guarantee to be initialized to zero
			/// </summary>
			/// <returns>return nullptr if failed, and the original memory block is unaffected</returns>
			RLIB_RESTRICT_RETURN void *ReAlloc(LPVOID, intptr_t new_size = -1);
			/// <summary>
			/// Trys to uncommit(return to the system) unused memory
			/// </summary>
			intptr_t Shrink();
			/// <summary>
			/// Gets the aligned size of memory allocated,
			/// @warning the function return unaligned size when defined(_DEBUG)
			/// </summary>
			RLIB_THREAD_SAFE intptr_t GetSize(LPCVOID);	    
			/// <summary>
			/// Gets the size of allocated memory
			/// </summary>
			intptr_t GetUsage();      
			/// <summary>
			/// Gets the size of committed memory	
			/// </summary>
			intptr_t GetMemorySize();    
			/// <summary>
			/// Gets the size of unallocated memory, no guarantee of continuity
			/// </summary>
			intptr_t GetUnusedSize();
			/// <summary>
			/// Gets the maximum size of available memory, no guarantee of accuracy as memory fragmentation
			/// </summary>
			intptr_t GetMaxAllocSize();
			/// <summary>
			/// Validate a pointer, and return zero if it belongs to current instance
			/// </summary>
			PTR_STATUS Validate(LPCVOID);       
			/// <summary>
			/// Dumps the entire pages into file for analysis
			/// </summary>
			void DumpToFile(const TCHAR *lpszFile);
		};
	};
};

#endif //_USE_MEMORY