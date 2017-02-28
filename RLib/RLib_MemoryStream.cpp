/********************************************************************
	Created:	2013/06/23  22:31
	Filename: 	RLib_Stream.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_MemoryStream.h"
#include "RLib_MemoryPool.h"
using namespace System::IO;

//-------------------------------------------------------------------------

intptr_t MemoryStream::stream_move(intptr_t offsetTo, intptr_t offsetFrom, intptr_t bytes)
{
	LPBYTE   pdata  = reinterpret_cast<LPBYTE>(this->ObjectData);
	intptr_t size   = this->Size;
	intptr_t length = this->Length;
	if ((offsetFrom + bytes) <= length) {
		if ((offsetTo + bytes) <= length) {
			memmove(&pdata[offsetTo], &pdata[offsetFrom], static_cast<size_t>(bytes));
		} else if ((offsetTo + bytes) <= size) {
			memmove(&pdata[offsetTo], &pdata[offsetFrom], static_cast<size_t>(bytes));
			length += (offsetTo + bytes) - length;
		} //if
	} //if
	return length;
}

//-------------------------------------------------------------------------

intptr_t MemoryStream::Read(void *buffer, intptr_t count) const
{
	trace(buffer != nullptr);
	trace(count >= 0);

    intptr_t max_read = this->MaxReadSize;
	if (count > max_read) {
		count = max_read;
	} //if

    memcpy(buffer, this->CurrentPtr, static_cast<size_t>(count));

    const_cast<MemoryStream *>(this)->Position += count;

    return count;
}

//-------------------------------------------------------------------------

void MemoryStream::SetPos(intptr_t Pos)
{
	assert(this->m_length <= this->m_size || this->m_size == 0);
	if (Pos <= 0) {
		Pos = 0;
	} else if (Pos > this->m_length) {
		Pos = this->m_length;
	} //if
	this->m_pos = Pos;
}