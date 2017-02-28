/********************************************************************
	Created:	2013/06/23  22:31
	Filename: 	RLib_Stream.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Stream.h"
#include "RLib_MemoryStream.h"

using namespace System::IO;

//-------------------------------------------------------------------------

void Stream::CopyTo(Stream *destination, 
					intptr_t bufferSize /* = RLIB_DEFAULT_BUFFER_SIZE */) const
{
	assert(bufferSize > 0);

	LPBYTE safe_buffer = reinterpret_cast<LPBYTE>(RLIB_GlobalAlloc(bufferSize));
	if (safe_buffer != nullptr) {
		intptr_t count;
		while ((count = this->Read(safe_buffer, bufferSize)) > 0) {
			assert(count <= bufferSize);
			destination->Write(safe_buffer, count);
		}

		RLIB_GlobalCollect(safe_buffer);
	} //if
}

//-------------------------------------------------------------------------

StreamReader::StreamReader(const Stream *input, intptr_t size/* = -1 */)
{
	if (input->InheritFromMemoryStream()) {
		RLIB_InitClass(this, StreamReader(static_cast<const MemoryStream *>(input), size));
		return;
	} //if

	if (size < 0) size = input->MaxReadSize;
	assert(size >= 0);

	this->BufferedData = RLIB_GlobalAllocAny(unsigned char *, size);
	this->Capacity =
		this->BufferedData != nullptr ? input->Read(this->BufferedData, size) : 0;
	this->CanWrite = true/*this->Capacity != 0*/;
}

//-------------------------------------------------------------------------

StreamReader::StreamReader(const MemoryStream *input, intptr_t size /* = -1 */)
{
	if (size < 0) size = input->MaxReadSize;
	assert(size >= 0);

	this->BufferedData = reinterpret_cast<LPBYTE>(input->CurrentPtr);
	this->Capacity     = size;
	this->CanWrite     = false;
}

//-------------------------------------------------------------------------

StreamReader::~StreamReader()
{
	if (this->CanWrite && this->BufferedData != nullptr) {
		RLIB_GlobalCollect(this->BufferedData);
	} //if

#ifdef _DEBUG
	this->BufferedData = nullptr;
#endif // _DEBUG
}