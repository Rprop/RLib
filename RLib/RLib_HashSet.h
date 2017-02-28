/********************************************************************
	Created:	2016/08/12  14:34
	Filename: 	RLib_HashSet.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_HashMap.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			template<typename K, class hasher = IHash<K, __int64>, class allocator = IO::IAllocator, typename kdisposer = IDisposable<K>>
			class HashSet : HashMap<K, intptr_t, hasher, allocator, kdisposer>
			{
			public:
				using HashMap::HashMap;

			public:
				/// <summary>
				/// Returns true if the set contains the specified element.
				/// </summary>
				bool Contains(const K &element) const
				{
					return this->ContainsKey(element);
				}
				/// <summary>
				/// Adds the specified element to the set.
				/// </summary>
				void Add(const K &element)
				{
					this->addEntry(element);
				}
				/// <summary>
				/// Removes the specified element from the set.
				/// </summary>
				bool Remove(const K &element)
				{
					return HashMap::Remove(element);
				}

			public: // iteration
				struct Iterator
				{
					 const ENTRY_REF *lpentry;
				
				public:
					Iterator &operator ++() {
						++this->lpentry;
						return *this;
					}
					bool operator != (const Iterator &itor) const {
						return this->lpentry != itor.lpentry;
					}
					const K &operator * () const {
						return this->lpentry->Key;
					}
				};
				/// <summary>
				/// Returns an iterator to the first element of the container.
				/// If the container is empty or nothing, the returned iterator will be equal to end()
				/// </summary>
				Iterator begin() const  /* noexcept */ {
					return { HashMap::begin() };
				}
				/// <summary>
				/// Returns an iterator to the element following the last element of the container.
				/// This element acts as a placeholder; attempting to access it results in undefined behavior.
				/// </summary>
				Iterator end() const /* noexcept */ {
					return { HashMap::end() };
				}
			};
		}
	}
}