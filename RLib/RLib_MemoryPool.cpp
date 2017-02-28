/********************************************************************
Created:	2012/04/22  8:46
Filename: 	RLib_MemoryPool.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Random.h"
#include "RLib_MemoryPool.h"
#include "RLib_Threading.h"

#if RLIB_DISABLE_NATIVE_API
# include <heapapi.h>
# define RLIB_HAEPALLOC HeapAlloc
# define RLIB_HAEPFREE  HeapFree
#else
# define RLIB_HAEPALLOC RtlAllocateHeap
# define RLIB_HAEPFREE  RtlFreeHeap
#endif // RLIB_DISABLE_NATIVE_API

using namespace System::IO;

#ifdef _DEBUG
volatile static __int64 __req_break = 0; // break request
volatile static __int64 __seq_order = 0; // sequential object allocation order number
# if (WINVER > _WIN32_WINNT_WIN7)
#  include <debugapi.h>
# endif
# include <CRTDBG.h>
# include <inttypes.h>
# include "RLib_StringHelper.h"
# define SET_REQ_BREAK(v)    Threading::Interlocked::Exchange64(&__req_break, v)
# define SET_SEQ_ORDER(p)    p->order = Threading::Interlocked::Increment64(&__seq_order); if (p->order == __req_break) _CrtDbgBreak()
# define LOCKER_TAKEN(index) ++this->MemoryList[index].Holders; assert(this->MemoryList[index].Holders == 1)
# define LOCKER_LOST(index)  --this->MemoryList[index].Holders
#else
# define SET_REQ_BREAK(v)    UNREFERENCED_PARAMETER(v)
# define SET_SEQ_ORDER(p)    ((void)0)
# define LOCKER_TAKEN(index) ((void)0)
# define LOCKER_LOST(index)  ((void)0)
#endif // _DEBUG

typedef struct MEMORY_BLOCK_HEADER
{
#ifdef _DEBUG
	__int64  order;  // the sequential object allocation order number 
# ifndef _WIN64
	intptr_t unused; // dummy, aligned
# endif // !_WIN64
#endif // _DEBUG
	intptr_t index;  // allocator index

public:
	RLIB_FORCE_INLINE void *skip_block_header() {
		return reinterpret_cast<LPBYTE>(this) + sizeof(MEMORY_BLOCK_HEADER);
	}
} *PMEMBLOCK;

//-------------------------------------------------------------------------

MemoryPool::MemoryPool(intptr_t memory_count /* = RLIB_PAGECOUNT */)
{
	auto lpprocheap   = static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessHeap;
	this->MemoryList  = static_cast<PMEMPAGE>(RLIB_HAEPALLOC(lpprocheap, HEAP_ZERO_MEMORY, sizeof(MEMORY_PAGE) * memory_count));
    this->MemoryCount = memory_count;
	
    for (intptr_t index = 0; index < this->MemoryCount; ++index)
    {
        RLIB_InitClass(&this->MemoryList[index], MEMORY_PAGE);
		// break if failed
		if (!this->MemoryList[index].Page.IsAvailable()) {			
			this->MemoryCount = index;
#ifdef _DEBUG
			trace(!"create memory page failed!");
			index != 0 && (this->MemoryList[index - 1].pNext = nullptr) == nullptr;
			this->MemoryList[index].pNext = nullptr;
#endif // _DEBUG
			break;
		} //if

#ifdef _DEBUG
		this->MemoryList[index].pNext   = 
			((index + 1) == this->MemoryCount) ? nullptr : &this->MemoryList[index + 1];
		this->MemoryList[index].Holders = 0;
#endif // _DEBUG
    }
}

//-------------------------------------------------------------------------

