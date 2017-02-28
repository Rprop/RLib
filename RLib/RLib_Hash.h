/********************************************************************
	Created:	2015/09/27  16:39
	Filename: 	RLib_Hash.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

namespace System
{
	namespace Generic
	{
		/// <summary>
		/// Provides several static methods for hashing
		/// </summary>
		namespace Hash
		{
			
		};
	}
	namespace Collections
	{
		namespace Generic 
		{
			/// <summary>
			/// Defines a method that a type implements to compute hash code
			/// </summary>
			template <typename K, typename H> class IHash
			{
			public:
				typedef H(*__HashDelegate)(const K *, size_t);
				typedef __HashDelegate Delegate;
				typedef K KeyType;
				typedef H Type;
				/// <summary>
				/// default string hasher
				/// </summary>
				static Type GetHashCode(const String *lpkey, size_t/* size*/)
				{
					Type hashCode = 0;
					for (auto i = lpkey->Length - 1; i >= 0; i--) {
						hashCode += (hashCode << 7) ^ lpkey->GetAt(i);
					}
					hashCode -= hashCode >> 17;
					hashCode -= hashCode >> 11;
					hashCode -= hashCode >> 5;
					return hashCode;
				}
				/// <summary>
				/// default LPCTSTR hasher
				/// </summary>
				static Type GetHashCode(const LPCTSTR *lpkey, size_t/* size*/)
				{
					Type hashCode = 0;
					for (auto i = _tcslen(lpkey) - 1; i >= 0; i--) {
						hashCode += (hashCode << 7) ^ lpkey[i];
					}
					hashCode -= hashCode >> 17;
					hashCode -= hashCode >> 11;
					hashCode -= hashCode >> 5;
					return hashCode;
				}
				/// <summary>
				/// default hasher
				/// </summary>
				static Type GetHashCode(const void *lpkey, size_t size)
				{
					auto   _First = static_cast<const unsigned char *>(lpkey);
					size_t _Count = size/*of(K)*/;

					// FNV-1a hash function for bytes in [_First, _First + _Count)
#if defined(_WIN64)
					static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
					const size_t _FNV_offset_basis = 14695981039346656037ULL;
					const size_t _FNV_prime        = 1099511628211ULL;

#else /* defined(_WIN64) */
					static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
					const size_t _FNV_offset_basis = 2166136261U;
					const size_t _FNV_prime        = 16777619U;
#endif /* defined(_WIN64) */

					size_t _Val = _FNV_offset_basis;
					for (size_t _Next = 0; _Next < _Count; ++_Next) {	// fold in another byte
						_Val ^= static_cast<size_t>(_First[_Next]);
						_Val *= _FNV_prime;
					}
					return static_cast<Type>(_Val);
				}
			};
		}
	}
};