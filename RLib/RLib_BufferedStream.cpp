/********************************************************************
	Created:	2013/06/23  22:31
	Filename: 	RLib_Stream.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_BufferedStream.h"
#include "RLib_MemoryPool.h"
#include "RLib_Utility.h"
using namespace System::IO;

//-------------------------------------------------------------------------

BufferedStream::BufferedStream(intptr_t size /* = RLIB_DEFAULT_BUFFER_SIZE */)
{
	this->m_pos    = 0;
	this->m_length = 0;
    this->m_buffer = RLIB_GlobalAlloc(size);
	this->m_size   = this->m_buffer != nullptr ? size : 0;
	assert(this->m_buffer != nullptr);
}

//-------------------------------------------------------------------------

BufferedStream::~BufferedStream()
{
    this->Close();
}

//-------------------------------------------------------------------------

void BufferedStream::Close()
{
	if (this->m_buffer != nullptr) {
		RLIB_GlobalCollect(this->m_buffer);
#ifdef _DEBUG
		memset(this, 0, sizeof(*this));
#else
		this->m_buffer = nullptr;
#endif // _DEBUG
	} //if
}

//-------------------------------------------------------------------------

void BufferedStream::SetLength(intptr_t length)
{
	assert(length >= 0);

	if (length == 0) {
		this->m_length = this->m_pos = 0;
	} else if (length > this->m_size) {
        this->m_length = this->m_size;
    } else {
		this->m_length = length;

		if (this->m_pos > this->m_length) {
			this->m_pos = this->m_length;
		} //if
	} //if
}

//-------------------------------------------------------------------------

void BufferedStream::ExChange(BufferedStream &obj)
{
	Utility::swap(*this, obj);
}

//-------------------------------------------------------------------------

intptr_t BufferedStream::Write(const void *buffer, intptr_t count)
{
	assert(buffer != nullptr);
	assert(count >= 0);

    // maximum not writeable count
	intptr_t max_not_writeable_count = count - this->MaxWriteSize;
    if (max_not_writeable_count > 0)
    {
		max_not_writeable_count = RLIB_MAX(max_not_writeable_count, RLIB_DEFAULT_BUFFER_SIZE);
		if (!this->EnsureCapacity(this->Capacity + max_not_writeable_count)) {
			trace(!"failed to alloc memory!");
			return -1;
        } //if
    } //if

    // 开始写入
	assert((static_cast<LPBYTE>(this->CurrentPtr) + count) <= (static_cast<LPBYTE>(this->m_buffer) + this->m_size));
    memcpy(this->CurrentPtr, buffer, static_cast<size_t>(count));

	// 移动流指针位置
	this->m_pos += count;

	// 修改流长度(注意需要考虑覆写问题)
	if (this->m_pos > this->m_length) {
		this->m_length = this->m_pos;
	} //if
  
	return count;
}

//-------------------------------------------------------------------------

bool BufferedStream::EnsureCapacity(intptr_t value)
{
	assert(value > this->Capacity);
	void *ptr = RLIB_GlobalReAlloc(this->m_buffer, value);
	if (ptr != nullptr) {
		this->m_buffer = ptr;
		this->m_size   = value;
		return true;
	} //if

	return false;
}

//-------------------------------------------------------------------------

void BufferedStream::Move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes)
{
	if (offsetFrom + bytes <= this->Length) {
		// maximum not writeable count
		intptr_t max_not_writeable_count = (offsetTo + bytes) - this->Capacity;
		if (max_not_writeable_count > 0) {
			// use (offsetTo + bytes) directly...
			if (!this->EnsureCapacity(this->Capacity + max_not_writeable_count)) {
				trace(!"insufficient memory, move failed");
				return;
			} //if
		} //if
		this->m_length = stream_move(offsetTo, offsetFrom, bytes);
	} //if
}