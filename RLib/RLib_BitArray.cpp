/********************************************************************
	Created:	2016/07/11  9:46
	Filename: 	RLib_BitArray.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_BitArray.h"
using namespace System::Collections;

#ifndef INT_MAX
# define INT_MAX 2147483647 // maximum (signed) int value
#endif // !INT_MAX

//-------------------------------------------------------------------------

intptr_t BitArray::GetArrayLength(intptr_t n, intptr_t div)
{
	assert(div > 0 && "div arg must be greater than 0");
	return n > 0 ? (((n - 1) / div) + 1) : 0;
}

//-------------------------------------------------------------------------

BitArray::BitArray(intptr_t length) : BitArray(length, false)
{
}

//-------------------------------------------------------------------------

BitArray::BitArray(intptr_t length, bool defaultValue)
{
	this->m_array_length = this->GetArrayLength(length, BitsPerInt32);
	this->m_array        = RLIB_GlobalAllocObj(int, this->m_array_length);
	this->m_length       = length;
	this->_version       = 0;

	memset(this->m_array, 
		   defaultValue ? static_cast<int>(0xffffffff) : 0, 
		   static_cast<size_t>(this->m_array_length * BytesPerInt32));			
}

//-------------------------------------------------------------------------

BitArray::BitArray(const unsigned char bytes[], intptr_t bytes_length)
{
	// this value is chosen to prevent overflow when computing m_length.
	// m_length is of type int32 and is exposed as a property, so 
	// type of m_length can't be changed to accommodate.
	assert(bytes_length <= INT_MAX / BitsPerByte);

	this->m_length       = bytes_length * BitsPerByte;
	this->m_array_length = this->GetArrayLength(this->m_length, BitsPerInt32);
	this->m_array        = RLIB_GlobalAllocObj(int, this->m_array_length);
	this->_version       = 0;

	intptr_t i = 0, j = 0;
	while (bytes_length - j >= 4) {
		m_array[i++] = (bytes[j] & 0xff) |
			((bytes[j + 1] & 0xff) << 8) |
			((bytes[j + 2] & 0xff) << 16) |
			((bytes[j + 3] & 0xff) << 24);
		j += 4;
	}

	assert(bytes_length - j >= 0 && "BitArray byteLength problem");
	assert(bytes_length - j < 4 && "BitArray byteLength problem #2");

	switch (bytes_length - j) 
	{
	case 3:
		this->m_array[i] = ((bytes[j + 2] & 0xff) << 16);
		// goto case 2;
		// fall through
	case 2:
		this->m_array[i] |= ((bytes[j + 1] & 0xff) << 8);
		// goto case 1;
		// fall through
	case 1:
		this->m_array[i] |= (bytes[j] & 0xff);
		break;
	}			
}

//-------------------------------------------------------------------------

BitArray::BitArray(const bool values[], intptr_t values_length)
{
	this->m_array_length = this->GetArrayLength(values_length, BitsPerInt32);
	this->m_array        = RLIB_GlobalAllocObj(int, this->m_array_length);
	this->m_length       = values_length;
	this->_version       = 0;

	for (intptr_t i = 0; i < values_length; ++i) {
		if (values[i])
			this->m_array[i / 32] |= (1 << (i % 32));
	}
}

//-------------------------------------------------------------------------

BitArray::BitArray(const int values[], intptr_t values_length)
{
	// this value is chosen to prevent overflow when computing m_length
	assert(values_length <= INT_MAX / BitsPerInt32);

	this->m_length       = values_length * BitsPerInt32;
	this->m_array_length = this->GetArrayLength(this->m_length, BitsPerInt32);
	this->m_array        = RLIB_GlobalAllocObj(int, this->m_array_length);
	this->_version       = 0;

	memcpy(this->m_array, values, 
		   static_cast<size_t>(values_length * BytesPerInt32));
}

//-------------------------------------------------------------------------

BitArray::BitArray(const BitArray &bits)
{
	this->m_length       = bits.m_length;
	this->m_array_length = bits.m_array_length;
	this->m_array        = RLIB_GlobalAllocObj(int, this->m_array_length);
	this->_version       = bits._version;

	memcpy(this->m_array, bits.m_array, 
		   static_cast<size_t>(bits.m_array_length * BytesPerInt32));
}

//-------------------------------------------------------------------------

BitArray::BitArray(const BitArray &&bits)
{
	this->m_length       = bits.m_length;
	this->m_array_length = bits.m_array_length;
	this->m_array        = bits.m_array;
	this->_version       = bits._version;

	const_cast<BitArray *>(&bits)->m_array = nullptr;
}

//-------------------------------------------------------------------------

BitArray::~BitArray()
{
	if (this->m_array != nullptr) RLIB_GlobalCollect(this->m_array);
#ifdef _DEBUG
	this->m_length = this->m_array_length = 0;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator = (const BitArray &bits)
{
	if (bits.m_array_length > this->m_array_length) {
		this->~BitArray();
		RLIB_InitClass(this, BitArray(bits));
	} else {
		// avoid reallocating memory, but possible memory waste if this->m_array_length >> bits.m_array_length
		this->m_length       = bits.m_length;
		this->m_array_length = bits.m_array_length;
		this->_version       = bits._version;
		memcpy(this->m_array, bits.m_array, 
			   static_cast<size_t>(bits.m_array_length * BytesPerInt32));
	} //if
	return *this;
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator = (const BitArray &&bits)
{
	this->~BitArray();
	RLIB_InitClass(this, BitArray(bits));
	return *this;
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator &= (const BitArray *value)
{
	return this->And(value);
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator |= (const BitArray *value)
{
	return this->Or(value);
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator ^= (const BitArray *value)
{
	return this->Xor(value);
}

//-------------------------------------------------------------------------

BitArray &BitArray::operator ~ ()
{
	return this->Not();
}

//-------------------------------------------------------------------------

BitArray &BitArray::And(const BitArray *value)
{
	assert(value != nullptr && this->m_length == value->m_length);

	for (intptr_t i = 0; i < this->m_array_length; ++i) {
		this->m_array[i] &= value->m_array[i];
	}

	++this->_version;
	return *this;
}

//-------------------------------------------------------------------------

BitArray &BitArray::Or(const BitArray *value)
{
	assert(value != nullptr && this->m_length == value->m_length);

	for (intptr_t i = 0; i < this->m_array_length; ++i) {
		this->m_array[i] |= value->m_array[i];
	}

	++this->_version;
	return *this;
}

//-------------------------------------------------------------------------

BitArray &BitArray::Xor(const BitArray *value)
{
	assert(value != nullptr && this->m_length == value->m_length);

	for (intptr_t i = 0; i < this->m_array_length; ++i) {
		this->m_array[i] ^= value->m_array[i];
	}

	++this->_version;
	return *this;
}

//-------------------------------------------------------------------------

BitArray &BitArray::Not()
{
	for (int i = 0; i < this->m_array_length; ++i) {
		this->m_array[i] = ~this->m_array[i];
	}

	++this->_version;
	return *this;
}

//-------------------------------------------------------------------------

bool BitArray::Get(intptr_t index) const
{
	assert(index >= 0 && index < Length);

	return (this->m_array[index / 32] & (1 << (index % 32))) != 0;
}

//-------------------------------------------------------------------------

void BitArray::Set(intptr_t index, bool value)
{
	assert(index >= 0 && index < Length);

	if (value) {
		this->m_array[index / 32] |= (1 << (index % 32));
	} else {
		this->m_array[index / 32] &= ~(1 << (index % 32));
	} //if

	++this->_version;
}

//-------------------------------------------------------------------------

void BitArray::SetAll(bool value)
{
	memset(this->m_array, value ? static_cast<int>(0xffffffff) : 0, 
		   static_cast<size_t>(this->m_array_length * BytesPerInt32));

	++this->_version;
}

//-------------------------------------------------------------------------

intptr_t BitArray::GetLength() const
{
	return this->m_length;
}

//-------------------------------------------------------------------------

void BitArray::SetLength(intptr_t length)
{
	assert(length >= 0);

	intptr_t oldints = this->m_array_length;
	intptr_t newints = GetArrayLength(length, BitsPerInt32);
	if (newints > this->m_array_length || newints + _ShrinkThreshold < this->m_array_length) {
		// grow or shrink (if wasting more than _ShrinkThreshold ints)
		this->m_array        = static_cast<int *>(RLIB_GlobalReAlloc(this->m_array, newints * BytesPerInt32));
		this->m_array_length = newints;
	} //if

	if (length > this->m_length) {
		assert(this->m_array_length >= oldints);
		// clear high bit values in the last int
		intptr_t last = oldints - 1;
		intptr_t bits = this->m_length % 32;
		if (bits > 0) {
			this->m_array[last] &= (1 << bits) - 1;
		} //if

		// clear remaining int values
		memset(this->m_array + oldints, 0, 
			   static_cast<size_t>(newints - oldints));
	}

	++this->_version;
	this->m_length = length;
}