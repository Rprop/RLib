/********************************************************************
	Created:	2014/12/28  10:31
	Filename: 	RLib_Stack.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_STACK
#define _USE_STACK
#include "RLib_List.h"

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			/// <summary>
			/// 表示对象的简单后进先出 (LIFO) 泛型集合
			/// </summary>
			template <class R> class Stack : List<R>
			{
			public:
				using List<R>::List;
				RLIB_DECLARE_DYNCREATE;
				/// <summary>
				/// 返回位于 Stack&lt;Of R&gt; 顶部的对象但不将其移除
				/// </summary>
				R &Peek()
				{
					R &_temp = this->Get(0);
					assert(!this->IsNull(_temp));
					return _temp;
				}
				/// <summary>
				/// 移除并返回位于 Stack&lt;Of R&gt; 顶部的对象
				/// </summary>
				R Pop()
				{
					R &obj_ref = this->Get(0);
					assert(!this->IsNull(obj_ref));
					R obj = obj_ref;
					this->RemoveFirst();
					return obj;
				}
				/// <summary>
				/// 将对象插入 Stack&lt;Of R&gt; 的顶部
				/// </summary>
				bool Push(const R &item)
				{
					return this->AddFirst(item) != nullptr;
				}
				/// <summary>
				/// 获取 Stack&lt;Of R&gt; 中包含的元素数
				/// </summary>
				const intptr_t GetLength() {
					return List<R>::Length;
				}
				/// <summary>
				/// 获取 Stack&lt;Of R&gt; 中包含的元素数
				/// </summary>
				RLIB_PROPERTY_GET(const intptr_t Count, GetLength);
				/// <summary>
				/// 清空栈
				/// </summary>
				void Clear() {
					List<R>::Clear();
				}
			};
		}
	}
}

#endif // _USE_STACK