/********************************************************************
	Created:	2014/10/12  15:50
	Filename: 	RLib_StringHelper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_Utility.h"
#include <stdio.h>

#define TLEN(s)				   static_cast<intptr_t>(_tcslen(s))
#ifdef _UNICODE
# define _R(a)				   (System::STRInfoW { RLIB_STR_LEN(_T(a)) })
# define RSTR(a)			   (System::STRInfoW { RLIB_STR_LEN(_T(a)) })
# define _RT(a)			       (System::STRInfoW { RLIB_STR_LEN(a) })
# define TSTR(a)			   (System::STRInfoW { RLIB_STR_LEN(a) })
# define _RC(a,b)			   (System::STRInfoW { a, static_cast<intptr_t>(b) })
#else
# define _R(a)				   (System::STRInfoA { RLIB_STR_LEN(a) })
# define RSTR(a)			   (System::STRInfoA { RLIB_STR_LEN(a) })
# define _RT(a)				   (System::STRInfoA { RLIB_STR_LEN(a) })
# define TSTR(a)			   (System::STRInfoA { RLIB_STR_LEN(a) })
# define _RC(a,b)			   (System::STRInfoA { a, static_cast<intptr_t>(b) })
#endif // _UNICODE
#define TSIZE(len)			   static_cast<intptr_t>((len + 1) * RLIB_SIZEOF(TCHAR))
#define TSIZE_ALIGNED(len)	   RLIB_ROUNDUP(TSIZE(len))

// useful macro
#ifdef _UNICODE
# define LOCAL_NEW LOCAL_NEWW
#else 
# define LOCAL_NEW LOCAL_NEWA
#endif // _UNICODE
#define LOCAL_NEWA(a,n)               char  a[n]
#define LOCAL_NEWW(a,n)               TCHAR a[n]
#define LOCAL_INIT(b)                 intptr_t _length_##b = 0
#define LOCAL_LENGTH(b)               _length_##b
#define LOCAL_EMPTY(b)                _length_##b = 0; b[0] = 0;
#define __LOCAL_APPEND(v,b,f,...)     _length_##b += v(&(b[_length_##b]), static_cast<size_t>(RLIB_COUNTOF(b) - _length_##b), static_cast<size_t>(RLIB_COUNTOF(b) - 1 - _length_##b), f, __VA_ARGS__)
#define __LOCAL_APPEND_L(v,b,f,l,...) _length_##b += v(&(b[_length_##b]), static_cast<size_t>(RLIB_COUNTOF(b) - _length_##b), static_cast<size_t>(RLIB_COUNTOF(b) - 1 - _length_##b), f, l, __VA_ARGS__)
#define LOCAL_APPEND(b,f,...)         __LOCAL_APPEND(_sntprintf_s, b, f, __VA_ARGS__)
#define LOCAL_APPENDA(b,f,...)        __LOCAL_APPEND(_snprintf_s, b, f, __VA_ARGS__)
#define LOCAL_APPENDW(b,f,...)        __LOCAL_APPEND(_snwprintf_s, b, f, __VA_ARGS__)
#define LOCAL_APPEND_L(b,f,l,...)     __LOCAL_APPEND_L(_sntprintf_s_l, b, f, l, __VA_ARGS__)
#define LOCAL_APPENDA_L(b,f,l,...)    __LOCAL_APPEND_L(_snprintf_s_l, b, f, l, __VA_ARGS__)
#define LOCAL_APPENDW_L(b,f,l,...)    __LOCAL_APPEND_L(_snwprintf_s_l, b, f, l, __VA_ARGS__)

/*
 *	Convert a writable string pointer to String object without copying
 */
#define StringReference(p)     System::String(static_cast<LPCTSTR>(p))
/*
 *	Provide support for convert other string object(CString, std::string, etc) to String
 */
#define StringFromCString(a)   const_cast<LPTSTR>(static_cast<LPCTSTR>(a))
/*
 *	Append to String
 */
#define StringAppend(s,a)      s.append(RLIB_STR_LEN(a))
/*
 *	Copy to String
 */
#define StringCopy(s,a)        s.Copy(RLIB_STR_LEN(a))
/*
 *	Copy String object to a buffer (bytes)
 */
