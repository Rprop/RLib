/********************************************************************
	Created:	2014/12/28  10:35
	Filename: 	RLib_Queue.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_QUEUE
#define _USE_QUEUE
#include "RLib_List.h"

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			/// <summary>
			/// 表示对象的先进先出集合
			/// </summary>
			template <class R> class Queue : List<R>
			{
			public:
				using List<R>::List;
				RLIB_DECLARE_DYNCREATE;

			public:
				/// <summary>
				/// 获取 Queue&lt;Of R&gt; 中包含的元素数
				/// </summary>
				const intptr_t GetLength() {
					return List<R>::Length;
				}
				/// <summary>
				/// 获取 Queue&lt;Of R&gt; 中包含的元素数
				/// </summary>
				RLIB_PROPERTY_GET(const intptr_t Count, GetLength);
				/// <summary>
				/// 返回位于 Queue&lt;Of R&gt; 顶部的对象但不将其移除
				/// </summary>
				R &Peek() const
				{
#ifdef _DEBUG
					R &_temp = this->Get(0);
					assert(!this->IsNull(_temp));
					return _temp;
#else
					return this->Get(0);
#endif // _DEBUG

				}
				/// <summary>
				/// 移除并返回位于 Queue&lt;Of R&gt; 开始处的对象
				/// </summary>
				R Dequeue()
				{
					R &obj_ref = this->Get(0);
					assert(!this->IsNull(obj_ref));

					R obj(obj_ref);
					this->RemoveFirst();
					return obj;
				}
				/// <summary>
				/// 将对象添加到 Queue&lt;Of R&gt; 的结尾处
				/// </summary>
				bool Enqueue(R item)
				{
					return this->AddLast(item) != nullptr;
				}
				/// <summary>
				/// 清空队列
				/// </summary>
				void Clear() 
				{
					List<R>::Clear();
				}
			};
		}
	}
}

#endif // _USE_QUEUE