MemoryPool::~MemoryPool()
{
#ifdef _DEBUG
	bool leak_detected = false;
#endif //_DEBUG

    for (int index = 0; index < this->MemoryCount; ++index)
    {
#ifdef _DEBUG
		assert(this->MemoryList[index].Holders == 0);
		if (this->MemoryList[index].Page.GetUsage() != 0) {
			if (!leak_detected) {
				leak_detected = true;
				OutputDebugString(RLIB_NEWLINE _T("Detected memory leaks!") RLIB_NEWLINE _T("Dumping objects ->") RLIB_NEWLINE);
				if (!AppBase::IsDebuggerPresent()) {
					alert(_T("Detected memory leaks!") RLIB_NEWLINE _T("Dumping objects ->") RLIB_NEWLINE);
				} //if
			} //if
		} //if

		this->MemoryList[index].Page.SetMemoryLeakHook([](void *lptr, intptr_t size, LPCTSTR func, LPCTSTR file) 
		{
			#pragma warning(disable:4710) // function not inlined, see declaration of '_sntprintf_s'
			size -= sizeof(MEMORY_BLOCK_HEADER);
			LOCAL_NEW(debug_msg, RLIB_DEFAULT_BUFFER_SIZE);
			LOCAL_INIT(debug_msg);
			LOCAL_APPEND(debug_msg, _T("{%") _T(PRId64) _T("} normal block at 0x%p, %") _T(PRId64) _T(" bytes long.") RLIB_NEWLINE,
						 static_cast<__int64>(static_cast<PMEMBLOCK>(lptr)->order),
						 static_cast<PMEMBLOCK>(lptr)->skip_block_header(),
						 static_cast<__int64>(size)
						 );
			// print data view
			LOCAL_NEW(print_msg, 32 + 1);
			LOCAL_NEW(print_hex, 48 + 1);
			LOCAL_INIT(print_hex);
			for (intptr_t i = 0; i < min(size, 16); i++) {
				int c = static_cast<unsigned char *>(static_cast<PMEMBLOCK>(lptr)->skip_block_header())[i];
				if (isprint(c)) {
					print_msg[i * 2] = static_cast<TCHAR>(c);
				} else {
					print_msg[i * 2] = _T('.');
				} //if
				print_msg[i * 2 + 1] = _T(' ');
				print_msg[i * 2 + 2] = _T('\0');
				LOCAL_APPEND(print_hex, _T("%.2x "), c);
				//_itot_s(c, &print_hex[i * 3], 3, 16);
			}
			LOCAL_APPEND(debug_msg, _T(" Data: %s %s") RLIB_NEWLINE, print_msg, print_hex);
			LOCAL_APPEND(debug_msg, _T(" at %s(%s)") RLIB_NEWLINE, func, file);
			OutputDebugString(debug_msg);
			if (!AppBase::IsDebuggerPresent()) {
				alert(debug_msg);
			} //if
		});
#endif // _DEBUG

        this->MemoryList[index].~MEMORY_PAGE();
    }
    
#ifdef _DEBUG
	if (leak_detected) {
		OutputDebugString(_T("Object dump complete.") RLIB_NEWLINE RLIB_NEWLINE);
	} //if
#endif // _DEBUG

	RLIB_HAEPFREE(static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessHeap, NULL, this->MemoryList);
}

//-------------------------------------------------------------------------

static void __check_index_bound(intptr_t index, intptr_t up_bound)
{
#ifdef _DEBUG
	assert(RLIB_ARRAYRANGE(index, 0, up_bound));
#else
	UNREFERENCED_PARAMETER(index);
	UNREFERENCED_PARAMETER(up_bound);
#endif // _DEBUG
}

//-------------------------------------------------------------------------

static void __print_fail_alloc(intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM)
{
#ifdef _DEBUG
	LOCAL_NEW(buffer, RLIB_DEFAULT_BUFFER_SIZE);
	LOCAL_INIT(buffer);
	LOCAL_APPEND(buffer,
				 _T("Insufficient memory!") RLIB_NEWLINE _T("Failed to allocate %") _T(PRId64) _T(" bytes in %s, file %s") RLIB_NEWLINE,
				 static_cast<__int64>(bytes) RLIB_INTERNAL_DEBUG_PARAM_VALUE
	);

	alert(buffer);
#else
	UNREFERENCED_PARAMETER(bytes);
#endif // _DEBUG

}

//-------------------------------------------------------------------------