#define StringCopyTo(s,c)  { const String &_string_ = s; \
							 auto _size_of_ = static_cast<size_t>(_string_.CanReadSize); \
							 static_assert(sizeof(c) >= sizeof(TCHAR), "overflow"); \
							 if (_size_of_ > sizeof(c) - sizeof(TCHAR)) _size_of_ = sizeof(c) - sizeof(TCHAR); \
							 memcpy(c, _string_.GetConstData(), _size_of_); \
							 memset(reinterpret_cast<unsigned char *>(c) + _size_of_, 0, sizeof(c) - _size_of_); \
}
/*
 *	Convert String object to GBK/GB2312 string and copy to a buffer (bytes)
 */
#define StringCopyToA(s,c) { const String &_string_ = s; \
							 auto _size_of_ = static_cast<size_t>(0UL); \
							 static_assert(sizeof(c) >= sizeof(char), "overflow"); \
							 if (!_string_.IsNull()) { \
									GlobalizeString _g_string_(_string_); \
									_size_of_ = _g_string_.sizeofGBK(); \
									if (_size_of_ > sizeof(c) - sizeof(char)) _size_of_ = sizeof(c) - sizeof(char); \
									memcpy(c, _g_string_.toGBK(), _size_of_); \
							 } \
							 *(reinterpret_cast<char *>(c) + _size_of_) = 0; \
}
/*
 *	Convert String object to UNICODE string and copy to a buffer (bytes)
 */
#define StringCopyToW(s,c) { const String &_string_ = s; \
							 auto _size_of_ = static_cast<size_t>(0UL); \
							 static_assert(sizeof(c) >= sizeof(wchar_t), "overflow"); \
							 if (!_string_.IsNull()) { \
									GlobalizeString _g_string_(_string_); \
									_size_of_ = static_cast<size_t>(_g_string_.sizeofUnicode()); \
									if (_size_of_ > sizeof(c) - sizeof(wchar_t)) _size_of_ = sizeof(c) - sizeof(wchar_t); \
									memcpy(c, _g_string_.toUnicode(), _size_of_); \
							 } \
							 *reinterpret_cast<wchar_t *>(reinterpret_cast<char *>(c) + _size_of_) = 0; \
}
/*
 *	Determines whether the beginning of this string matches the specified string (length = sizeof(short))
 */
#define StringStartWith_2_A(a,b) (*reinterpret_cast<const short *>(a) == *reinterpret_cast<const short *>(b))
/*
 *	Determines whether the beginning of this string matches the specified string (length = sizeof(int))
 */
#define StringStartWith_2_W(a,b) (*reinterpret_cast<const int *>(a) == *reinterpret_cast<const int *>(b))
/*
 *	Determines whether the beginning of this string matches the specified string
 */
#ifdef _UNICODE
#define StringStartWith_2 StringStartWith_2_W
#else
#define StringStartWith_2 StringStartWith_2_A
#endif // _UNICODE
/*
 *	Determines whether the beginning of this string matches the specified string (length = sizeof(int))
 */
#define StringStartWith_4_A(a,b) (*reinterpret_cast<const int *>(a) == *reinterpret_cast<const int *>(b))
/*
 *	Determines whether the beginning of this string matches the specified string (length = sizeof(__int64))
 */
#define StringStartWith_4_W(a,b) (*reinterpret_cast<const __int64 *>(a) == *reinterpret_cast<const __int64 *>(b))
/*
 *	Determines whether the beginning of this string matches the specified string
 */
#ifdef _UNICODE
#define StringStartWith_4 StringStartWith_4_W
#else
#define StringStartWith_4 StringStartWith_4_A
#endif // _UNICODE
/*
 *	Determines whether the beginning of this string matches the specified string (length = sizeof(__int64))
 */
#define StringStartWith_8_A(a,b)  (*reinterpret_cast<const __int64 *>(a) == *reinterpret_cast<const __int64 *>(b))
/*
 *	Terminates string by appending null terminator 
 */
#define StringTermAtBytes(s,n)    (*reinterpret_cast<TCHAR *>(reinterpret_cast<char *>(s) + n) = _T('\0'))
#define StringTermAtBytes_A(s,n)  (*(reinterpret_cast<char *>(s) + n) = '\0')
#define StringTermAtBytes_W(s,n)  (*reinterpret_cast<wchar_t *>(reinterpret_cast<char *>(s) + n) = L'\0')
#define StringTermAtLength(s,n)   (*(reinterpret_cast<TCHAR *>(s)+ n) = _T('\0'))
#define StringTermAtLength_A(s,n) (*(reinterpret_cast<char *>(s)+ n) = '\0')
#define StringTermAtLength_W(s,n) (*(reinterpret_cast<wchar_t *>(s)+ n) = L'\0')