/********************************************************************
	Created:	2016/07/20  22:22
	Filename: 	RLib_StringConvHelper.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"
#include "RLib_Native.h"
#include "RLib_StringConvHelper.h"

#ifndef _ENABLE_RTL_CONV
# ifdef _UNICODE
#  define _tcreate_locale _wcreate_locale
# else
#  define _tcreate_locale _create_locale
# endif // _UNICODE

# include "RLib_Interlocked.h"
# ifdef _DEBUG
#  include <debugapi.h>
# endif // _DEBUG
using namespace System::Threading;

//-------------------------------------------------------------------------

volatile _locale_t &RT2::getlocale(_In_z_ const TCHAR *_Locale /* = _T("chs") */)
{
	static volatile _locale_t s_locale = NULL;
	if (s_locale == NULL && _Locale) {
		_locale_t loc = _tcreate_locale(LC_CTYPE, _Locale);
		if (Interlocked::CompareExchangePointer<PVOID>(reinterpret_cast<volatile PVOID *>(&s_locale), loc, NULL) != NULL) {
			_free_locale(loc);
		} //if
#ifdef _DEBUG
		if (s_locale != NULL && AppBase::IsDebuggerPresent()) {
			OutputDebugString(RLIB_NEWLINE _T("* locale created at ") RLIB_FUNCTION _T(", use freelocale to destroy it.") RLIB_NEWLINE);
		} //if
#endif // _DEBUG
	} //if
	return s_locale;
}

//-------------------------------------------------------------------------

void RT2::freelocale() 
{
	auto loc = Interlocked::ExchangePointer<PVOID>(reinterpret_cast<volatile PVOID *>(&getlocale(NULL)), NULL);
	if (loc != NULL) {
		_free_locale(static_cast<_locale_t>(loc));
	} //if
}
#endif // !_ENABLE_RTL_CONV

//-------------------------------------------------------------------------

intptr_t RT2::UnicodeToMultiByte(_Out_ PCHAR MultiByteString,
								 _In_ size_t MaxBytesInMultiByteString,
								 _In_ PCWCH UnicodeString,
								 _In_ size_t BytesInUnicodeString)
{
	ULONG sizeInBytes;
	RtlUnicodeToMultiByteN(MultiByteString, static_cast<ULONG>(MaxBytesInMultiByteString),
						   &sizeInBytes, 
						   UnicodeString, static_cast<ULONG>(BytesInUnicodeString));
	return static_cast<intptr_t>(sizeInBytes);
}

//-------------------------------------------------------------------------

intptr_t RT2::UnicodeToMultiByteSize(_In_ PCWCH UnicodeString,
									 _In_ size_t BytesInUnicodeString)
{
	ULONG sizeInBytes;
	RtlUnicodeToMultiByteSize(&sizeInBytes, UnicodeString, 
							  static_cast<ULONG>(BytesInUnicodeString));
	return static_cast<intptr_t>(sizeInBytes);
}

//-------------------------------------------------------------------------

intptr_t RT2::MultiByteToUnicode(_Out_ PWCH UnicodeString,
								 _In_ size_t MaxBytesInUnicodeString,
								 _In_ const CHAR *MultiByteString,
								 _In_ size_t BytesInMultiByteString)
{
	ULONG sizeInBytes;
	RtlMultiByteToUnicodeN(UnicodeString, static_cast<ULONG>(MaxBytesInUnicodeString),
						   &sizeInBytes,
						   MultiByteString, static_cast<ULONG>(BytesInMultiByteString));
	return static_cast<intptr_t>(sizeInBytes);
}

//-------------------------------------------------------------------------

intptr_t RT2::MultiByteToUnicodeSize(_In_ const CHAR *MultiByteString,
									 _In_ size_t BytesInMultiByteString)
{
	ULONG sizeInBytes;
	RtlMultiByteToUnicodeSize(&sizeInBytes, MultiByteString, 
							  static_cast<ULONG>(BytesInMultiByteString));
	return static_cast<intptr_t>(sizeInBytes);
}