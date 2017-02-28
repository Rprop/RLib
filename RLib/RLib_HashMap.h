/********************************************************************
	Created:	2012/06/08  22:19
	Filename: 	RLib_HashMap.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HASHMAP
#define _USE_HASHMAP
#include "RLib_Utility.h"
#include "RLib_CollectionGeneric.h"

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			/// <summary>
			/// Represents a collection of key/value pairs that are organized based on the hash code of the key
			/// </summary>
			template<typename K, typename V, class hasher = IHash<K, __int64>, class allocator = IO::IAllocator, typename kdisposer = IDisposable<K>, typename vdisposer = IDisposable<V>>
			class HashMap/* : public Iterable*/
			{
			public:
				typedef typename hasher::Type H;

			protected:
				enum BUCKET_STATE : long
				{
					BUCKET_FREE = 0,
					BUCKET_BASE = 1
				};
				struct BUCKET
				{
					BUCKET_STATE state; // index of first entry, SLOT_BASE based, SLOT_FREE if unused
				};
				enum ENTRY_STATE : long
				{
					ENTRY_FREE = -1, // entry removed
					ENTRY_USED,
					ENTRY_INTERNAL_INIT, // entry val uninitiated
				};
				enum ENTRY_INDEX : long
				{
					ENTRY_LAST = -1, // no next entry
					ENTRY_BASE = 0
				};
				struct ENTRY
				{
					ENTRY_INDEX next; // index of next entry
					ENTRY_STATE state;
					H hash; // stores hash code
					K key;
					V val;
				
				public:
					ENTRY()  = delete;
					~ENTRY() = delete;
				};

			protected:
				ENTRY  *m_entries;
				BUCKET *m_buckets;
	#ifdef _DEBUG
				struct
				{
					ENTRY(*m_entries)[RLIB_DEFAULT_LENGTH];
					BUCKET(*m_buckets)[RLIB_DEFAULT_LENGTH];
				} *m_debugview;
				long    m_bk_hits;
	#endif // _DEBUG
				long    m_index;
				long    m_bk_count; // buckets count

			public:
				long  Capacity; // all entries count
				long  Length;

			public:
				/// <summary>
				/// Constructs an empty HashMap with the specified initial capacity and the default load factor (0.75)
				/// </summary>
				HashMap(long initialCapacity = RLIB_DEFAULT_CAPACITY, float loadFactor = 0.75f)
				{
	#ifdef _DEBUG
					this->m_debugview = reinterpret_cast<RLIB_TYPE(this->m_debugview)>(&this->m_entries);
					this->m_bk_hits   = 0;
	#endif // _DEBUG

					assert(loadFactor > 0.0f && loadFactor <= 1.0f);
					this->m_bk_count = Utility::round_up_pow_2(initialCapacity);
					this->m_buckets  = static_cast<BUCKET *>(allocator::allocateMemory(this->m_bk_count * RLIB_SIZEOF(BUCKET)
																					   RLIB_INTERNAL_DEBUG_PARAM_HERE));
					::memset(this->m_buckets, 0, sizeof(BUCKET) * this->m_bk_count);

					this->Length    = 0;
					this->m_index   = -1;
					this->Capacity  = static_cast<long>(this->m_bk_count * loadFactor); // threshold
					this->m_entries = static_cast<ENTRY *>(allocator::allocateMemory(this->Capacity * RLIB_SIZEOF(ENTRY)
																					 RLIB_INTERNAL_DEBUG_PARAM_HERE));
				}
				HashMap(const HashMap &hashmap)
				{
					Utility::copy(hashmap, *this);

#ifdef _DEBUG
					this->m_debugview = reinterpret_cast<RLIB_TYPE(this->m_debugview)>(&this->m_entries);
#endif // _DEBUG
					this->m_buckets = static_cast<BUCKET *>(allocator::allocateMemory(this->m_bk_count * RLIB_SIZEOF(BUCKET)
																					  RLIB_INTERNAL_DEBUG_PARAM_HERE));
					this->m_entries = static_cast<ENTRY *>(allocator::allocateMemory(this->Capacity * RLIB_SIZEOF(ENTRY)
																					 RLIB_INTERNAL_DEBUG_PARAM_HERE));
					::memcpy(this->m_buckets, hashmap.m_buckets, this->m_bk_count * sizeof(BUCKET));
					::memcpy(this->m_entries, hashmap.m_entries, this->Capacity * sizeof(ENTRY));
				}
				~HashMap()
				{
					this->Clear();
					allocator::freeMemory(this->m_entries);
					allocator::freeMemory(this->m_buckets);
				}
				RLIB_DECLARE_DYNCREATE;

			public:
				HashMap &operator = (const HashMap &hashmap)
				{
					this->~HashMap();
					RLIB_InitClass(this, HashMap(hashmap));
					return *this;
				}

			protected:
				long indexFor(const H &h) const
				{
					return static_cast<long>(h & (this->m_bk_count - 1));
				}
				long indexForKey(const K &key) const
				{
					auto h = hasher::GetHashCode(&key, sizeof(key));
					return this->indexFor(h);
				}
				void moveEntry(ENTRY *lpsrc, ENTRY *lpdst)
				{
					ENTRY_INDEX next = lpdst->next; // keeps the next field
					memcpy(lpdst, lpsrc, sizeof(ENTRY));
					lpdst->next = next;
				}
				void rehash(BUCKET *lpbuckets, long bk_count, ENTRY *lpentries)
				{
					BUCKET     *lpbucket;
					ENTRY      *lpentry;
					ENTRY_INDEX index;
					for (int i = 0; i < bk_count; ++i) {
						lpbucket = &lpbuckets[i];
						if (lpbucket->state != BUCKET_FREE) {
							index = static_cast<ENTRY_INDEX>(lpbucket->state - BUCKET_BASE);
							do {
								lpentry = &lpentries[index];
								if (lpentry->state != ENTRY_FREE) {
									bool isnew;
									this->moveEntry(lpentry, this->addEntry(lpentry->key, lpentry->hash, isnew));
									assert(isnew == true);
								} //if

								index = lpentry->next;
							} while (index != ENTRY_LAST);
						} //if
					} //for
				}
				void resize()
				{
					assert(this->m_bk_count < LONG_MAX / 2);
					auto bk_count      = this->m_bk_count * 2;
					auto lpbuckets     = this->m_buckets;
					auto prev_bk_count = this->m_bk_count;
					auto loadFactor    = static_cast<float>(this->Capacity) / prev_bk_count;
					auto lpentries     = this->m_entries;
					this->m_buckets    = static_cast<BUCKET *>(allocator::allocateMemory(bk_count * RLIB_SIZEOF(BUCKET)
																						 RLIB_INTERNAL_DEBUG_PARAM_HERE));
					this->m_bk_count   = bk_count;
					::memset(this->m_buckets, 0, sizeof(BUCKET) * bk_count);

					this->Length    = 0;
					this->m_index   = -1;
					this->Capacity  = static_cast<long>(bk_count * loadFactor);
					this->m_entries = static_cast<ENTRY *>(allocator::allocateMemory(this->Capacity * RLIB_SIZEOF(ENTRY)
																					 RLIB_INTERNAL_DEBUG_PARAM_HERE));

	#ifdef _DEBUG
					this->m_bk_hits = 0;
	#endif // _DEBUG
					this->rehash(lpbuckets, prev_bk_count, lpentries);
					allocator::freeMemory(lpbuckets);
					allocator::freeMemory(lpentries);

				}
				ENTRY_INDEX allocEntry()
				{
					return static_cast<ENTRY_INDEX>(++this->m_index);
				}
				ENTRY *addEntry(const K &key, const H &hash, _Out_ bool &isnew)
				{
					/*
					* Maps the specified key and returns the entry mapped.
					* The key-value may be uninitiated(state == ENTRY_INTERNAL_INIT).
					*/
					if (this->m_index + 1 >= this->Capacity) {
						this->resize();
					} //if
					auto lpbucket = &this->m_buckets[this->indexFor(hash)];
					ENTRY *lpentry;
					if (lpbucket->state != BUCKET_FREE) {
						assert(lpbucket->state > BUCKET_FREE);

						auto currentSlotIndex = static_cast<ENTRY_INDEX>(lpbucket->state - BUCKET_BASE);
						auto emptySlotIndex   = ENTRY_LAST; // We use the empty slot index to cache the first empty slot. We chose to reuse slots
															// create by remove.
						do {
							lpentry = &this->m_entries[currentSlotIndex];

							// We need to search this entire collision chain because we have to ensure that there are no 
							// duplicate entries.
							if (lpentry->state >= ENTRY_USED) {
								if (lpentry->key == key) {
									// the entry is in use and the key matched
									isnew = false;
									return lpentry;
								} //if
							} else {
								assert(emptySlotIndex == ENTRY_LAST);
								// Set emptySlot index to current bucket if it is the first available bucket that we have seen
								// that once contained an entry and also has had a collision.
								emptySlotIndex = currentSlotIndex;
							} //if

							currentSlotIndex = lpentry->next;
						} while (currentSlotIndex >= ENTRY_BASE);

						assert(currentSlotIndex == ENTRY_LAST);
						assert(lpentry != nullptr);

						// insert
						if (emptySlotIndex != ENTRY_LAST) {
							lpentry = &this->m_entries[emptySlotIndex];
						} else {
							currentSlotIndex = this->allocEntry();
							lpentry->next    = currentSlotIndex;
							lpentry          = &this->m_entries[currentSlotIndex];
							lpentry->next    = ENTRY_LAST;
						} //if
					} else {
						auto index      = this->allocEntry();
						lpbucket->state = static_cast<BUCKET_STATE>(index + BUCKET_BASE);
						lpentry         = &this->m_entries[index];
						lpentry->next   = ENTRY_LAST;
	#ifdef _DEBUG
						++this->m_bk_hits;
	#endif // _DEBUG
					} //if

					lpentry->state = ENTRY_INTERNAL_INIT;

					++this->Length;
					isnew = true;
					return lpentry;
				}
				ENTRY *addEntry(const K &key)
				{
					/*
					* Maps the specified key and returns the entry mapped.
					* The value associated with the specified key may be uninitiated(state == ENTRY_INTERNAL_INIT).
					*/
					auto hashcode = hasher::GetHashCode(&key, sizeof(key));
					auto isnew    = true;
					auto lpentry  = this->addEntry(key, hashcode, isnew);

					if (lpentry->state != ENTRY_INTERNAL_INIT) {
						assert(lpentry->key == key);
						assert(lpentry->hash == hashcode);
					} else if (isnew) {
						RLIB_InitClass(&lpentry->key, K(key));
						RLIB_InitClass(&lpentry->hash, H(hashcode));
					} //if

					return lpentry;
				}

			public:
				class ENTRY_REF : protected ENTRY
				{
					friend class HashMap;
				public:
					ENTRY_REF() = delete;
					~ENTRY_REF() = delete;
					ENTRY_REF(const ENTRY_REF &) = delete;
					ENTRY_REF &operator = (const ENTRY_REF &) = delete;

				public:
					operator V &() {
						return this->getValue();
					}
					operator const V &() const {
						return this->GetValue();
					}
					ENTRY_REF &operator = (const V &_val) {
						this->SetValue(_val);
						return *this;
					}
					V &GetValue() {
						assert(this->IsValidValue());
						return this->val;
					}
					const V &GetValue() const {
						assert(this->IsValidValue());
						return this->val;
					}
					ENTRY_REF *SetValue(const V &_val) {
						assert(!this->IsNull());
						if (this->state == ENTRY_INTERNAL_INIT) {
							RLIB_InitClass(&this->val, V(_val));
						} else {
							this->val = _val;
						} //if
						this->state = ENTRY_USED;
						return this;
					}
					RLIB_PROPERTY_GET_SET(V &Value, GetValue, SetValue);
// 					K &GetKey() {
// 						assert(!this->IsNull());
// 						return this->key;
// 					}
					const K &GetKey() const {
						assert(!this->IsNull());
						return this->key;
					}
					RLIB_PROPERTY_GET(const K &Key, GetKey);

				public:
					bool IsNull() const {
						return this == nullptr;
					}
					bool IsValidValue() const {
						return this != nullptr && this->state != ENTRY_INTERNAL_INIT;
					}
				};

			public:
				/// <summary>
				/// @see AddEntry
				/// </summary>
				ENTRY_REF *Add(const K &key, const V &val, bool update = true)
				{
					return this->AddEntry(key, val, update);
				}
				/// <summary>
				/// Associates the specified value with the specified key. 
				/// If the container previously contained a mapping for the key and update == true, the old value is replaced.
				/// </summary>
				ENTRY_REF *AddEntry(const K &key, const V &val, bool update = true)
				{
					auto lpentry = this->addEntry(key);
					if (lpentry->state == ENTRY_INTERNAL_INIT) {
						RLIB_InitClass(&lpentry->val, V(val));
					} else if (update) {
						lpentry->val = val;
					} //if
					lpentry->state = ENTRY_USED;
					return static_cast<ENTRY_REF *>(lpentry);
				}
				/// <summary>
				/// @see Emplace
				/// </summary>
				template<class... P>
				RLIB_INLINE ENTRY_REF *AddInPlace(const K &key, P &&... _args)
				{
					return this->Emplace(key, Standard::forward<P>(_args)...);
				}
				/// <summary>
				/// Associates the specified value with the specified key. 
				/// If the container previously contained a mapping for the key, the old value is replaced.
				/// </summary>
				template<class... P> 
				ENTRY_REF *Emplace(const K &key, P &&... _args)
				{
					auto lpentry = this->addEntry(key);
					if (lpentry->state != ENTRY_INTERNAL_INIT) {
						vdisposer::Dispose(&lpentry->val);
					} //if
					RLIB_InitClass(&lpentry->val, V(Standard::forward<P>(_args)...));
					lpentry->state = ENTRY_USED; 
					return static_cast<ENTRY_REF *>(lpentry);
				}
				/// <summary>
				/// Returns the entry pointer to which the specified key is mapped, or nullptr if there is no mapping for the key.
				/// </summary>
				ENTRY_REF *GetEntry(const K &key) const
				{
					auto lpbucket = &this->m_buckets[this->indexForKey(key)];
					if (lpbucket->state != BUCKET_FREE) {
						assert(lpbucket->state > BUCKET_FREE);

						auto currentSlotIndex = static_cast<ENTRY_INDEX>(lpbucket->state - BUCKET_BASE);
						do {
							auto lpentry = &this->m_entries[currentSlotIndex];

							// the entry is in use and the key matched
							if ((lpentry->state >= ENTRY_USED) && lpentry->key == key) {
								return static_cast<ENTRY_REF *>(lpentry);
							} //if

							currentSlotIndex = lpentry->next;
						} while (currentSlotIndex >= ENTRY_BASE);
					} //if

					return nullptr;
				}
				/// <summary>
				/// Removes the mapping for the specified key if present.
				/// </summary>
				bool Remove(const K &key)
				{
					auto lpentry = this->GetEntry(key);
					if (lpentry != nullptr) {
						this->RemoveEntry(lpentry);
						return true;
					} //if

					return false;
				}
				/// <summary>
				/// Removes the specified entry.
				/// </summary>
				void RemoveEntry(ENTRY *lpentry)
				{
					assert(lpentry->state != ENTRY_FREE);

					kdisposer::Dispose(&lpentry->key);
					if (lpentry->state == ENTRY_USED) {
						vdisposer::Dispose(&lpentry->val);
					} //if
					lpentry->state = ENTRY_FREE;

					--this->Length;
				}
				/// <summary>
				/// Removes all entries
				/// </summary>
				void Clear()
				{
					for (long k = 0; k <= this->m_index && this->Length > 0; ++k) {
						auto lpentry = &this->m_entries[k];
						if (lpentry->state != ENTRY_FREE) {
							this->RemoveEntry(lpentry);
						} //if
					} //for
					::memset(this->m_buckets, 0, sizeof(BUCKET) * this->m_bk_count);
					assert(this->Length == 0);
					this->m_index = -1;
				}
				/// <summary>
				/// Returns true if this container contains a mapping for the specified key.
				/// </summary>
				bool ContainsKey(const K &key) const
				{
					return this->GetEntry(key) != nullptr;
				}

			public:
				ENTRY_REF &operator [] (const K &key)
				{
					static_assert(sizeof(ENTRY_REF) == sizeof(ENTRY), "BOOM");
					return *static_cast<ENTRY_REF *>(this->addEntry(key));
				}
				const ENTRY_REF &operator [] (const K &key) const
				{
					// this const method does not add entry
					static_assert(sizeof(ENTRY_REF) == sizeof(ENTRY), "BOOM");
					auto lpentry = static_cast<ENTRY_REF *>(this->GetEntry(key));
					assert(lpentry != nullptr);
					return *lpentry;
				}

			public: // iteration
					/// <summary>
					/// Returns an iterator to the first element of the container.
					/// If the container is empty or nothing, the returned iterator will be equal to end()
					/// </summary>
				ENTRY_REF *begin() /* noexcept */ {
					return this != nullptr && this->m_index >= 0 ? static_cast<ENTRY_REF *>(this->m_entries) : nullptr;
				}
				const ENTRY_REF *begin() const  /* noexcept */ {
					return this != nullptr && this->m_index >= 0 ? static_cast<ENTRY_REF *>(this->m_entries) : nullptr;
				}
				/// <summary>
				/// Returns an iterator to the element following the last element of the container.
				/// This element acts as a placeholder; attempting to access it results in undefined behavior.
				/// </summary>
				ENTRY_REF *end() /* noexcept */ {
					return this != nullptr && this->m_index >= 0 ? static_cast<ENTRY_REF *>(&this->m_entries[this->m_index + 1]) : nullptr;
				}
				const ENTRY_REF *end() const /* noexcept */ {
					return this != nullptr && this->m_index >= 0 ? static_cast<ENTRY_REF *>(&this->m_entries[this->m_index + 1]) : nullptr;
				}
				/// <summary>
				/// Performs the given action for each entry in this map until all entries have been processed or the action return false.
				/// </summary>
				template<typename T>
				void forEach(bool(*callback)(ENTRY_REF &, long, T), T lpuserdata)
				{
					BUCKET     *lpbucket;
					ENTRY      *lpentry;
					ENTRY_INDEX index;
					for (long i = 0; i < this->m_bk_count && this->Length > 0; ++i) {
						lpbucket = &this->m_buckets[i];
						if (lpbucket->state != BUCKET_FREE) {
							index = static_cast<ENTRY_INDEX>(lpbucket->state - BUCKET_BASE);
							do {
								lpentry = &this->m_entries[index];
								if (lpentry->state != ENTRY_FREE && !callback(*static_cast<ENTRY_REF *>(lpentry), i, lpuserdata)) {
									return;
								} //if

								index = lpentry->next;
							} while (index != ENTRY_LAST);
						} //if
					} //for
				}
				/// <summary>
				/// Performs the given action for each entry in this map until all entries have been processed or the action return false.
				/// </summary>
				void forEach(bool(*callback)(ENTRY_REF &, long))
				{
					this->forEach<RLIB_TYPE(callback)>([](ENTRY_REF &entry, long index, RLIB_TYPE(callback) callback) {
						return callback(entry, index);
					}, callback);
				}
			};
		}
	}
}

//-------------------------------------------------------------------------
#define foreachHashMap(p, map)  for(auto p = map.Start(); p != NULL; p = map.Next(p))

#endif // _USE_HASHMAP