void MemoryPool::SetBreakAlloc(__int64 lBreakAlloc)
{
	SET_REQ_BREAK(lBreakAlloc);
}

//-------------------------------------------------------------------------

LPVOID MemoryPool::page_try_alloc(intptr_t index, intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM)
{
#ifdef _DEBUG
	if (!this->MemoryList[index].Page.IsAvailable()) {
		trace(!"an unexpected exception occurred!");
		return nullptr;
	} //if
#endif // _DEBUG

	LPVOID lpblock = nullptr;
	if (this->MemoryList[index].SyncRoot.TryLock()) {
		LOCKER_TAKEN(index);
		if (bytes <= this->MemoryList[index].Page.GetMaxAllocSize()) {
			lpblock = this->MemoryList[index].Page.AllocByte(bytes RLIB_INTERNAL_DEBUG_PARAM_VALUE);
		} //if
		LOCKER_LOST(index);
		this->MemoryList[index].SyncRoot.UnLock();
	} //if
	return lpblock;
}

//-------------------------------------------------------------------------

LPVOID MemoryPool::rand_alloc(intptr_t low, intptr_t up, intptr_t bytes
							  RLIB_INTERNAL_DEBUG_PARAM)
{
	intptr_t  index;
	intptr_t  retry_count = RLIB_ALLOCTRYCOUNT;
	PMEMBLOCK lpblock;
	Random    rnd(PtrToUlong(&bytes));
	do {
		index = rnd.NextLong(low, up);
		assert(index >= low && index < up);

		lpblock = static_cast<PMEMBLOCK>(this->page_try_alloc(index, bytes RLIB_INTERNAL_DEBUG_PARAM_VALUE));
		if (lpblock != nullptr) {
			lpblock->index = index;
			SET_SEQ_ORDER(lpblock);
			break;
		} //if

	} while (--retry_count >= 0);

	return lpblock;
}

//-------------------------------------------------------------------------

