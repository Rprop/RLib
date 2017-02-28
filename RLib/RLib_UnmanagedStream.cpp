/********************************************************************
	Created:	2013/06/23  22:31
	Filename: 	RLib_Stream.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_UnmanagedStream.h"
#include "RLib_MemoryPool.h"
using namespace System::IO;

//-------------------------------------------------------------------------

intptr_t UnmanagedMemoryStream::Write(const void *buffer, intptr_t count)
{
	trace(buffer != nullptr);
	trace(count >= 0);

    // 可写入字节
    intptr_t max_write = this->MaxWriteSize;
	if (count > max_write) {
		trace(!"write overflow detected");
		count = max_write;
	} //if

	// 开始写入
	assert((reinterpret_cast<LPBYTE>(this->CurrentPtr) + count) <= (reinterpret_cast<LPBYTE>(this->m_buffer) + this->m_size));
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

void UnmanagedMemoryStream::ExChange(UnmanagedMemoryStream &obj)
{
	BYTE buffer[sizeof(UnmanagedMemoryStream)];
	memcpy(buffer, &obj, sizeof(UnmanagedMemoryStream));
	memcpy(&obj, this, sizeof(UnmanagedMemoryStream));
	memcpy(this, buffer, sizeof(UnmanagedMemoryStream));
}

//-------------------------------------------------------------------------

void UnmanagedMemoryStream::Move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes)
{
	this->m_length = stream_move(offsetFrom, offsetTo, bytes);
}