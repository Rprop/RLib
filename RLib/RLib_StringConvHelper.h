/********************************************************************
	Created:	2016/07/15  13:10
	Filename: 	RLib_StringConvHelper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#define _ENABLE_RTL_CONV // uses Rtl functions to perform conversion

/*
 *	string conversion macros 
 */
#define RA2WL(s,l)    RA2WEX<>(s,l)
#define RW2AL(s,l)    RW2AEX<>(s,l)
#ifdef _UNICODE
# define RT2A(s)      RW2A(s)
# define RT2A_S(s,c)  RW2A_S(s,c)
# define RT2W(s)      static_cast<LPCWSTR>(s)
# define RT2W_S(s,c)  static_cast<LPCWSTR>(s)
#else
# define RT2A(s)      static_cast<LPCSTR>(s)
# define RT2A_S(s,c)  static_cast<LPCSTR>(s)
# define RT2W(s)      RA2W(s)
# define RT2W_S(s,c)  RA2W_S(s,c)
#endif // _UNICODE

//-------------------------------------------------------------------------

namespace System
{
	class RLIB_API RT2
	{
#ifdef _ENABLE_RTL_CONV
	protected:
		size_t m_sizeConverted; // in bytes, exclude '\0'
#else
	protected:
		size_t m_charsConverted; // in chars, include '\0'

	public:
		/// <summary>
		/// Gets the length of string converted, in chars,
		/// not null terminator included.
		/// </summary>
		RLIB_INLINE intptr_t length() const {
			assert(this->m_charsConverted >= 1);
			return this->m_charsConverted - 1;
		}

	public:
		static volatile _locale_t &getlocale(_In_z_ const TCHAR *_Locale = _T("chs"));
		static void freelocale();
#endif // _ENABLE_RTL_CONV

	public:
		/// <summary>
		/// Translates the specified Unicode string into a new character string, using the current system ANSI code page (ACP).
		/// It returns a null-terminated multibyte string if the given BytesInUnicodeString included a NULL terminator and if the given MaxBytesInMultiByteString did not cause truncation.
		/// </summary>
		/// <returns>receives the length, in bytes, of the translated string</returns>
		static intptr_t UnicodeToMultiByte(_Out_ PCHAR MultiByteString,
										   _In_ size_t MaxBytesInMultiByteString,
										   _In_ PCWCH UnicodeString,
										   _In_ size_t BytesInUnicodeString);
		/// <summary>
		/// Determines the number of bytes that are required to store the multibyte translation for the specified Unicode string. 
		/// The returned value does not include space for a NULL terminator for the ANSI string.
		/// The translation is assumed to use the current system ANSI code page (ACP).
		/// </summary>
		/// <returns>receives the number of bytes required to store the translated string</returns>
		static intptr_t UnicodeToMultiByteSize(_In_ PCWCH UnicodeString,
											   _In_ size_t BytesInUnicodeString);
		/// <summary>
		/// Translates the specified source string into a Unicode string, using the current system ANSI code page (ACP). 
		/// The returned Unicode string is not null-terminated
		/// </summary>
		/// <returns>receives the length, in bytes, of the translated string</returns>
		static intptr_t MultiByteToUnicode(_Out_ PWCH UnicodeString,
										   _In_ size_t MaxBytesInUnicodeString,
										   _In_ const CHAR *MultiByteString,
										   _In_ size_t BytesInMultiByteString);

		/// <summary>
		/// Determines the number of bytes that are required to store the Unicode translation for the specified source string.
		/// The returned value does not include space for a NULL terminator for the Unicode string.
		/// The translation is assumed to use the current system ANSI code page (ACP)
		/// </summary>
		/// <returns>receives the number of bytes that are required to store the translated string</returns>
		static intptr_t MultiByteToUnicodeSize(_In_ const CHAR *MultiByteString,
											   _In_ size_t BytesInMultiByteString);
	};
}

#ifdef _ENABLE_RTL_CONV
# include "RLib_StringConvRtl.h"
#else
# include "RLib_StringConv.h"
#endif // _ENABLE_RTL_CONV
