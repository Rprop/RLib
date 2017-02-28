/********************************************************************
	Created:	2012/02/06  19:40
	Filename: 	RLib_Array.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_ARRAY
#define _USE_ARRAY
#include "RLib_Utility.h"
#include "RLib_CollectionGeneric.h"
#include "RLib_Search.h"
#include "RLib_Sort.h"

//#define RLIB_ARRAY_SYNC   
#ifdef RLIB_ARRAY_SYNC
# define RLIB_ARRAY_LOCK   this->SyncRoot.Enter()
# define RLIB_ARRAY_UNLOCK this->SyncRoot.Exit()
#else
# define RLIB_ARRAY_LOCK
# define RLIB_ARRAY_UNLOCK
#endif // RLIB_ARRAY_SYNC

//-------------------------------------------------------------------------

namespace System
{
	namespace Collections
	{
		/// <summary>
		/// Contains interfaces and classes that define generic collections, 
		/// which allow users to create strongly typed collections that provide better type safety and performance than non-generic strongly typed collections
		/// </summary>
		namespace Generic
		{
			/// <summary>
			/// Provides methods for creating, manipulating, searching, and sorting arrays, 
			/// thereby serving as the base class for all arrays
			/// </summary>
			template <class R, typename disposer = IDisposable<R>, class allocator = IO::IAllocator>
			class Array/* : public Iterable*/
			{
			protected:
#ifdef _DEBUG
				typedef R R_ARR[RLIB_DEFAULT_LENGTH];
				union
				{
					R_ARR *pDebugView;
					R     *m_pItems; 
				};
#else
				R *m_pItems;
#endif // _DEBUG
			
			private:
				RLIB_INLINE void _copy_forward(intptr_t begin)
				{
					if ((this->Length - begin) <= 0) return;
					memcpy(&this->m_pItems[begin], &this->m_pItems[begin + 1],
						   sizeof(R) * (this->Length - begin));
				}
				RLIB_INLINE void _copy_backward(intptr_t begin)
				{
					if ((this->Length - begin) <= 0) return;
					memcpy(&this->m_pItems[begin + 1], &this->m_pItems[begin],
						   sizeof(R) * (this->Length - begin));
				}
				void _init(intptr_t length)
				{
					this->m_pItems  = static_cast<R *>(allocator::allocateMemory(RLIB_SIZEOF(R) * length
																				 RLIB_INTERNAL_DEBUG_PARAM_HERE));			
					this->MaxLength = length;
					this->Length    = 0;
					
					assert(this->m_pItems != nullptr);
				}
				void _expand(intptr_t length)
				{
					_ensureCapacity(this->MaxLength + length);
				}
				void _ensureCapacity(intptr_t value)
				{
					/*
					 *	Ensures that the capacity of this instance is at least the specified value
					 */
					auto cache_ptr = allocator::reallocateMemory(this->m_pItems,
																 RLIB_SIZEOF(R) * value);
					if (cache_ptr == nullptr) {
						trace(!"reallocateMemory failed");
						return;
					} //if
					this->m_pItems  = static_cast<R *>(cache_ptr);
					this->MaxLength = value;
				}
				template<class... P> intptr_t _add_nolock(P &&... args)
				{
					// @see Insert
					RLIB_InitClass(&this->m_pItems[this->Length], R(Standard::forward<P>(args)...));
					return this->Length++; // !++
				}
				template<class... P> intptr_t _add_sequence_nolock(const R &item)
				{
					return this->_add_nolock(item);
				}
				template<class... P> intptr_t _add_sequence_nolock(const R &item, const P& ... args)
				{
					intptr_t index = this->_add_sequence_nolock(item);
					// loop unfold
					this->_add_sequence_nolock(args...);
					
					return index;
				}
			
			public:
				/// <summary>
				/// Gets the total number of elements of the Array&lt;Of R&gt;
				/// </summary>
				intptr_t Length; 
				/// <summary>
				/// Gets the maximum capacity of the Array&lt;Of R&gt; without reallocating memory
				/// </summary>
				intptr_t MaxLength; 
#ifdef RLIB_ARRAY_SYNC
				/// <summary>
				/// Gets an object that can be used to synchronize access to the Array&lt;Of R&gt;
				/// </summary>
				Threading::CriticalSection SyncRoot;
#endif // RLIB_ARRAY_SYNC

			public:
				Array(intptr_t length = RLIB_DEFAULT_CAPACITY)
				{
					this->_init(length);
				}
				/// <summary>
				/// Initializes Array&lt;Of R&gt; using initializer-list
				/// </summary>
				Array(const Standard::initializer_list<const R> &data)
				{
					// @warning constructor is not thread-safe
					this->_init(static_cast<intptr_t>(data.size()));
					for (auto &item : data) {
						this->_add_nolock(item);
					}
				}
				Array(const R items[], intptr_t length) : Array(length)
				{
					for(--length; length >=0; --length) {
						this->_add_nolock(items[length]);
					}
				}
				Array(const Array &array_from)
				{
					this->_init(array_from.Length);
					foreach(pitem, array_from) {
						this->Add(*pitem);
					}
				}
				~Array()
				{
					this->Clear();
					allocator::freeMemory(this->m_pItems);
				}
				RLIB_DECLARE_DYNCREATE;

			public:
				Array &operator = (const Array &obj)
				{
					this->Clear();
					foreach(pitem, obj) {
						this->Add(*pitem);
					}
					return *this;
				}
				Array &operator += (const R &item)
				{
					this->Add(item);
					return *this;
				}
				/// <summary>
				/// Gets the Type of the current instance
				/// </summary>
				RLIB_FORCE_INLINE R *GetType() const { return nullptr; }
				/// <summary>
				/// Requests that the Array&lt;Of R&gt; be at least enough to contain n elements
				/// </summary>
				void InitStorage(intptr_t n)
				{
					RLIB_ARRAY_LOCK;
					if (n > this->MaxLength) {
						_expand(n - this->MaxLength);
					} //if
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// Represents the Type of the current instance
				/// </summary>
				typedef R Type;
				/// <summary>
				/// Adds specified item and returns the index of the added.
				/// </summary>
				template<class... P>
				intptr_t Add(P &&... args)
				{
					intptr_t index;

					RLIB_ARRAY_LOCK;
					if (this->Length + 1 > this->MaxLength) {
						this->_expand((this->Length + 1) - this->MaxLength);
					} //if
					index = this->_add_nolock(Standard::forward<P>(args)...);
					RLIB_ARRAY_UNLOCK;

					return index;
				}
				/// <summary>
				/// Adds the specified items and returns the index of the first added.
				/// </summary>
				template <typename... P> intptr_t AddRange(const R &item, const P &... args)
				{
					intptr_t count = sizeof...(args) + 1;
					RLIB_ARRAY_LOCK;

					if (this->Length + count > this->MaxLength) {
						this->_expand((this->Length + count) - this->MaxLength);
					} //if
					RLIB_RENAME(count, index);
					index = this->_add_sequence_nolock(item, args...);

					RLIB_ARRAY_UNLOCK;
					return index;
				}
				/// <summary>
				/// Adds the specified items and returns the index of the first added.
				/// </summary>
				intptr_t AddRange(const R items[], intptr_t count)
				{
					RLIB_ARRAY_LOCK;

					if (this->Length + count > this->MaxLength) {
						this->_expand((this->Length + count) - this->MaxLength);
					} //if
					intptr_t index = -1;
					if (--count >= 0) {
						index = this->_add_nolock(*items);
						while (--count >= 0) {
							this->_add_nolock(*items);
							++items;
						}
					} //if

					RLIB_ARRAY_UNLOCK;
					return index;
				}
				/// <summary>
				/// Adds the specified items and returns the index of the first added.
				/// </summary>
				template<intptr_t N>
				intptr_t AddRange(const R(&items)[N])
				{
					return this->AddRange(items, N);
				}
				/// <summary>
				/// Removes all elements from the Array&lt;Of R&gt;
				/// </summary>
				void Clear()
				{
					RLIB_ARRAY_LOCK;
					while(--this->Length >= 0) {
						disposer::Dispose(&this->m_pItems[this->Length]);
					}
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// Gets the element at the specified location
				/// @warning index is not validated against the number of elements in the array
				/// </summary>
				R &operator [] (intptr_t index) const
				{ 
					return this->GetValue(index);
				} 
				/// <summary>
				/// Gets the element at the specified location
				/// </summary>
				R &GetValue(intptr_t index) const
				{ 
					assert(RLIB_ARRAYRANGE(index, 0, this->Length) || !"overflow"); 
					return this->m_pItems[index]; 
				} 
				/// <summary>
				/// 确定某元素是否在当前 Array&lt;Of R&gt; 中
				/// </summary>
				bool Contains(const R &item) const
				{
					return this->IndexOf(item) != -1;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 Array&lt;Of R&gt; 中第一个匹配项的索引
				/// </summary>
				/// <returns>失败返回-1</returns>
				intptr_t IndexOf(const R &item, 
								 typename IComparer<R>::EqualsDelegate equals,
								 intptr_t begin = 0) const
				{
					if (begin < 0 || begin >= this->Length) return -1;

					intptr_t rindex;

					RLIB_ARRAY_LOCK;
					rindex = System::Generic::Search::sequential_search(this->m_pItems,
																		begin,
																		this->Length - 1,
																		item,
																		equals);
					RLIB_ARRAY_UNLOCK;

					return rindex;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 Array&lt;Of R&gt; 中第一个匹配项的索引
				/// </summary>
				/// <returns>失败返回-1</returns>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals>
				intptr_t IndexOf(const R &item, intptr_t begin = 0) const
				{
					if (begin < 0 || begin >= this->Length) return -1;

					intptr_t rindex;

					RLIB_ARRAY_LOCK;
					rindex = System::Generic::Search::sequential_search<R, equals>(this->m_pItems,
																				   begin,
																				   this->Length - 1,
																				   item);
					RLIB_ARRAY_UNLOCK;

					return rindex;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 Array&lt;Of R&gt; 中最后一个匹配项的索引
				/// </summary>
				/// <returns>失败返回-1</returns>
				intptr_t LastIndexOf(const R &item, typename IComparer<R>::EqualsDelegate equals, intptr_t rbegin = 0) const
				{
					if (rbegin < 0 || rbegin >= this->Length) return -1;

					intptr_t rindex;

					RLIB_ARRAY_LOCK;
					rindex = System::Generic::Search::sequential_search(this->m_pItems,
																		rbegin,
																		this->Length - 1,
																		item,
																		equals);
					RLIB_ARRAY_UNLOCK;

					return rindex;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 Array&lt;Of R&gt; 中最后一个匹配项的索引
				/// </summary>
				/// <returns>失败返回-1</returns>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals> 
				intptr_t LastIndexOf(const R &item, intptr_t rbegin = 0) const
				{
					if (rbegin < 0 || rbegin >= this->Length) return -1;

					intptr_t rindex;

					RLIB_ARRAY_LOCK;
					rindex = System::Generic::Search::sequential_search_end<R, equals>(this->m_pItems,
																					   rbegin,
																					   this->Length - 1,
																					   item);
					RLIB_ARRAY_UNLOCK;

					return rindex;
				}
				/// <summary>
				/// 移除特定元素的第一个匹配项
				/// </summary>
				void Remove(const R &item, typename IComparer<R>::EqualsDelegate equals)
				{
					this->RemoveAt(this->IndexOf(item, equals));
				}
				/// <summary>
				/// 移除特定元素的第一个匹配项
				/// </summary>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals> void Remove(const R &item)
				{
					this->RemoveAt(this->IndexOf<equals>(item));
				}
				/// <summary>
				/// 移除特定元素的所有匹配项
				/// </summary>
				void RemoveAll(const R &item)
				{
					intptr_t prev = 0, that;
					while ((that = this->IndexOf(item, prev)) != -1) {
						prev = that;
						this->RemoveAt(that);
					}
				}
				/// <summary>
				/// 移除指定索引处的元素
				/// </summary>
				void RemoveAt(intptr_t index)
				{
					if (index < 0 || index >= this->Length) return;

					RLIB_ARRAY_LOCK;
					disposer::Dispose(&this->m_pItems[index]);
					--this->Length;
					_copy_forward(index);
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// Searches an entire one-dimensional sorted array for a value using the specified IComparer interface.
				/// @warning undefined behaviour if the array is unsorted or contains duplicate values
				/// </summary>
				intptr_t BinarySearch(const R &item, typename IComparer<R>::Delegate comparer, intptr_t begin = 0) const
				{
					if (begin < 0 || begin >= this->Length) return -1;

					auto r = System::Generic::Search::binary_search(this->m_pItems,
																	begin,
																	this->Length - 1,
																	item,
																	comparer);
					return r;
				}
				/// <summary>
				/// Searches an entire one-dimensional sorted array for a value using the specified IComparer interface.
				/// @warning undefined behaviour if the array is unsorted or contains duplicate values
				/// </summary>
				template<typename IComparer<R>::Delegate comparer = IComparer<R>::Compare> 
				intptr_t BinarySearch(const R &item, intptr_t begin = 0) const
				{
					if (begin < 0 || begin >= this->Length) return -1;

					auto r = System::Generic::Search::binary_search<R, comparer>(this->m_pItems,
																				 begin,
																				 this->Length - 1,
																				 item);
					return r;
				}
				/// <summary>
				/// Sorts the elements in the entire Array&lt;Of R&gt; using the IComparable generic interface implementation.
				/// This method uses the QuickSort algorithm, which on average, is an O(n log n) operation;
				/// in the worst case it is an O(n ^ 2) operation
				/// </summary>
				void Sort(typename IComparer<R>::Delegate comparer, intptr_t begin = 0, intptr_t count = -1)
				{
					if (begin < 0 || begin >= this->Length || count == 0) return;

					RLIB_ARRAY_LOCK;
					intptr_t length = this->Length - begin;
					if (count < 0 || count > length) count = length;
					System::Generic::Sorting::quick_sort(&this->m_pItems[begin],
														 static_cast<size_t>(count),
														 sizeof(R),
														 reinterpret_cast<int(*)(const void *, const void *)>(comparer));
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// Sorts the elements in the entire Array&lt;Of R&gt;
				/// O(N·log(N))
				/// </summary>
				void Sort(intptr_t begin = 0, intptr_t count = -1)
				{
					if (begin < 0 || begin >= this->Length || count == 0) return;

					RLIB_ARRAY_LOCK;

					intptr_t length = this->Length - begin;
					if (count < 0 || count > length) count = length;
					System::Generic::Sorting::sort(&this->m_pItems[begin],
												   &this->m_pItems[begin + count]);

					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// 删除 Array&lt;Of R&gt; 中所有重复的相邻元素
				/// </summary>
				void Unique(typename IComparer<R>::EqualsDelegate equals)
				{
					RLIB_ARRAY_LOCK;
					// erase each element matching previous
					R *_Pnode = this->m_pItems;
					intptr_t _k; // 需要删除的元素个数
					while (_Pnode < &this->m_pItems[this->Length]) {
						_k = 0;
						while (&_Pnode[_k] < &this->m_pItems[this->Length - 1] && 
							   equals(&_Pnode[_k], &_Pnode[_k + 1]))
						{	// 计算连续且重复的个数
							++_k;
						}
						++_Pnode;
						if (_k != 0) {
							// 析构对象
							for (intptr_t _i = 0; _i < _k; ++_i) {
								disposer::Dispose(&_Pnode[_i]);
							}
							memcpy(_Pnode, _Pnode + _k,
								   sizeof(R) * (&this->m_pItems[this->Length] - (_Pnode + _k)));
							this->Length -= _k;
						} //if
						// no match, advance
					}
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// 删除 Array&lt;Of R&gt; 中所有重复的相邻元素
				/// </summary>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals> void Unique()
				{
					return this->Unique(equals);
				}
				/// <summary>
				/// 插入新元素到指定位置
				/// </summary>
				void Insert(intptr_t index, const R &item)
				{
					if (index < 0) return;
		            if(index >= this->Length) { 
						this->Add(item);
						return; 
					} //if

					RLIB_ARRAY_LOCK;
					if (this->Length >= this->MaxLength) _expand(RLIB_DEFAULT_CAPACITY);
	                _copy_backward(index);
				    ++this->Length;
                    RLIB_InitClass(&this->m_pItems[index], R(item));
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// 将指定范围中元素的顺序反转
				/// </summary>
				/// <param name="order">从0开始的顺序位置</param>
				void Reverse(intptr_t begin = 0)
				{
					if (begin < 0 || begin >= (this->Length - 1)) {
						return;
					} //if

					RLIB_ARRAY_LOCK;
					{
						intptr_t last = this->Length - 1;
						while (begin != last) {
							Utility::swap(this->m_pItems[begin], this->m_pItems[last]);
							++begin;
							--last;
						}
					}
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// 将 Array&lt;Of R&gt; 完全拷贝至指定集合处
				/// </summary>
				template<class collection_t> void CopyTo(collection_t &obj) const
				{
					RLIB_LIST_LOCK;
					for (intptr_t i = 0; i < this->Length; ++i) {
						obj.Add(this->m_pItems[i]);
					} //for
					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// 将 Array&lt;Of R&gt; 完全转移至另一 Array&lt;Of R&gt; 处
				/// </summary>
				void MoveTo(Array &obj)
				{
					RLIB_ARRAY_LOCK;

					if (obj.Length == 0) {
						Utility::swap(*this, obj);
					} else {
						obj.WriteToEnd(this->m_pItems, this->Length);
					} //if
					
					this->Length = 0;

					RLIB_ARRAY_LOCK;
				}
				/// <summary>
				/// 将指定数组浅拷贝至当前 Array&lt;Of R&gt;
				/// </summary>
				void WriteToEnd(const R *data, intptr_t count)
				{
					RLIB_ARRAY_LOCK;
					{
						if (this->Length + count > this->MaxLength) {
							_expand((this->Length + count) - this->MaxLength);
						} //if
						memcpy(&this->m_pItems[this->Length], data, count * sizeof(R));
						this->Length += count;
						assert(this->Length <= this->MaxLength);
					}
					RLIB_ARRAY_UNLOCK;
				}
				/// <summary>
				/// 获取 Array&lt;Of R&gt; 数据存储区指针
				/// @warning 不提供线程安全支持
				/// </summary>
				R *ToByteArray() {
					return this->m_pItems;
				}
				/// <summary>
				/// 获取 Array&lt;Of R&gt; 数据存储区有效数据段大小(in bytes)
				/// @warning 不提供线程安全支持
				/// </summary>
				intptr_t GetByteArrayLength() {
					return this->Length * this->GetByteArrayItemSize();
				}
				/// <summary>
				/// 获取 Array&lt;Of R&gt; 数据存储区总可读写大小(in bytes)
				/// @warning 不提供线程安全支持
				/// </summary>
				intptr_t GetByteArraySize() {
					return this->MaxLength * this->GetByteArrayItemSize();
				}
				/// <summary>
				/// 获取 Array&lt;Of R&gt; 数据存储区每个有效数据元素大小(in bytes)
				/// </summary>
				RLIB_INLINE intptr_t GetByteArrayItemSize() {
					return static_cast<intptr_t>(sizeof(R));
				}

			public: // iteration
				/// <summary>
				/// Returns an iterator to the first element of the container.
				/// If the container is empty or nothing, the returned iterator will be equal to end()
				/// </summary>
				R *begin() /* noexcept */ {
					return this != nullptr && this->Length > 0 ? this->m_pItems : nullptr;
				}
				const R *begin() const  /* noexcept */ {
					return this != nullptr && this->Length > 0 ? this->m_pItems : nullptr;
				}
				/// <summary>
				/// Returns an iterator to the element following the last element of the container.
				/// This element acts as a placeholder; attempting to access it results in undefined behavior.
				/// </summary>
				R *end() /* noexcept */ {
					return this != nullptr && this->Length > 0 ? &this->m_pItems[this->Length] : nullptr;
				}
				const R *end() const /* noexcept */ {
					return this != nullptr && this->Length > 0 ? &this->m_pItems[this->Length] : nullptr;
				}
			};
		}
	}
}
#endif // _USE_ARRAY
