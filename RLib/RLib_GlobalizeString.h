/********************************************************************
	Created:	2015/01/29  19:26
	Filename: 	RLib_GlobalizeString.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

namespace System
{
	class RLIB_API GlobalizeString
	{
	private:
		template<typename char_t> struct StringInfo
		{
			char_t   *pstr;
			intptr_t  size;
		public:
			RLIB_FORCE_INLINE bool isReleasable() const {
				return this->pstr != nullptr/* && this->size > 0*/;
			}
			RLIB_FORCE_INLINE intptr_t getSize() const {
				return size;
			}
		};

	protected:
		String m_str;
#pragma warning(push)
#pragma warning(disable:4201)
		union
		{
			StringInfo<void> m_data[3];
			struct
			{
				mutable StringInfo<wchar_t> unicode;
				mutable StringInfo<char> gbk;
				mutable StringInfo<char> utf8;
			};
		};
#pragma warning(pop)

	protected:
		void release_all();

	public:
		GlobalizeString();
		GlobalizeString(const String &);
		explicit GlobalizeString(LPCTSTR lpctstr, intptr_t length);
		~GlobalizeString();
		RLIB_DECLARE_DYNCREATE;

	public:
		/// <summary>
		/// set string
		/// </summary>
		GlobalizeString &operator = (const String &str);
		/// <summary>
		/// return as gbk string
		/// </summary>
		operator char *() const {
			return this->toGBK();
		}
		/// <summary>
		/// return as unicode string
		/// </summary>
		operator wchar_t *() const {
			return this->toUnicode();
		}
		/// <summary>
		/// return internal string
		/// </summary>
		const String &toString() const {
			return this->m_str;
		}
		/// <summary>
		/// return as gbk string
		/// </summary>
		char *toGBK() const;
		/// <summary>
		/// return as unicode string
		/// </summary>
		wchar_t *toUnicode() const;
		/// <summary>
		/// return as utf-8 string
		/// </summary>
		char *toUtf8() const;
		/// <summary>
		/// get the size of gbk string return by
		/// toGBK(), in bytes, not null terminator included.
		/// </summary>
		intptr_t sizeofGBK() const;
		/// <summary>
		/// get the size of unicode string return by
		/// toUnicode(), in bytes, not null terminator included.
		/// </summary>
		intptr_t sizeofUnicode() const;
		/// <summary>
		/// get the size of utf8 string return by
		/// toUtf8(), in bytes, not null terminator included.
		/// </summary>
		intptr_t sizeofUtf8() const;
		/// <summary>
		/// 取消对返回的数据进行自动释放
		/// 因此, 必须手动调用 String::Collect() 方法来释放内存
		/// </summary>
		template<typename char_t> char_t *SuppressFinalize(char_t *pstr) const {
			for each(auto info in this->m_data)
			{
				if (info.pstr == pstr) {
					if (info.isReleasable()) {
						info.pstr = nullptr;
						return pstr;
					} //if
				} //if
			} //for
			//any case, there was an error
			trace(!"failed to SuppressFinalize!");
			return nullptr;
		}

	public:
		/// <summary>
		/// 转换为多字符格式字符串
		/// 必须手动调用 String::Collect() 方法以释放内存
		/// </summary>
		/// <param name="out_size">不包括 \0 结尾符大小, in bytes</param>
		static LPSTR ConvertToMultiByte(LPCWSTR, intptr_t length = -1, intptr_t *out_size = nullptr,
										Text::Encoding to_codepage = Text::ASCIIEncoding);
		/// <summary>
		/// 转换为宽字符格式字符串
		/// 必须手动调用 String::Collect() 方法以释放内存
		/// </summary>
		/// <param name="out_size">不包括 \0 结尾符大小, in bytes</param>
		static LPWSTR ConvertToWideChar(LPCSTR, intptr_t length = -1, intptr_t *out_size = nullptr,
										Text::Encoding from_codepage = Text::ASCIIEncoding);
	};
}