LPVOID MemoryPool::page_alloc(intptr_t bytes RLIB_INTERNAL_DEBUG_PARAM)
{
	PMEMBLOCK lpblock = nullptr;
	for (intptr_t index = this->MemoryCount - 1; index >= 0; --index) {
		Synchronized(this->MemoryList[index].SyncRoot, {
			LOCKER_TAKEN(index); 
			if (bytes <= this->MemoryList[index].Page.GetMaxAllocSize()) {
				lpblock = static_cast<PMEMBLOCK>(this->MemoryList[index].Page.AllocByte(bytes 
																						RLIB_INTERNAL_DEBUG_PARAM_VALUE));
			} //if
			LOCKER_LOST(index);
		});
		if (lpblock != nullptr) {
			lpblock->index = index;
			SET_SEQ_ORDER(lpblock);
#if defined(_DEBUG) && !defined(_WIN64)
			lpblock->unused = 0xbbbbbbbb;
#endif // _DEBUG && !_WIN64

			// must return user pointer
			return lpblock->skip_block_header();
		} //if
	}

#ifdef _DEBUG
	__print_fail_alloc(bytes RLIB_INTERNAL_DEBUG_PARAM_VALUE);
#endif // _DEBUG

	return nullptr;
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN void *MemoryPool::AllocByte(intptr_t bytes
												 RLIB_INTERNAL_DEBUG_PARAM)
{
	intptr_t  partition_index = this->MemoryCount / 2;
	intptr_t  internal_bytes  = bytes + RLIB_SIZEOF(MEMORY_BLOCK_HEADER);
	PMEMBLOCK lpblock;
	if (internal_bytes <= RLIB_FRAGTHRESHOLD) {
		lpblock = static_cast<PMEMBLOCK>(this->rand_alloc(0, partition_index, internal_bytes
														  RLIB_INTERNAL_DEBUG_PARAM_VALUE));
	} else {
		lpblock = static_cast<PMEMBLOCK>(this->rand_alloc(partition_index, this->MemoryCount, internal_bytes
														  RLIB_INTERNAL_DEBUG_PARAM_VALUE));
	} //if

	if (lpblock != nullptr) {
		// allocated successfully
#if defined(_DEBUG) && !defined(_WIN64)
		lpblock->unused = 0xbbbbbbbb;
#endif // _DEBUG && !_WIN64
		return lpblock->skip_block_header();
	} //if

	// forces to do usual allocation
	return this->page_alloc(internal_bytes RLIB_INTERNAL_DEBUG_PARAM_VALUE);
}

//-------------------------------------------------------------------------

void MemoryPool::Collect(void *lptr)
{
	assert(lptr != nullptr);
	auto lpblock = RLIB_INTERNAL_USER_TO_HEADER(lptr);

	__check_index_bound(lpblock->index, this->MemoryCount);
#if defined(_DEBUG) && !defined(_WIN64)
	lpblock->unused = 0xaaaaaaaa;
#endif // _DEBUG && !_WIN64

	assert(this->MemoryList[lpblock->index].Page.IsAvailable());
	Synchronized(this->MemoryList[lpblock->index].SyncRoot, {
		LOCKER_TAKEN(lpblock->index); 
		this->MemoryList[lpblock->index].Page.Collect(lpblock);
		LOCKER_LOST(lpblock->index);
	});
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN void *MemoryPool::TryReAlloc(void *lptr, intptr_t bytes)
{
	assert(lptr != nullptr);

	PMEMBLOCK lpblock       = RLIB_INTERNAL_USER_TO_HEADER(lptr);
	intptr_t internal_bytes = bytes + RLIB_SIZEOF(MEMORY_BLOCK_HEADER);
	intptr_t index          = lpblock->index;

	__check_index_bound(index, this->MemoryCount);
	assert(this->MemoryList[index].Page.IsAvailable());

	// trys to reallocate on the original page first
	LPVOID result_ptr = nullptr; // user pointer
	Synchronized(this->MemoryList[index].SyncRoot, {
		LOCKER_TAKEN(index);
		if (this->MemoryList[index].Page.ReSize(lpblock, internal_bytes)) {
			// resizes successfully, just return the original pointer 
			result_ptr = lptr;
		} else {
			result_ptr = this->MemoryList[index].Page.ReAlloc(lpblock, internal_bytes);
			if (result_ptr != nullptr) {
				// lpblock is now released
				SET_SEQ_ORDER(static_cast<PMEMBLOCK>(result_ptr));
				result_ptr = static_cast<PMEMBLOCK>(result_ptr)->skip_block_header();
			} //if
		} //if
		LOCKER_LOST(index);
	});

	if (result_ptr == nullptr) {
		// failed, trys other pages
		result_ptr = MemoryPool::AllocByte(bytes RLIB_INTERNAL_DEBUG_PARAM_HERE);
		if (result_ptr != nullptr) {
			// gets original internal size, thread safe
			intptr_t ptr_size = this->MemoryList[index].Page.GetSize(lpblock) - RLIB_SIZEOF(MEMORY_BLOCK_HEADER);
			assert(lpblock >= 0);
			memcpy(result_ptr, lptr, static_cast<size_t>(RLIB_MIN(ptr_size, bytes)));

			// releases original memory
			Synchronized(this->MemoryList[index].SyncRoot, {
				LOCKER_TAKEN(index); 
				this->MemoryList[index].Page.Collect(lpblock);
				LOCKER_LOST(index);
			});
		} //if
	} //if

    return result_ptr;
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN void *MemoryPool::ReAlloc(void *lptr, intptr_t bytes)
{
	assert(lptr != nullptr);

	LPVOID result_ptr = MemoryPool::AllocByte(bytes RLIB_INTERNAL_DEBUG_PARAM_HERE);
	if (result_ptr != nullptr)
	{
		PMEMBLOCK lpblock = RLIB_INTERNAL_USER_TO_HEADER(lptr);
		intptr_t index    = lpblock->index;
		__check_index_bound(index, this->MemoryCount);
		assert(this->MemoryList[index].Page.IsAvailable());

		intptr_t ptr_size = this->MemoryList[index].Page.GetSize(lpblock) - RLIB_SIZEOF(MEMORY_BLOCK_HEADER);
		assert(ptr_size >= 0);
		memcpy(result_ptr, lptr, static_cast<size_t>(RLIB_MIN(bytes, ptr_size)));

		Synchronized(this->MemoryList[index].SyncRoot, {
			LOCKER_TAKEN(index);
			this->MemoryList[index].Page.Collect(lpblock);
			LOCKER_LOST(index);
		});
	} //if

	return result_ptr;
}

//-------------------------------------------------------------------------

bool MemoryPool::ReSize(LPVOID lptr, intptr_t bytes)
{
	assert(lptr != nullptr);

	PMEMBLOCK lpblock = RLIB_INTERNAL_USER_TO_HEADER(lptr);
	intptr_t index    = lpblock->index;
	__check_index_bound(index, this->MemoryCount);
	assert(this->MemoryList[index].Page.IsAvailable());

	bool result = false;
	Synchronized(this->MemoryList[index].SyncRoot, {
		LOCKER_TAKEN(index);
		result = this->MemoryList[index].Page.ReSize(lpblock, bytes + RLIB_SIZEOF(MEMORY_BLOCK_HEADER));
		LOCKER_LOST(index);
	});

	return result;
}

//-------------------------------------------------------------------------

intptr_t MemoryPool::GetSize(LPVOID lptr)
{
	assert(lptr != nullptr);

	PMEMBLOCK lpblock = RLIB_INTERNAL_USER_TO_HEADER(lptr);
	intptr_t index    = lpblock->index;
	__check_index_bound(index, this->MemoryCount);
	assert(this->MemoryList[index].Page.IsAvailable());

	// thread safe
	return this->MemoryList[index].Page.GetSize(lpblock) - RLIB_SIZEOF(MEMORY_BLOCK_HEADER);
}

//-------------------------------------------------------------------------

intptr_t MemoryPool::GetUsage()
{
    intptr_t bytesUsed = 0;
    for (int index = 0; index < this->MemoryCount; ++index)
    {
		if (this->MemoryList[index].Page.IsAvailable() &&
			this->MemoryList[index].SyncRoot.TryEnter())
        {
			LOCKER_TAKEN(index);
            bytesUsed += this->MemoryList[index].Page.GetUsage();
			LOCKER_LOST(index);
            this->MemoryList[index].SyncRoot.Exit();
        } //if
    }
    return bytesUsed;
}

//-------------------------------------------------------------------------

intptr_t MemoryPool::GetMemorySize()
{
	intptr_t bytesUsed = 0;
	for (int index = 0; index < this->MemoryCount; ++index) {
		if (this->MemoryList[index].Page.IsAvailable() &&
			this->MemoryList[index].SyncRoot.TryEnter()) {
			LOCKER_TAKEN(index);
			bytesUsed += this->MemoryList[index].Page.GetMemorySize();
			LOCKER_LOST(index);
			this->MemoryList[index].SyncRoot.Exit();
		} //if
	}
	return bytesUsed;
}

//-------------------------------------------------------------------------

intptr_t MemoryPool::TryGCCollect()
{
	intptr_t bytesCollected = 0;
	for (int index = 0; index < this->MemoryCount; ++index) {
		if (this->MemoryList[index].Page.IsAvailable() &&
			this->MemoryList[index].SyncRoot.TryEnter()) {
			LOCKER_TAKEN(index);
			bytesCollected += this->MemoryList[index].Page.Shrink();
			LOCKER_LOST(index);
			this->MemoryList[index].SyncRoot.Exit();
		} //if
	}
	return bytesCollected;
}

//-------------------------------------------------------------------------

intptr_t MemoryPool::WaitForFullGCComplete()
{
    intptr_t bytesCollected = 0;
    for (int index = 0; index < this->MemoryCount; ++index)
    {
		if (!this->MemoryList[index].Page.IsAvailable()) continue;

		Synchronized(this->MemoryList[index].SyncRoot, {
			LOCKER_TAKEN(index);
			bytesCollected += this->MemoryList[index].Page.Shrink();
			LOCKER_LOST(index);
		});
    }
    return bytesCollected;
}