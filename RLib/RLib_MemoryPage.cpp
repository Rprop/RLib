/********************************************************************
Created:	2011/08/14  20:34
Filename: 	RLib_Memory.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Utility.h"
#include "RLib_MemoryPage.h"
using namespace System::IO;

#ifdef _DEBUG
# include "RLib_CRC.h"
# ifdef _WIN64
#  define RtlComputeCrc(a, b, c) System::Security::Cryptography::CRC::native_crc64(c, a, b)
# else
#  define RtlComputeCrc(a, b, c) System::Security::Cryptography::CRC::native_crc32(c, a, b)
# endif // _WIN64
# define __rlib_debug_increase_count(_count) ++_count;
#else
# define __rlib_debug_increase_count(_count) ((void)0)
#endif // _DEBUG

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		struct BLOCK_INFO
		{
#ifdef _DEBUG
			uintptr_t   crc; // used for overflow checking
#endif // _DEBUG
			intptr_t    aligned_size; // aligned size, including block info header size		
			BLOCK_INFO *prior;
			BLOCK_INFO *next;
#ifdef _DEBUG
			LPCTSTR  alloc_function;
			LPCTSTR  alloc_file;
			union VALID_MARK // used for overflow checking, must be filled with RLIB_VALIDBYTE
			{
				intptr_t      valid_mark;
				unsigned char valid_bytes[sizeof(intptr_t)];
			};
			intptr_t valid_index; // used for overflow checking, indicating the index of valid mark
#endif // _DEBUG

		public:
			RLIB_INLINE void set_crc() {
#ifdef _DEBUG
				this->crc = RtlComputeCrc(&this->crc + 1, RLIB_INTERNAL_BLOCK_SIZE - sizeof(this->crc), 0U);
#endif // _DEBUG
			}
			RLIB_INLINE void set_valid_mark() {
#ifdef _DEBUG
				this->set_crc();

				auto lpmark = RLIB_INTERNAL_HANDLE_TO_USER(this) + this->valid_index;
				for (intptr_t i = 0; i < sizeof(VALID_MARK); ++i) {
					reinterpret_cast<VALID_MARK *>(lpmark)->valid_bytes[i] = RLIB_VALIDBYTE;
				}
#endif // _DEBUG
			}
			RLIB_INLINE void valid() {
#ifdef _DEBUG
				uintptr_t kcrc = RtlComputeCrc(&this->crc + 1, RLIB_INTERNAL_BLOCK_SIZE - sizeof(this->crc), 0U);

				auto lpmark = RLIB_INTERNAL_HANDLE_TO_USER(this) + this->valid_index;
				bool bcheck = true;
				for (intptr_t i = 0; i < sizeof(VALID_MARK); ++i) {
					bcheck &= reinterpret_cast<VALID_MARK *>(lpmark)->valid_bytes[i] == RLIB_VALIDBYTE;
				}

				assert(bcheck || !"Detected memory overflow!");
				assert(kcrc == this->crc || !"Detected memory block corruption!");
#endif // _DEBUG
			}
		};
	}
}

//-------------------------------------------------------------------------

void MemoryPage::init(intptr_t _commit_size, intptr_t _reserve_size)
{
	_commit_size = Utility::round_up(_commit_size, PageSize);
	if (_reserve_size < _commit_size) {
		_reserve_size = _commit_size; // minimum reserve size
	} else if (_reserve_size % PageSize != 0) {
		_reserve_size = ((_reserve_size / PageSize) + 1) * PageSize;
	} //if

    // reserves memory
	this->m_base_ptr = static_cast<PBLOCK_INFO>(MemoryAllocator::Allocate(AppBase::GetCurrentProcess(),
																		  NULL, static_cast<SIZE_T>(_reserve_size), RLIB_MEM_RESERVE, RLIB_PAGE_READWRITE));
	if (this->m_base_ptr == nullptr) {
		// reserves failed
		return;
	} //if

	// field init
	this->m_now       = _commit_size;
	this->m_used      = 0;
    this->m_reserved  = _reserve_size;
    this->m_first_ptr = nullptr;
    this->m_last_ptr  = nullptr;
#ifdef _DEBUG
    this->m_succeed_count   = 0;
    this->m_failed_count    = 0;
	this->m_memoryleak_hook = nullptr;
#endif // _DEBUG

    // commits memory  
	LPVOID lptr = MemoryAllocator::Allocate(AppBase::GetCurrentProcess(), this->m_base_ptr,
											static_cast<SIZE_T>(_commit_size), RLIB_MEM_COMMIT, RLIB_PAGE_READWRITE);
    if (lptr == nullptr) {
		this->~MemoryPage(); // commits failed		
	} //if
}

//-------------------------------------------------------------------------

MemoryPage::MemoryPage()
{
    this->init(RLIB_COMMITSIZE, RLIB_RESERVESIZE);
}

//-------------------------------------------------------------------------

MemoryPage::MemoryPage(intptr_t commitSize, intptr_t reserveSize)
{
    this->init(commitSize, reserveSize);
}

//-------------------------------------------------------------------------

MemoryPage::~MemoryPage()
{
#ifdef _DEBUG
	if (this->m_used != 0) {
		assert(this->m_first_ptr != nullptr);

		auto lptr = this->m_first_ptr;
		while (lptr != nullptr) {
			if (this->m_memoryleak_hook) {
				this->m_memoryleak_hook(RLIB_INTERNAL_HANDLE_TO_USER(lptr), lptr->valid_index,
										lptr->alloc_function, lptr->alloc_file);
			} else {
				TCHAR debug_msg[512] = { RLIB_NEWLINE _T("Detected memory leaks!") RLIB_NEWLINE _T("    at ") };
				_tcscat_s(debug_msg, RLIB_COUNTOF(debug_msg), lptr->alloc_function);
				_tcscat_s(debug_msg, RLIB_COUNTOF(debug_msg), _T("("));
				_tcscat_s(debug_msg, RLIB_COUNTOF(debug_msg), lptr->alloc_file);
				_tcscat_s(debug_msg, RLIB_COUNTOF(debug_msg), _T(")") RLIB_NEWLINE);
				alert(debug_msg);
			} //if
			lptr = lptr->next;
		}
	} else {
		assert(this->m_first_ptr == nullptr && this->m_last_ptr == nullptr);
	} //if
#endif // _DEBUG

	if (this->IsAvailable()) {
		MemoryAllocator::Free(AppBase::GetCurrentProcess(), this->m_base_ptr, 0, RLIB_MEM_RELEASE);
		this->m_base_ptr = nullptr;
	} //if
}

//-------------------------------------------------------------------------

bool MemoryPage::extend(intptr_t ex_size)
{
	assert(ex_size >= 0);

	// extended size
	ex_size = Utility::round_up(ex_size, PageSize) + this->m_now;
	if (ex_size <= this->m_reserved) {
		auto result = MemoryAllocator::Allocate(AppBase::GetCurrentProcess(),
												this->m_base_ptr, 
												static_cast<SIZE_T>(ex_size),
												RLIB_MEM_COMMIT, RLIB_PAGE_READWRITE);
		if (result != nullptr) {
			this->m_now = ex_size;
			return true;
		} //if
	} //if

    return false;
}

//-------------------------------------------------------------------------

MemoryPage::PBLOCK_INFO MemoryPage::alloc_tail(MemoryPage::PBLOCK_INFO expected_ptr, intptr_t size_required,
											   MemoryPage::PBLOCK_INFO prior_ptr)
{
	auto avail_size = (reinterpret_cast<LPBYTE>(this->m_base_ptr) + this->m_now) - reinterpret_cast<LPBYTE>(expected_ptr);
    if (avail_size < size_required) {
		// not enough space can be allocated
		if (!this->extend(size_required - avail_size)) {
			return nullptr;
		} //if
    } //if

	// allocate successfully
	assert(prior_ptr->next == nullptr);
	prior_ptr->valid();
	prior_ptr->next     = expected_ptr;
	prior_ptr->set_crc();
	this->m_last_ptr    = expected_ptr;
	expected_ptr->prior = prior_ptr;
    expected_ptr->next  = nullptr;
    return expected_ptr;
}

//-------------------------------------------------------------------------

MemoryPage::PBLOCK_INFO MemoryPage::find(intptr_t _size)
{
    // at least one node when calling find
    assert(this->m_first_ptr != nullptr);
    assert(this->m_first_ptr->prior == nullptr);
    assert(this->m_last_ptr != nullptr);
    assert(this->m_last_ptr->next == nullptr);

	// it is impossible to allocate before this->m_last_ptr
	if (_size >= (this->m_now - this->m_used)) {
		auto lpend = reinterpret_cast<LPBYTE>(this->m_last_ptr) + this->m_last_ptr->aligned_size;
		return this->alloc_tail(reinterpret_cast<PBLOCK_INFO>(lpend), 
								_size, this->m_last_ptr);
	} //if

	this->m_first_ptr->valid(); // debug only

    // check if blocks before the first node are available
    if (this->m_first_ptr != static_cast<PBLOCK_INFO>(this->m_base_ptr))
    {
        assert(this->m_first_ptr > reinterpret_cast<PBLOCK_INFO>(this->m_base_ptr));
		ptrdiff_t freesize = reinterpret_cast<LPBYTE>(this->m_first_ptr) - reinterpret_cast<LPBYTE>(this->m_base_ptr);
		if (freesize >= _size) {
			// allocate successfully
			this->m_base_ptr->prior  = nullptr;
			this->m_base_ptr->next   = this->m_first_ptr;
			this->m_first_ptr->valid();
			this->m_first_ptr->prior = this->m_base_ptr;
			this->m_first_ptr->set_crc();
			this->m_first_ptr        = this->m_base_ptr;
			return static_cast<PBLOCK_INFO>(this->m_base_ptr);
		} //if
    } //if

	// case when only one node allocated
    if (this->m_first_ptr->next == nullptr)
    {
        assert(this->m_last_ptr == this->m_first_ptr);
		assert(this->m_first_ptr->aligned_size <= this->GetUsage());
		assert(this->m_first_ptr->aligned_size <= this->GetMemorySize());

		auto lpend = reinterpret_cast<LPBYTE>(this->m_first_ptr) + this->m_first_ptr->aligned_size;
        return this->alloc_tail(reinterpret_cast<PBLOCK_INFO>(lpend), _size, this->m_first_ptr);
    } //if

	PBLOCK_INFO find_ptr = this->m_first_ptr->next;
	while (find_ptr->next != nullptr) // loop until find_ptr == this->m_last_ptr
    {
		// end of find_ptr
		auto end_ptr = reinterpret_cast<LPBYTE>(find_ptr) + find_ptr->aligned_size;

#ifdef _DEBUG
		find_ptr->valid();
		// check link order		
		assert(reinterpret_cast<LPBYTE>(find_ptr->next) >= end_ptr);
		if (find_ptr->next->prior != find_ptr) {
			if (find_ptr->next->alloc_function) {
				alert(find_ptr->next->alloc_function);
			} else if (find_ptr->alloc_function) {
				alert(find_ptr->alloc_function);
			} else {
				trace(!"bad link");
			} //if
		} //if
		assert(find_ptr->aligned_size <= this->GetUsage());
		assert(find_ptr->aligned_size <= this->GetMemorySize());
#endif // _DEBUG

		// break if _size is large than maximum possible size of the rest of committed memory
		auto probable_size = this->m_now - (end_ptr - reinterpret_cast<LPBYTE>(this->m_base_ptr));
		if (_size >= probable_size) {
			find_ptr = this->m_last_ptr;
			break;
		} //if

		// check if memory fragmentation is available
		if ((reinterpret_cast<LPBYTE>(find_ptr->next) - end_ptr) >= _size) {
			PBLOCK_INFO avail_ptr = reinterpret_cast<PBLOCK_INFO>(end_ptr);

			assert(this->Validate(avail_ptr) == PTR_IN);

			avail_ptr->prior      = find_ptr;
			avail_ptr->next       = find_ptr->next;
			find_ptr->next->valid();
			find_ptr->next->prior = avail_ptr;
			find_ptr->next->set_crc();
			find_ptr->valid();
			find_ptr->next        = avail_ptr;
			find_ptr->set_crc();
			return avail_ptr;
		} //if

        find_ptr = find_ptr->next;
	}
    assert(find_ptr == this->m_last_ptr); // must be the last node
    
	find_ptr->valid();
	find_ptr = reinterpret_cast<PBLOCK_INFO>(reinterpret_cast<LPBYTE>(find_ptr) + find_ptr->aligned_size);
	return this->alloc_tail(find_ptr, _size, this->m_last_ptr);
}

//-------------------------------------------------------------------------

LPVOID MemoryPage::alloc_block(intptr_t _size)
{
#ifdef _DEBUG
	assert(_size >= 0);

	// adds valid mark at the tail of memory block for overflow checking
	intptr_t mark_index = _size;
	_size += sizeof(BLOCK_INFO::VALID_MARK);
#endif // _DEBUG

	_size = RLIB_ROUNDUP(RLIB_INTERNAL_BLOCK_SIZE + _size);

	PBLOCK_INFO result_ptr;
	if (this->m_used != 0) {
		result_ptr = this->find(_size);
	} else {
		result_ptr = reinterpret_cast<PBLOCK_INFO>(this->m_base_ptr);
		if (_size <= this->m_now || this->extend(_size - this->m_now)) {
			// allocate successfully
			assert(this->m_first_ptr == nullptr);
			assert(this->m_last_ptr == nullptr);
			this->m_first_ptr = result_ptr;			
			this->m_last_ptr  = result_ptr;
			result_ptr->prior = nullptr;
			result_ptr->next  = nullptr;		
		} else {
			result_ptr        = nullptr;
		} //if
	} //if

	if (result_ptr != nullptr) {
		this->m_used            += _size;
		result_ptr->aligned_size = _size;

#ifdef _DEBUG
		result_ptr->valid_index  = mark_index;
		__rlib_debug_increase_count(this->m_succeed_count);
#endif // _DEBUG

		return result_ptr;
	} //if

	__rlib_debug_increase_count(this->m_failed_count);
    return nullptr;
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN void *MemoryPage::AllocByte(intptr_t bytes
												 RLIB_INTERNAL_DEBUG_PARAM)
{
#ifdef _DEBUG
	auto lpblock = static_cast<PBLOCK_INFO>(this->alloc_block(bytes));
	if (lpblock == nullptr) {
		return nullptr;
	} //if

	memset(RLIB_INTERNAL_HANDLE_TO_USER(lpblock), static_cast<int>(0xfefefefe), static_cast<size_t>(bytes));
	lpblock->alloc_function = function;
	lpblock->alloc_file     = file;
	lpblock->set_valid_mark();

	return RLIB_INTERNAL_HANDLE_TO_USER(lpblock);
#else
	return RLIB_INTERNAL_HANDLE_TO_USER(this->alloc_block(bytes));
#endif // _DEBUG
}

//-------------------------------------------------------------------------

void MemoryPage::Collect(LPVOID lpuser)
{
	assert(this->Validate(lpuser) == PTR_IN);
    this->free_block(RLIB_INTERNAL_USER_TO_HANDLE(lpuser));
}

//-------------------------------------------------------------------------

void MemoryPage::free_block(PBLOCK_INFO handle)
{
#ifdef _DEBUG
	handle->valid();
	assert(handle >= this->m_first_ptr);
	assert(handle <= this->m_last_ptr);
	assert(handle->prior == nullptr || handle->prior->next == handle);
	assert(handle->next == nullptr || handle->next->prior == handle);
#endif // _DEBUG

	if (handle == this->m_first_ptr) {
		assert(handle->prior == nullptr);
		assert(this->m_last_ptr != nullptr);
		assert(this->m_first_ptr == this->m_last_ptr || this->m_first_ptr->next != nullptr);

		// be the first node
		this->m_first_ptr = handle->next;
		if (this->m_first_ptr != nullptr) {
			this->m_first_ptr->valid();
			this->m_first_ptr->prior = nullptr;
			this->m_first_ptr->set_crc();
		} //if
		
		// update the last node
		if (handle == this->m_last_ptr) {
			assert(this->m_first_ptr == nullptr);
			this->m_last_ptr = nullptr;
		} //if

	} else if (handle == this->m_last_ptr) {
		assert(handle->next == nullptr); 
		assert(handle->prior != nullptr);

		// be the last node
		this->m_last_ptr = handle->prior;
		this->m_last_ptr->valid();
		this->m_last_ptr->next = nullptr;
		this->m_last_ptr->set_crc();

	} else {
		assert(handle->prior != nullptr);
		assert(handle->next != nullptr);

		handle->prior->valid();
		handle->prior->next = handle->next;
		handle->prior->set_crc();
		handle->next->valid();
		handle->next->prior = handle->prior;
		handle->next->set_crc();
	} //if

	this->m_used -= handle->aligned_size;

#ifdef _DEBUG
	memset(handle, static_cast<int>(0xEEEEEEEE), sizeof(BLOCK_INFO));
#endif // _DEBUG
}

//-------------------------------------------------------------------------

bool MemoryPage::IsAvailable() const
{
	return this->m_base_ptr != nullptr;
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::Shrink()
{
	// minimum size of memory committed
	if (this->m_now <= RLIB_COMMITSIZE) {
		return 0;
	} //if

	intptr_t bytesCleaned = 0;
	if (this->m_used == 0) {
		assert(this->m_last_ptr == nullptr);
		auto result = MemoryAllocator::Free(AppBase::GetCurrentProcess(),
											reinterpret_cast<LPBYTE>(this->m_base_ptr) + RLIB_COMMITSIZE,
											static_cast<SIZE_T>(this->m_now - RLIB_COMMITSIZE),
											RLIB_MEM_DECOMMIT);
		if (result != nullptr) {
			bytesCleaned = this->m_now - RLIB_COMMITSIZE;
			this->m_now  = RLIB_COMMITSIZE;
		} //if
	} else {
		assert(this->m_last_ptr != nullptr);
		LPBYTE    ptrBound  = reinterpret_cast<LPBYTE>(this->m_base_ptr) + this->m_now;
		LPBYTE    ptrLast   = reinterpret_cast<LPBYTE>(this->m_last_ptr) + this->m_last_ptr->aligned_size;
		ptrdiff_t bytesIdle = ptrBound - ptrLast;
		if (bytesIdle > PageSize) {
			// at least one page
			(bytesIdle /= PageSize) *= PageSize;

			auto result = MemoryAllocator::Free(AppBase::GetCurrentProcess(),
												ptrBound - bytesIdle,
												static_cast<SIZE_T>(bytesIdle),
												RLIB_MEM_DECOMMIT);
			if (result != nullptr) {
				bytesCleaned = bytesIdle;
				this->m_now -= bytesIdle;
			} //if
		} //if
	} //if

	return bytesCleaned;
}

//-------------------------------------------------------------------------

bool MemoryPage::ReSize(LPVOID lpuser, intptr_t new_size)
{
#ifdef _DEBUG
	if (this->Validate(lpuser) != PTR_IN) {
		trace(!"BAD POOL CALL");
		__rlib_debug_increase_count(this->m_failed_count);
		return false;
	} //if

	// adds valid mark at the tail of memory block for overflow checking
	intptr_t mark_index = new_size;
	new_size += sizeof(BLOCK_INFO::VALID_MARK);
#endif // _DEBUG

	intptr_t aligned_size = RLIB_ROUNDUP(RLIB_INTERNAL_BLOCK_SIZE + new_size);
    PBLOCK_INFO lpblock   = RLIB_INTERNAL_USER_TO_HANDLE(lpuser); 
    if (aligned_size < lpblock->aligned_size) // shrink
    {
		this->m_used -= (lpblock->aligned_size - aligned_size);
    }
    else if (aligned_size > lpblock->aligned_size) // extend
    {
		LPBYTE lpend = reinterpret_cast<LPBYTE>(lpblock) + aligned_size;
        if (lpblock->next == nullptr)
        {
            assert(lpblock == this->m_last_ptr);
			LPBYTE lpover = reinterpret_cast<LPBYTE>(this->m_base_ptr) + this->m_now;
			if (lpend > lpover && !this->extend(lpend - lpover)) {
				__rlib_debug_increase_count(this->m_failed_count);
				return false;
			} //if
        } else if (lpend > reinterpret_cast<LPBYTE>(lpblock->next)){
			__rlib_debug_increase_count(this->m_failed_count);
			return false;
        } //if
		
		// extended size
		this->m_used += aligned_size - lpblock->aligned_size;
    } //if

	lpblock->aligned_size = aligned_size;
#ifdef _DEBUG
	lpblock->valid_index  = mark_index;
	lpblock->set_valid_mark();
	__rlib_debug_increase_count(this->m_succeed_count);
#endif // _DEBUG

	return true;
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN void *MemoryPage::ReAlloc(LPVOID lpuser, 
											   intptr_t new_size /* = -1 */)
{
#ifdef _DEBUG
	if (this->Validate(lpuser) != PTR_IN) {
		trace(!"BAD POOL CALL");
		__rlib_debug_increase_count(this->m_failed_count);
		return nullptr;
	} //if
	LPCTSTR function = RLIB_INTERNAL_USER_TO_HANDLE(lpuser)->alloc_function;
	LPCTSTR 	file = RLIB_INTERNAL_USER_TO_HANDLE(lpuser)->alloc_file;
#endif // _DEBUG

	// aligned size without block info header size
    intptr_t old_size = RLIB_INTERNAL_USER_TO_HANDLE(lpuser)->aligned_size - RLIB_INTERNAL_BLOCK_SIZE;
	if (new_size < 0) {
		new_size = old_size;
	} else {
		new_size = RLIB_ROUNDUP(new_size);
	} //if

	LPVOID lpnew = this->AllocByte(new_size
								   RLIB_INTERNAL_DEBUG_PARAM_VALUE);
	if (lpnew != nullptr) {
		memcpy(lpnew, lpuser, static_cast<size_t>(RLIB_MIN(new_size, old_size)));

		this->Collect(lpuser);

		__rlib_debug_increase_count(this->m_succeed_count);
		return lpnew;
	} //if

	__rlib_debug_increase_count(this->m_failed_count);
    return nullptr;
}

//------------------------------------------------------------------------

MemoryPage::PTR_STATUS MemoryPage::Validate(LPCVOID p)
{
	if (p >= (reinterpret_cast<LPBYTE>(this->m_base_ptr) + this->m_now)) {
		return PTR_HIGH;
	} else if (p < this->m_base_ptr) {
		return PTR_LOW;
	} //if

    return PTR_IN;
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::GetSize(LPCVOID user)
{
#ifdef _DEBUG
	assert(this->Validate(user) == PTR_IN);
	return RLIB_INTERNAL_USER_TO_HANDLE(user)->valid_index; // original unaligned size
#else
	return RLIB_INTERNAL_USER_TO_HANDLE(user)->aligned_size - RLIB_INTERNAL_BLOCK_SIZE;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::GetMaxAllocSize()
{
    return this->m_reserved - (this->m_now + this->m_used);
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::GetUnusedSize()
{
	return this->m_now - this->m_used;
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::GetUsage()
{
    return this->m_used;
}

//-------------------------------------------------------------------------

intptr_t MemoryPage::GetMemorySize()
{
    return this->m_now;
}
