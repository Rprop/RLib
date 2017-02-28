/********************************************************************
	Created:	2016/07/11  9:46
	Filename: 	RLib_BitArray.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_CollectionGeneric.h"

namespace System
{
	namespace Collections
	{
		/// <summary>
		/// A vector of bits.  Use this to store bits efficiently, without having to do bit 
		/// shifting yourself
		/// </summary>
		class RLIB_API BitArray
		{
		private:
			int      *m_array;
			intptr_t  m_array_length;
			intptr_t  m_length;
			intptr_t _version;

			const intptr_t _ShrinkThreshold = 256;
		private:
			// XPerY=n means that n Xs can be stored in 1 Y. 
			const intptr_t BytesPerInt32 = sizeof(int);
			const intptr_t BitsPerByte   = 8;
			const intptr_t BitsPerInt32  = BytesPerInt32 * BitsPerByte;

			/// <summary>
			/// Used for conversion between different representations of bit array. 
			/// Returns (n+(div-1))/div, rearranged to avoid arithmetic overflow. 
			/// For example, in the bit to int case, the straightforward calc would 
			/// be (n+31)/32, but that would cause overflow. So instead it's 
			/// rearranged to ((n-1)/32) + 1, with special casing for 0.
			/// 
			/// Usage:
			/// GetArrayLength(77, BitsPerInt32): returns how many ints must be 
			/// allocated to store 77 bits.
			/// </summary>
			/// <param name="n"></param>
			/// <param name="div">use a conversion constant, e.g. BytesPerInt32 to get
			/// how many ints are required to store n bytes</param>
			/// <returns></returns>
			static intptr_t GetArrayLength(intptr_t n, intptr_t div);
		public:
			/// <summary>
			/// Allocates space to hold length bit values. All of the values in the bit array are set to false.
			/// </summary>
			BitArray(intptr_t length);
			/// <summary>
			/// Allocates space to hold length bit values. All of the values in the bit array are set to defaultValue.
			/// </summary>
			BitArray(intptr_t length, bool defaultValue);
			/// <summary>
			/// Allocates space to hold the bit values in bytes. bytes[0] represents
			/// bits 0 - 7, bytes[1] represents bits 8 - 15, etc.The LSB of each byte
			/// represents the lowest index value; bytes[0] & 1 represents bit 0,
			/// bytes[0] & 2 represents bit 1, bytes[0] & 4 represents bit 2, etc.
			/// </summary>
			BitArray(const unsigned char bytes[], intptr_t bytes_length);
			/// <summary>
			/// Allocates space to hold the bit values in bytes. values[0] represents
			/// bits 0, values[1] represents bits 1, etc.
			/// </summary>
			BitArray(const bool values[], intptr_t values_length);
			/// <summary>
			/// Allocates space to hold the bit values in values. values[0] represents
			/// bits 0 - 31, values[1] represents bits 32 - 63, etc.The LSB of each
			/// integer represents the lowest index value; values[0] & 1 represents bit
			/// 0, values[0] & 2 represents bit 1, values[0] & 4 represents bit 2, etc.
			/// </summary>
			BitArray(const int values[], intptr_t values_length);
			/// <summary>
			/// Allocates a new BitArray with the same length and bit values as bits.
			/// </summary>
			BitArray(const BitArray &bits);
			BitArray(const BitArray &&bits);
			~BitArray();
		public:
			BitArray &operator = (const BitArray &bits);
			BitArray &operator = (const BitArray &&bits);
			BitArray &operator &= (const BitArray *value);
			BitArray &operator |= (const BitArray *value);
			BitArray &operator ^= (const BitArray *value);
			BitArray &operator ~ ();
			/// <summary>
			/// Returns a reference to the current instance ANDed with value.
			/// </summary>
			BitArray &And(const BitArray *value);
			/// <summary>
			/// Returns a reference to the current instance ORed with value.
			/// </summary>
			BitArray &Or(const BitArray *value);
			/// <summary>
			/// Returns a reference to the current instance XORed with value.
			/// </summary>
			BitArray &Xor(const BitArray *value);
			/// <summary>
			/// Inverts all the bit values. On/true bit values are converted to
			/// off / false.Off / false bit values are turned on / true.The current instance
			/// is updated and returned.
			/// </summary>
			BitArray &Not();
			/// <summary>
			/// Returns the bit value at position index.
			/// </summary>
			bool Get(intptr_t index) const;
			/// <summary>
			/// Sets the bit value at position index to value.
			/// </summary>
			void Set(intptr_t index, bool value);
			/// <summary>
			/// Sets all the bit values to value.
			/// </summary>
			void SetAll(bool value);
			/// <summary>
			/// Gets the number of elements in the BitArray.
			/// </summary>
			intptr_t GetLength() const;
			/// <summary>
			/// Sets the number of elements in the BitArray.
			/// </summary>
			void SetLength(intptr_t length);
			/// <summary>
			/// Gets or sets the number of elements in the BitArray.
			/// </summary>
			RLIB_PROPERTY_GET_SET(intptr_t Length, GetLength, SetLength);
		public:
			struct Indexer
			{
				BitArray *_inst;
				intptr_t  _index;
			public:
				operator bool() const {
					return _inst->Get(_index);
				}
				Indexer & operator = (bool value) {
					_inst->Set(_index, value);
					return *this;
				}
			};
			Indexer operator [] (intptr_t index) {
				return Indexer{ this, index };
			}
			bool operator [] (intptr_t index) const {
				return this->Get(index);
			}
		};
	}
}