/********************************************************************
	Created:	2016/07/28  9:44
	Filename: 	RLib_WebHeader.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

namespace System
{
	namespace Net
	{
		/// <summary>
		/// 包含与请求或响应关联的协议标头
		/// </summary>
		class RLIB_API WebHeaderCollection
		{
		public:
			/// <summary>
			/// 计算并返回标头数量
			/// </summary>
			RLIB_PROPERTY_GET(const intptr_t Count, GetCount);
			/// <summary>
			/// 计算并返回标头数量
			/// </summary>
			intptr_t GetCount();
			/// <summary>
			/// 将具有指定名称和值的标头插入到集合中
			/// </summary>
			void Add(LPCSTR name, LPCSTR val);
			/// <summary>
			/// 将具有指定名称和值的标头插入到集合中
			/// </summary>
			void Add(LPCSTR name, intptr_t len_name, LPCSTR val, intptr_t len_val);
			/// <summary>
			/// 获取集合中特定标头的值，该值由标头名指定
			/// </summary>
			String Get(LPCSTR name);
			/// <summary>
			/// 获取集合中特定标头的值，该值由标头名指定
			/// </summary>
			String Get(LPCSTR name, intptr_t len_name);
			/// <summary>
			/// 从集合中移除所有标头
			/// </summary>
			void Clear();
			/// <summary>
			/// 将 WebHeaderCollection 转换为字节数组, 并返回临时内存地址
			/// </summary>
			char *ToByteArray();
			/// <summary>
			/// 获取将 WebHeaderCollection 转换为字节数组后的大小
			/// </summary>
			intptr_t GetByteArraySize() {
				return this->m_headers.Length;
			}
			/// <summary>
			/// 获取 WebHeaderCollection 是否为空
			/// </summary>
			bool IsEmpty() {
				return this->m_headers.Length == 0;
			}
			/// <summary>
			/// 将 字节数组 写入 WebHeaderCollection, 末尾不需要换行符
			/// </summary>
			void WriteByteArray(char *pByteArray, intptr_t size);

		public:
			WebHeaderCollection() : m_headers(RLIB_DEFAULT_BUFFER_SIZE * 2) {}
			RLIB_DECLARE_DYNCREATE;

		private:
			IO::BufferedStream m_headers;
		};
	}
}