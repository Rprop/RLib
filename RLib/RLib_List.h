/********************************************************************
	Created:	2012/02/06  19:40
	Filename: 	RLib_List.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_LIST
#define _USE_LIST
#include "RLib_CollectionGeneric.h"
#include "RLib_Array.h"

//#define RLIB_LIST_SYNC   
#ifdef RLIB_LIST_SYNC
# define RLIB_LIST_LOCK   this->SyncRoot.Enter()
# define RLIB_LIST_UNLOCK this->SyncRoot.Exit()
#else
# define RLIB_LIST_LOCK
# define RLIB_LIST_UNLOCK
#endif // RLIB_ARRAY_SYNC

#define foreachList(start, _list)  for (auto start = _list.begin(); start != _list.end(); ++start)
#define foreachpList(start, plist) for (auto start = plist->begin(); start != plist->end(); ++start)

//-------------------------------------------------------------------------

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			template <class R, typename ListNodePointer>
			struct ListIterator
			{
				mutable ListNodePointer lpnode;

			public:
				bool operator != (const ListIterator &itor) const {
					return this->lpnode != itor.lpnode;
				}
				ListIterator &operator = (ListNodePointer p) {
					this->lpnode = p;
					return *this;
				}
				ListIterator &operator ++() {
					return *this = this->Next();
				}
				const ListIterator &operator ++() const {
					return *this = this->Next();
				}
				ListIterator &operator --() {
					return *this = this->Prev();
				}
				const ListIterator &operator --() const {
					return *this = this->Prev();
				}
				operator ListNodePointer() {
					return this->lpnode;
				}
				ListNodePointer Prev() {
					return this->lpnode->pPrev;
				}
				const ListNodePointer Prev() const {
					return this->lpnode->pPrev;
				}
				ListNodePointer Next() {
					return this->lpnode->pNext;
				}
				const ListNodePointer Next() const {
					return this->lpnode->pNext;
				}
				operator const ListNodePointer() const {
					return this->lpnode;
				}
				R &operator *() {
					return this->lpnode->Node;
				}
				const R &operator *() const {
					return this->lpnode->Node;
				}
			};
			/// <summary>
			/// Represents a doubly linked list.
			/// Provides methods to search, sort, and manipulate lists
			/// </summary>
			template <class R, class disposer = IDisposable<R>, class allocator = IO::IAllocator>
			class List/* : public Iterable*/
			{
			private:
				typedef struct ListNode
				{
					friend class List;
					friend struct Iterator;

				public:
					R &GetNode() {
						return reinterpret_cast<R &>(this->obj);
					}
					const R &GetNode() const {
						return reinterpret_cast<const R &>(this->obj);
					}
					RLIB_PROPERTY_GET(R &Node, GetNode);

				protected:
					struct ListNode *pNext;
					struct ListNode *pPrev;
#ifdef _DEBUG
					List *pOwner;
					R    *pInstance;
#endif // _DEBUG

				private:
					unsigned char obj[sizeof(R)];
				} ListNode;

			public:
				typedef ListNode *ListNodePointer;

			private:
				ListNodePointer pFirstNode;
				ListNodePointer pLastNode;
				ListNodePointer pCacheNodes;
				LONG            nCacheTotal, nCacheIndex;
				ListNodePointer allocListNode()
				{
					ListNodePointer p;
					RLIB_LIST_LOCK;
					if (this->pCacheNodes != nullptr && this->nCacheIndex + 1 < this->nCacheTotal) {
						p = &this->pCacheNodes[++this->nCacheIndex];
						RLIB_LIST_UNLOCK;
					} else {
						RLIB_LIST_UNLOCK;
						p = static_cast<ListNodePointer>(allocator::allocateMemory(sizeof(ListNode)
																				   RLIB_INTERNAL_DEBUG_PARAM_HERE));
					} //if
#ifdef _DEBUG					
					if (p != nullptr) {
						p->pOwner    = this;
						p->pInstance = &p->Node;
					} //if
#endif // _DEBUG
					return p;
				}
				template<class... P>
				ListNodePointer newListNode(P &&... args)
				{
					ListNodePointer p = this->allocListNode();
					if (p != nullptr) {
						RLIB_InitClass(&p->Node, R(Standard::forward<P>(args)...));
					} //if
					return p;
				}
				void delListNode(ListNodePointer pNode)
				{
					disposer::Dispose(&pNode->Node);
					if (pNode >= &this->pCacheNodes[this->nCacheTotal] || pNode < this->pCacheNodes) {
						allocator::freeMemory(pNode);
					} //if			
				}
				void swap(ListNodePointer pNode, ListNodePointer pNextNode)
				{
					if (pNode->pNext == pNextNode) // 相邻元素的交换处理
					{
						assert(pNextNode->pPrev == pNode); 
						// 交换前 A→←B(pNode)→←C(pNextNode)→←D
						// 交换后 A→←C(pNextNode)→←B(pNode)→←D

						if (pNode->pPrev != nullptr) { // 考虑与开始交换
							pNode->pPrev->pNext = pNextNode;
						} //if
						pNextNode->pPrev    = pNode->pPrev;
						pNode->pPrev        = pNextNode;
						pNode->pNext        = pNextNode->pNext;

						if (pNode->pNext != nullptr) { // 考虑与结尾交换
							pNode->pNext->pPrev = pNode;
						} //if
						pNextNode->pNext    = pNode;
						return;
					} //if

					ListNodePointer pTempListNode = pNode->pPrev;
					pNode->pPrev                  = pNextNode->pPrev;
					pNextNode->pPrev              = pTempListNode;

					pTempListNode    = pNode->pNext;
					pNode->pNext     = pNextNode->pNext;
					pNextNode->pNext = pTempListNode;

					if (pNode->pPrev != nullptr) pNode->pPrev->pNext = pNode;
					if (pNode->pNext != nullptr) pNode->pNext->pPrev = pNode;

					if (pNextNode->pPrev != nullptr) pNextNode->pPrev->pNext = pNextNode;
					if (pNextNode->pNext != nullptr) pNextNode->pNext->pPrev = pNextNode;
				}
				RLIB_INLINE void link(ListNodePointer pNode, ListNodePointer pNextNode)
				{
					pNode->pNext     = pNextNode;
					pNextNode->pPrev = pNode;
				}
	
			protected:
				intptr_t Count;

			public:
#ifdef RLIB_LIST_SYNC
				/// <summary>
				/// 用于同步 List&lt;Of R&gt; 访问的对象
				/// </summary>
				Threading::CriticalSection SyncRoot;
#endif // RLIB_LIST_SYNC

			public:
				RLIB_DECLARE_DYNCREATE;
				List(intptr_t cache_capcity = RLIB_DEFAULT_CAPACITY)
				{
					this->pFirstNode  = nullptr;
					this->pLastNode   = nullptr; 
					this->pCacheNodes = nullptr;
					this->nCacheTotal = 0;
					this->Count       = 0;
					if (cache_capcity > 0) this->InitStorage(cache_capcity);
				}
				List(const List<R> &list_from) : List()
				{
					foreach(pitem, list_from) {
						this->Add(*pitem);
					}
				}
				List(const Standard::initializer_list<const R> &data)
					: List(Utility::select_max<intptr_t>(static_cast<intptr_t>(data.size()), RLIB_DEFAULT_CAPACITY))
				{
					for (auto &item : data) {
						this->Add(item);
					}
				}
				~List() 
				{
					this->Clear();
					this->ReleaseStorage();
				}

			public:
				/// <summary>
				/// Creates back storage for caching.
				/// This function helps speed up the insertion of List that have a large number of elements,
				/// by avoiding the frequent memory allocation
				/// </summary>
				void InitStorage(intptr_t cache_count)
				{					
					if (this->pCacheNodes != nullptr) {
						assert(this->nCacheTotal != 0 && this->nCacheIndex <= this->nCacheTotal);
						if ((this->Count > 0 && this->nCacheIndex >= 0) || cache_count <= this->nCacheTotal) {
							// some nodes holds at least one cache
							return;
						} //if
						this->ReleaseStorage();
					} //if

					this->nCacheTotal = static_cast<long>(cache_count);
					this->nCacheIndex = -1;
					this->pCacheNodes = static_cast<ListNodePointer>(allocator::allocateMemory(RLIB_SIZEOF(ListNode) * cache_count 
																							   RLIB_INTERNAL_DEBUG_PARAM_HERE));
				}
				/// <summary>
				/// destroy back storage
				/// </summary>
				void ReleaseStorage()
				{
					if (this->pCacheNodes != nullptr) {
						assert(this->nCacheTotal != 0 && this->nCacheIndex < 0);
						allocator::freeMemory(this->pCacheNodes);
						this->pCacheNodes = nullptr;
					} //if
				}
#pragma optimize("", off)
				/// <summary>
				/// 安全获取 List&lt;Of R&gt; 中实际包含的元素数
				/// </summary>
				intptr_t GetSafeLength() const
				{
#ifndef RLIB_LIST_SYNC
					alert(_T("not supported"));
#endif // RLIB_LIST_SYNC
					RLIB_LIST_LOCK;
					auto length = this->Count;
					RLIB_LIST_UNLOCK;
					return length;
				}
#pragma optimize("", on)
				/// <summary>
				/// 安全获取 List&lt;Of R&gt; 中实际包含的元素数
				/// </summary>
				RLIB_PROPERTY_GET(intptr_t SafeLength, GetSafeLength);
				/// <summary>
				/// Gets the Type of the current instance
				/// </summary>
				RLIB_FORCE_INLINE R *GetType() const { return nullptr; }
				/// <summary>
				/// Represents the Type of the current instance
				/// </summary>
				typedef R Type;
				/// <summary>
				/// 获取 List&lt;Of R&gt; 中实际包含的元素数
				/// </summary>
				intptr_t GetLength() const
				{
					return this->Count;
				}
				/// <summary>
				/// 获取 List&lt;Of R&gt; 中实际包含的元素数
				/// </summary>
				RLIB_PROPERTY_GET(const intptr_t Length, GetLength);
				/// <summary>
				/// 从 List&lt;Of R&gt; 中移除所有元素
				/// </summary>
				template<typename disposer = IDisposable<R>> void Clear()
				{
					RLIB_LIST_LOCK;
					{
						ListNodePointer pNext = this->pFirstNode;
						while (pNext != nullptr)
						{
							this->pFirstNode = pNext;
							pNext            = pNext->pNext;
							delListNode(this->pFirstNode); // !!keep del order						
						}
						this->pFirstNode  = nullptr;
						this->pLastNode   = nullptr;
						this->nCacheIndex = -1;
						this->Count       = 0;	
					}
					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// 赋值运算符 复制对象
				/// </summary>
				List<R> &operator = (const List<R> &obj)
				{
					this->Clear();
					foreach(pitem, obj)
					{
						if(pitem != nullptr) this->Add(*pitem);
					}
					return *this;
				}
				/// <summary>
				/// 拓展运算符 添加对象
				/// </summary>
				List &operator += (const R &item)
				{
					this->Add(item);
					return *this;
				}
				/// <summary>
				/// @see AddLast
				/// </summary>
				template<class... P>
				ListNodePointer Add(P &&... args)
				{
					return this->AddLast(Standard::forward<P>(args)...);
				}
				/// <summary>
				/// 将对象添加到 List&lt;Of R&gt; 的开始处
				/// </summary>
				template<class... P>
				ListNodePointer AddFirst(P &&... args)
				{
					ListNodePointer newNode = this->newListNode(Standard::forward<P>(args)...);
					if (newNode != nullptr)
					{
						newNode->pPrev = nullptr;
						RLIB_LIST_LOCK;
						{
							if (this->pFirstNode == nullptr) {
								assert(this->Count == 0);
								newNode->pNext   = nullptr;
								this->pFirstNode = newNode;
								this->pLastNode  = newNode;
							} else {
								link(newNode, this->pFirstNode);								
								this->pFirstNode = newNode;
							} //if
							
							++this->Count;
						}
						RLIB_LIST_UNLOCK;
					} //if
					return newNode;
				} 	
				/// <summary>
				/// 将对象添加到 List&lt;Of R&gt; 的结尾处
				/// </summary>
				template<class... P>
				ListNodePointer AddLast(P &&... args)
				{
					ListNodePointer newNode = this->newListNode(Standard::forward<P>(args)...);
					if (newNode != nullptr) {
						newNode->pNext = nullptr;
						RLIB_LIST_LOCK;
						{
							if (this->pLastNode != nullptr) {
								link(this->pLastNode, newNode);
								this->pLastNode = newNode;							
							} else {
								assert(this->pFirstNode == nullptr);
								assert(this->Count == 0);
								newNode->pPrev   = nullptr;
								this->pFirstNode = newNode;
								this->pLastNode  = newNode;
							} //if
							++this->Count;
						}
						RLIB_LIST_UNLOCK;
					} //if				
					return newNode;
				} 	
				/// <summary>
				/// 添加指定元素到 List&lt;Of R&gt; 的末尾, 
				/// 并返回首元素链节点指针
				/// </summary>
				template <typename ... Args> ListNodePointer AddRange(const R &item)
				{
					return this->Add(item);
				}
				/// <summary>
				/// 添加指定元素到 List&lt;Of R&gt; 的末尾, 
				/// 并返回首元素链节点指针
				/// </summary>
				template <typename ... Args> ListNodePointer AddRange(const R &item, const Args& ... args)
				{
					auto lpnode = this->Add(item);
					if (lpnode != nullptr) {
						this->AddRange(args...);
					} //if
					return lpnode;
				}
				/// <summary>
				/// 将指定集合的元素添加到 List&lt;Of R&gt; 的末尾, 
				/// 并返回首元素链节点指针
				/// </summary>
				template<intptr_t N> 
				ListNodePointer AddRange(const R(&items)[N])
				{
					return this->AddRange(items, N);
				}
				/// <summary>
				/// 将指定集合的元素添加到 List&lt;Of R&gt; 的末尾, 
				/// 并返回首元素链节点指针
				/// </summary>
				ListNodePointer AddRange(const R items[], intptr_t count)
				{
					assert(items != nullptr);
					ListNodePointer pAddedNode = nullptr;
					if (this->pFirstNode == nullptr)
					{
						// add first node
						if ((pAddedNode = this->AddFirst(*items)) == nullptr) {
							return nullptr;
						} //if
						--count;
						++items;
					} //if
					if (count >= 1) {
						RLIB_LIST_LOCK;
						do {
							assert(this->pLastNode != nullptr);
							ListNodePointer newNode = this->newListNode(*items);
							if (newNode == nullptr) {
								break;
							} //if

							link(this->pLastNode, newNode);
							newNode->pNext  = nullptr;
							this->pLastNode = newNode;
							++this->Count;
							++items;
						} while (--count > 0);
						RLIB_LIST_UNLOCK;
					} //if

					return pAddedNode;
				}
				/// <summary>
				/// 将 List&lt;Of R&gt; 的值转换成 Array<Of R>
				/// </summary>
				Array<R> *ToArray(intptr_t count = 0)
				{
					if (count <= 0 || count > this->Count) {
						count = this->Count;
					} //if

					Array<R> *_array = new Array<R>(count);
					RLIB_LIST_LOCK;
					ListNodePointer pNode = this->pFirstNode;
					while(pNode != nullptr && count > 0)
					{
						_array->Add(pNode->Node);
						pNode = pNode->pNext;
						--count;
					}
					RLIB_LIST_UNLOCK;
					return _array;
				}
				/// <summary>
				/// 确定某元素是否在 List&lt;Of R&gt; 中
				/// </summary>
				bool Contains(const R &item) const
				{
					return this->IndexOf(item) != -1;
				}
				/// <summary>
				/// 将元素插入 List&lt;Of R&gt; 的指定元素前面, 否则插入末尾
				/// </summary>
				ListNodePointer Insert(const R &item, const R &itemToInsert, 
									   typename IComparer<R>::EqualsDelegate equals)
				{
					if (this->pFirstNode == nullptr) {
						return this->AddFirst(itemToInsert);
					} //if

					RLIB_LIST_LOCK;
					ListNodePointer p = this->pFirstNode, newNode = nullptr;
					while(p != nullptr)
					{
						if (equals(&p->Node, &item)) {
							newNode = this->newListNode(itemToInsert);
							if (newNode != nullptr) {
								if (p->pPrev != nullptr) {
									link(p->pPrev, newNode);
									link(newNode, p);
								} else {
									assert(p == this->pFirstNode);
									link(newNode, p);
									newNode->pPrev   = nullptr;
									this->pFirstNode = newNode;
								} //if
								++this->Count;
							} //if
							break;
						} //if
						p = p->pNext;
					}
					RLIB_LIST_UNLOCK;
					return newNode != nullptr ? newNode : this->AddLast(itemToInsert);
				}
				/// <summary>
				/// 将元素插入 List&lt;Of R&gt; 的指定元素前面
				/// </summary>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals>
				ListNodePointer Insert(const R &item, const R &itemToInsert)
				{
					return this->Insert(item, itemToInsert, equals);
				}
				/// <summary>
				/// 将元素插入 List&lt;Of R&gt; 的指定元素前面
				/// </summary>
				ListNodePointer Insert(ListNodePointer pNode, const R &item)
				{
#ifdef _DEBUG
					assert(pNode->pOwner == this);
#endif // _DEBUG
					if (pNode->pPrev == nullptr) {
						assert(pNode == this->pFirstNode);
						return this->AddFirst(item);
					} //if

					ListNodePointer newNode = this->newListNode(item);
					if (newNode != nullptr) {
						RLIB_LIST_LOCK;
						link(pNode->pPrev, newNode);
						link(newNode, pNode);
						++this->Count;
						RLIB_LIST_UNLOCK;
					} //if
					return newNode; 
				} 
				/// <summary>
				/// 将元素插入 List&lt;Of R&gt; 的从零开始的指定位置顺序前面
				/// </summary>
				ListNodePointer InsertAt(intptr_t order, const R &item)
				{
					if (order <= 0) {
						trace(!"List<Of R> underflow");
						return this->AddFirst(item);
					} else if (order >= (this->Count - 1)) {
						trace(!"List<Of R> overflow");
						return this->AddLast(item);
					} //if

					ListNodePointer p = this->pFirstNode, newNode = newListNode(item);
					if (newNode != nullptr) {
						RLIB_LIST_LOCK;
						{
							while(--order >= 0)
							{
								p = p->pNext;
							}					
							assert(p != nullptr);
							link(p->pPrev, newNode);
							link(newNode, p);
							++this->Count;
						}
						RLIB_LIST_UNLOCK;
					} //if
					return newNode; 
				} 
				/// <summary>
				/// 将集合中的某个元素插入 List&lt;Of R&gt; 的指定指定元素前面
				/// </summary>
				ListNodePointer InsertRange(const R &item, const R *itemsToInsert, intptr_t count)
				{
					assert(itemsToInsert != nullptr);
					ListNodePointer pAddedNode = nullptr;
					while (count > 0)
					{
						ListNodePointer p = this->Insert(item, *itemsToInsert);
						if (p == nullptr) {
							break;
						} //if

						++itemsToInsert;
						--count;

						if (pAddedNode == nullptr) {
							pAddedNode = p;
						} //if
					}
					return pAddedNode;
				}
				/// <summary>
				/// 从 List&lt;Of R&gt; 中移除特定对象的第一个匹配项 
				/// </summary>
				/// <returns>如果成功移除item, 则为true;否则为false.如果没有找到item, 也会返回false</returns>
				bool Remove(const R &item, typename IComparer<R>::EqualsDelegate equals)
				{
					bool result = false;
					RLIB_LIST_LOCK;
					{
						ListNodePointer p = this->pFirstNode;
						while (p != nullptr)
						{
							if (equals(&p->Node, &item)) {
								if (p->pPrev != nullptr) {
									assert(this->pFirstNode != nullptr);
									if (p->pNext != nullptr) {
										assert(this->pLastNode != nullptr);
										link(p->pPrev, p->pNext);
									} else {
										assert(p == this->pLastNode);
										this->pLastNode        = p->pPrev;
										this->pLastNode->pNext = nullptr;
									} //if
								} else {
									assert(p == this->pFirstNode);
									this->pFirstNode = p->pNext;
									if (p->pNext != nullptr) {
										assert(this->pLastNode != nullptr);
										p->pNext->pPrev = nullptr;
									} else {
										assert(p == this->pLastNode);
										this->pLastNode = nullptr;
									} //if
								} //if

								delListNode(p);
								--this->Count;
								result = true;
								break;
							} //if
							p = p->pNext;
						}
					}
					RLIB_LIST_UNLOCK;
					return result;
				}
				/// <summary>
				/// 从 List&lt;Of R&gt; 中移除特定对象的第一个匹配项 
				/// </summary>
				/// <returns>如果成功移除item, 则为true;否则为false.如果没有找到item, 也会返回false</returns>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals>
				bool Remove(const R &item)
				{
					return this->Remove(item, equals);
				}
				/// <summary>
				/// 从 List&lt;Of R&gt; 中移除特定链节点
				/// </summary>
				void Remove(ListNodePointer pNode)
				{
					assert(pNode != nullptr);
					assert(pNode->pOwner == this);

					RLIB_LIST_LOCK;
					if (pNode->pPrev == nullptr) {
						assert(this->pFirstNode == pNode);
						if (pNode->pNext == nullptr) {
							assert(this->pLastNode == pNode);
							assert(this->Count == 1);
							this->pFirstNode = this->pLastNode = nullptr;
						} else {
							assert(this->pLastNode != nullptr);
							this->pFirstNode    = pNode->pNext;
							pNode->pNext->pPrev = nullptr;
						} //if
					} else if (pNode->pNext == nullptr) {					
						assert(this->pFirstNode != pNode);
						assert(this->pLastNode == pNode);
						this->pLastNode     = pNode->pPrev;
						pNode->pPrev->pNext = nullptr;
						assert(this->pLastNode != nullptr);
					} else {
						link(pNode->pPrev, pNode->pNext);
					} //if
					
					delListNode(pNode);
					--this->Count;

					RLIB_LIST_UNLOCK;
				} 
				/// <summary>
				/// 从 List&lt;Of R&gt; 中移除指定位置顺序的对象
				/// </summary>
				/// <returns>如果成功移除item, 则为true</returns>
				bool RemoveAt(intptr_t order)
				{
					if (order < 0 || order > (this->Count - 1) || this->Count == 0) {
						trace(!"List<Of R> overflow");
						return false;
					} //if
		
					RLIB_LIST_LOCK;
					ListNodePointer p = this->pFirstNode;
					while (order > 0) {
						--order;
						p = p->pNext;
					}
					assert(p != nullptr);
					if (p->pPrev == nullptr) {
						assert(this->pFirstNode == p);
						if (p->pNext == nullptr) {
							assert(this->pLastNode == p);
							assert(this->Count == 1);
							this->pFirstNode = this->pLastNode = nullptr;
						} else {
							this->pFirstNode = p->pNext;
							p->pNext->pPrev  = nullptr;
						} //if
					} else if (p->pNext == nullptr) {
						assert(this->pFirstNode != p);
						assert(this->pLastNode == p);
						this->pLastNode = p->pPrev;
						p->pPrev->pNext = nullptr;
					} else {
						link(p->pPrev, p->pNext);
					} //if

					delListNode(p);
					--this->Count;

					RLIB_LIST_UNLOCK;
					return true; 
				} 
				/// <summary>
				/// 移除与特定对象相匹配的所有元素
				/// </summary>
				/// <returns>从 List&lt;Of R&gt; 中移除的元素的数目</returns>
				intptr_t RemoveAll(const R &item)
				{
					intptr_t removeCount = 0;
					while (this->Remove(item)) ++removeCount;
					return removeCount;
				}
				/// <summary>
				/// 移除位于 List&lt;Of R&gt; 开头处的节点
				/// </summary>
				bool RemoveFirst()
				{
					return this->RemoveAt(0);
				} 
				/// <summary>
				/// 移除位于 List&lt;Of R&gt; 结尾处的节点
				/// </summary>
				bool RemoveLast()
				{
					return this->RemoveAt(this->Count - 1);
				} 
				/// <summary>
				/// 获取 List&lt;Of R&gt; 中指定位置的元素
				/// </summary>
				R &operator [](intptr_t order) const 
				{
					return this->Get(order);
				}
				/// <summary>
				/// 获取 List&lt;Of R&gt; 中指定顺序(从0开始)的元素
				/// </summary>
				R &Get(intptr_t order) const
				{
					if (order <= 0) {
						assert(order == 0 || !"List<Of R> overflow");
						return this->pFirstNode != nullptr ?
							this->pFirstNode->Node : *reinterpret_cast<R *>(nullptr);
					} else if (order >= (this->Count - 1)) {
						assert(order == (this->Count - 1) || !"List<Of R> overflow");
						return this->pLastNode != nullptr ?
							this->pLastNode->Node : *reinterpret_cast<R *>(nullptr);
					} //if

					ListNodePointer p;
					RLIB_LIST_LOCK;
					// 如果order位于下半段, 采用逆寻方式
					if (this->Count >= 8)
					{
						intptr_t mid_offset = static_cast<intptr_t>((this->Count - 1) >> 1);
						if (order > mid_offset) {
							p     = this->pLastNode;
							order = (this->Count - 1) - order;
							while (order > 0) {
								--order;
								p = p->pPrev;
							}
							goto __next;
						} //if
					} //if
					p = this->pFirstNode;
					while (order > 0) {
						--order;
						p = p->pNext;
					}
__next:
					RLIB_LIST_UNLOCK;
					assert(p != nullptr);
					return p->Node; 
				} 
				/// <summary>
				/// 判断获取到的对象是否为空引用
				/// </summary>
				bool IsNull(const R &Node) const
				{
					return &Node == nullptr;
				}
				/// <summary>
				/// 获取 List&lt;Of R&gt; 中指定顺序(从0开始)的元素的链节点指针
				/// </summary>
				ListNodePointer GetListNode(intptr_t order) const
				{
					if (order <= 0) {
						assert(order == 0 || !"List<Of R> overflow");
						return this->pFirstNode != nullptr ? this->pFirstNode : nullptr;
					} else if (order >= (this->Count - 1)) {
						assert(order == (this->Count - 1) || !"List<Of R> overflow");
						return this->pLastNode != nullptr ? this->pLastNode : nullptr;
					} //if

					ListNodePointer p;
					RLIB_LIST_LOCK;
					// 如果order位于下半段, 采用逆寻方式
					if (this->Count >= 8) {
						intptr_t mid_offset = static_cast<intptr_t>((this->Count - 1) >> 1);
						if (order > mid_offset) {
							p     = this->pLastNode;
							order = (this->Count - 1) - order;
							while (order > 0) {
								--order;
								p = p->pPrev;
							}
							goto __next;
						} //if
					} //if

					p = this->pFirstNode;
					while (order > 0) {
						--order;
						p = p->pNext;
					}

__next:
					RLIB_LIST_UNLOCK;
					assert(p != nullptr);
					return p; 
				} 
				/// <summary>
				/// 将指定范围中元素的顺序反转
				/// </summary>
				/// <param name="order">从0开始的顺序位置</param>
				/// <param name="count">交换次数, 默认0为全部</param>
				void Reverse(intptr_t order = 0, intptr_t count = 0)
				{
					if (order < 0) {
						order = 0;
					} else if (order >= (this->Count - 1)) {
						return;
					} //if

					if (count <= 0 || count > this->Count) {
						count = this->Count >> 1;
					} //if

					RLIB_LIST_LOCK;
					{
						ListNodePointer pLeft = this->pFirstNode, pRight = this->pLastNode;
						while(--order >= 0)
						{
							if (pLeft == nullptr) return;
							pLeft = pLeft->pNext;
						}

						if (pLeft->pPrev == nullptr) this->pFirstNode = pRight;
						if (pRight->pNext == nullptr) this->pLastNode = pLeft;

						while(pLeft != pRight && --count >= 0)
						{
							swap(pLeft, pRight);
							auto temp = pLeft->pPrev;
							pLeft     = pRight->pNext;
							pRight    = temp;
						}
					}
					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 List&lt;Of R&gt; 中第一个匹配项的从零开始的位置顺序
				/// </summary>
				/// <returns>失败返回-1</returns>
				intptr_t IndexOf(const R &item, typename IComparer<R>::EqualsDelegate equals) const
				{
					intptr_t result = 0;
					RLIB_LIST_LOCK;
					{
						ListNodePointer p = this->pFirstNode;
						while(p != nullptr)
						{
							if (equals(&p->Node, &item)) {
								goto __found_it;
							} //if
							p = p->pNext;
							++result;
						}
						result = -1;
					}
__found_it:
					RLIB_LIST_UNLOCK;
					return result;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 List&lt;Of R&gt; 中第一个匹配项的从零开始的位置顺序
				/// </summary>
				/// <returns>失败返回-1</returns>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals> 
				intptr_t IndexOf(const R &item) const
				{
					return this->IndexOf(item, equals);
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 List&lt;Of R&gt; 中最后一个匹配项的从零开始的位置顺序
				/// </summary>
				/// <returns>失败返回-1</returns>
				intptr_t LastIndexOf(const R &item, typename IComparer<R>::EqualsDelegate equals) const
				{
					intptr_t result = 0;
					RLIB_LIST_LOCK;
					{
						ListNodePointer p = this->pLastNode;
						while(p != nullptr)
						{
							if (equals(&p->Node, &item)) {
								result = (this->Count - 1) - result; 
								goto __found;
							} //if
							p = p->pPrev;
							++result;
						}
						result = -1;
					}
__found:
					RLIB_LIST_UNLOCK;
					return result;
				}
				/// <summary>
				/// 搜索指定的对象，并返回整个 List&lt;Of R&gt; 中最后一个匹配项的从零开始的位置顺序
				/// </summary>
				/// <returns>失败返回-1</returns>
				template<typename IComparer<R>::EqualsDelegate equals = IComparer<R>::Equals>
				intptr_t LastIndexOf(const R &item) const
				{
					return this->LastIndexOf(item, equals);
				}
				/// <summary>
				/// 删除 List&lt;Of R&gt; 中所有重复的相邻元素
				/// </summary>
				void Unique()
				{	
					// erase each element matching previous
					ListNodePointer _Pprev = this->pFirstNode;
					ListNodePointer _Pnode = _Pprev->pNext;

					while (_Pnode != nullptr)
						if (_Pprev->Node == _Pnode->Node) {
							// match, remove it
							const ListNodePointer _Perase = _Pnode;
							_Pnode = _Pnode->pNext;

							_Pprev->pNext = _Pnode;
							_Pnode->pPrev = _Pprev;

							delListNode(_Perase);
							--this->Count;
						} else {	// no match, advance
							_Pprev = _Pnode;
							_Pnode = _Pnode->pNext;
						} //if
				}
				/// <summary>
				/// 对 List&lt;Of R&gt; 进行简单选择排序
				/// </summary>
				void Sort(typename IComparer<R>::Delegate comparer = IComparer<R>::Compare, intptr_t begin = 0, intptr_t count = -1)
				{	
					if (begin < 0 || begin >= this->Length || count == 0) return;
					RLIB_LIST_LOCK;
					intptr_t length = this->Count - begin;
					if (count < 0 || count > length) count = length;

					ListNodePointer pBeginNode = this->pFirstNode;
					while(begin > 0)
					{
						assert(pBeginNode != nullptr);
						pBeginNode = pBeginNode->pNext;
						--begin;
					}
					ListNodePointer pNode, pSelectedNode;
					intptr_t t_count;
					do 
					{
						pNode         = pBeginNode;
						pSelectedNode = pBeginNode;
						t_count       = count;
						while (pNode != nullptr && t_count > 0) {
							if (pNode != pSelectedNode && comparer(&pNode->Node, &pSelectedNode->Node) < 0) {
								pSelectedNode = pNode;
							} //if
							pNode = pNode->pNext;
							--t_count;
						}
						if (pSelectedNode != pBeginNode) {
							if (pBeginNode == pFirstNode) {
								pFirstNode = pSelectedNode;
							} //if 

							swap(pBeginNode, pSelectedNode);

							if (pSelectedNode == pLastNode) {
								pLastNode = pBeginNode;
							} //if
						} //if
						pBeginNode = pSelectedNode->pNext;
						--count;
					} while (pBeginNode != nullptr && count > 0);
					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// 对 List&lt;Of R&gt; 进行简单选择排序
				/// </summary>
				template<typename comparer = IComparer<R>> void Sort(intptr_t begin = 0, intptr_t count = -1)
				{
					return this->Sort(comparer::Compare, begin, count);
				}
				/// <summary>
				/// 将 List&lt;Of R&gt; 完全拷贝至指定集合处
				/// </summary>
				template<class collection_t> void CopyTo(collection_t &obj) const
				{
					RLIB_LIST_LOCK;
					ListNodePointer p = this->pFirstNode;
					while (p != nullptr)
					{
						obj.Add(p->Node);
						p = p->pNext;
					}
					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// 将 List&lt;Of R&gt; 完全转移至另一 List&lt;Of R&gt; 处
				/// </summary>
				void MoveTo(List<R> &obj)
				{
					RLIB_LIST_LOCK;

					// if there is cache
					if (this->pCacheNodes && this->nCacheIndex >= 0) {
						if (obj.pCacheNodes) {
							trace(!"this operation cannot be performed because cache exist");
							RLIB_LIST_UNLOCK;
							return;
						} //if

						// obj becomes the cache manager
						obj.pCacheNodes = this->pCacheNodes;
						obj.nCacheTotal = this->nCacheTotal;
						obj.nCacheIndex = this->nCacheIndex;
						this->pCacheNodes = nullptr;
						this->nCacheTotal = 0;
					} //if

					if (obj.pFirstNode == nullptr) {
						assert(obj.pLastNode == nullptr);
						assert(obj.Count == 0);
						obj.pFirstNode = this->pFirstNode;
						obj.pLastNode  = this->pLastNode;
						obj.Count      = this->Count;
					} else {
						assert(obj.pLastNode != nullptr);
						obj.link(obj.pLastNode, this->pFirstNode);
						obj.pLastNode = this->pLastNode;
						obj.Count    += this->Count;
					} //if

					// reset
					this->pFirstNode = this->pLastNode = nullptr;
					this->Count = 0;

					RLIB_LIST_UNLOCK;
				}

			public:
				/// <summary>
				/// Moves the specified element to the first place
				/// </summary>
				void MoveFirst(ListNodePointer pNode)
				{
					assert(pNode != nullptr);
					assert(pNode->pOwner == this);

					RLIB_LIST_LOCK;
					if (pNode->pPrev == nullptr) {
						assert(this->pFirstNode == pNode);
						// already the first
					} else if (pNode->pNext == nullptr) {
						assert(this->pFirstNode != pNode);
						assert(this->pLastNode == pNode);
						this->pLastNode     = pNode->pPrev;
						pNode->pPrev->pNext = nullptr;
						pNode->pPrev        = nullptr;
						link(pNode, this->pFirstNode);
						this->pFirstNode    = pNode;
						assert(this->pLastNode != nullptr);
					} else {
						link(pNode->pPrev, pNode->pNext);
						link(pNode, this->pFirstNode);
						pNode->pPrev     = nullptr;
						this->pFirstNode = pNode;
					} //if

					RLIB_LIST_UNLOCK;
				}
				/// <summary>
				/// Moves the specified element to the last place
				/// </summary>
				void MoveLast(ListNodePointer pNode)
				{
					assert(pNode != nullptr);
					assert(pNode->pOwner == this);

					RLIB_LIST_LOCK;
					if (pNode->pNext == nullptr) {
						assert(this->pLastNode == pNode);
						// already the last
					} else if (pNode->pPrev == nullptr) {
						assert(this->pFirstNode == pNode);
						assert(this->pLastNode != pNode);
						this->pFirstNode    = pNode->pNext;
						pNode->pNext->pPrev = nullptr;
						pNode->pNext        = nullptr;
						link(this->pLastNode, pNode);
						this->pLastNode = pNode;
						assert(this->pFirstNode != nullptr);
					} else {
						link(pNode->pPrev, pNode->pNext);
						link(this->pLastNode, pNode);
						pNode->pNext     = nullptr;
						this->pLastNode  = pNode;
					} //if

					RLIB_LIST_UNLOCK;
				}

			public: // iteration
				typedef ListIterator<R, ListNodePointer> Iterator;
				/// <summary>
				/// Returns an iterator to the first element of the container.
				/// If the container is empty or nothing, the returned iterator will be equal to end()
				/// </summary>
				Iterator begin() /* noexcept */ {
					return Iterator { this != nullptr ? this->pFirstNode : nullptr };
				}
				const Iterator begin() const /* noexcept */ {
					return Iterator { this != nullptr ? this->pFirstNode : nullptr };
				}
				/// <summary>
				/// Returns an iterator to the element following the last element of the container.
				/// This element acts as a placeholder; attempting to access it results in undefined behavior.
				/// </summary>
				Iterator end() /* noexcept */ {
					return Iterator { nullptr };
				}
				const Iterator end() const /* noexcept */ {
					return Iterator { nullptr };
				}
			};
		}
	}
}

#endif // _USE_LIST
