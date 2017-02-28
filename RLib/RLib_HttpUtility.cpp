/********************************************************************
	Created:	2015/06/14  9:16
	Filename: 	RLib_HttpUtility.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Object.h"
#include "RLib_Helper.h"
#include "RLib_Fundamental.h"
#include "RLib_HttpUtility.h"
#include "RLib_StringHelper.h"
#include "RLib_StringConvHelper.h"
#include "RLib_UnmanagedStream.h"
#if defined(_DEBUG) && defined(_WINDOWS)
#include <windows.h>
#include <WinBase.h>
#endif // defined(_DEBUG) && defined(_WINDOWS)


using namespace System::IO;
using namespace System::Net;
using namespace System::Text;

//-------------------------------------------------------------------------

String HttpUtility::UrlEncode(const String &str,
							  Text::Encoding codepage /* = Text::UTF8Encoding */)
{
	while (!str.IsNullOrEmpty()) {
#ifdef _UNICODE
		ManagedObject<BufferedStream> stream = Encoder::WideCharTo(str.GetConstData(), str.GetCanReadSize(), 
																   codepage);
#else
		ManagedObject<BufferedStream> stream = Encoder::ToWideChar(str.GetConstData(), str.GetCanReadSize(),
																   Text::ASCIIEncoding);
		if (stream.IsNull()) break;

		stream = Text::Encoder::WideCharTo(codepage, stream.GetInstance());
#endif // _UNICODE
		if (stream.IsNull()) break;

		String dst(str.Length * 2);
		auto lpdata = reinterpret_cast<unsigned char *>(stream->ObjectData);

		for (int i = 0; i < stream->Length; ++i) {
			if (isalnum(static_cast<unsigned char>(lpdata[i]))) {
				dst += static_cast<TCHAR>(lpdata[i]);
			} else
				if (lpdata[i] == ' ') {
					dst += _T('+');
				} else {
					const TCHAR __hex[] = _T("0123456789ABCDEF");
					TCHAR cs[] = {
						_T('%'),
						__hex[static_cast<unsigned char>(lpdata[i]) / 16],
						__hex[static_cast<unsigned char>(lpdata[i]) % 16]
					};
					dst.append(cs, RLIB_COUNTOF(cs));
				} //if
		}

		return dst;
	}

	return str;
}

//-------------------------------------------------------------------------

String HttpUtility::UrlDecode(const String &str, 
							  Text::Encoding codepage /* = Text::UTF8Encoding */)
{
	auto first_encode = str.IndexOf(_T("%"));
	if (first_encode == -1) {
		return str;
	} //if

	String dst(str.Length);
	dst.copy(str, first_encode);

	unsigned char buf[RLIB_DEFAULT_BUFFER_SIZE];
	auto lpbuf    = buf;
#ifdef _UNICODE
	RW2AEX<RLIB_DEFAULT_BUFFER_SIZE> r(str.GetConstData() + first_encode,
									   str.Length - first_encode);
	auto src      = r.toGBK();
	auto src_size = r.sizeofGBK();
#else
	auto src      = static_cast<const char *>(str.GetConstData()) + first_encode;
	auto src_size = str.CanReadSize - first_encode;
#endif // _UNICODE
	if (src_size >= RLIB_COUNTOF(buf)) {
		src_size = RLIB_COUNTOF(buf) - 1;
	} //if

	for (intptr_t i = 0; i < src_size; ++i, ++lpbuf) {
		if (src[i] == '%') {
			if (isxdigit(static_cast<unsigned char>(src[i + 1])) &&
				isxdigit(static_cast<unsigned char>(src[i + 2]))) {
				char c1 = src[++i];
				char c2 = src[++i];
				c1 = c1 - 48 - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
				c2 = c2 - 48 - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);
				lpbuf[0] = static_cast<unsigned char>(c1 * 16 + c2);
			}
		} else {
			if (src[i] == '+') {
				lpbuf[0] = ' ';
			} else {
				lpbuf[0] = static_cast<unsigned char>(src[i]);
			} //if
		} //if
	}

	auto pdata = Text::Encoder::ToCurrentEncoding(buf, lpbuf - buf, codepage);
	if (pdata != nullptr) {
		dst.append(reinterpret_cast<LPTSTR>(pdata->ObjectData),
				   pdata->Size / RLIB_SIZEOF(TCHAR));
		delete pdata;
	} //if

	return dst;
}

//-------------------------------------------------------------------------

RLIB_FORCE_INLINE intptr_t __indexOfHtmlEncodingChars(const wchar_t *s, intptr_t len,
													  intptr_t pos)
{
	auto num = len - pos;
	auto ptr = s + pos;
	while (num > 0) {
		if (ptr[0] <= L'>') {
			switch (ptr[0]) {
			case L'&':
			case L'\'':
			case L'"':
			case L'<':
			case L'>':
				return (len - num);
			}
		} else {
			if ((ptr[0] >= L'\x00a0') && (ptr[0] <= L'\x00ff')) {
				return (len - num);
			}
		}
		++ptr;
		--num;
	}
	return -1;
}

//-------------------------------------------------------------------------

String HttpUtility::HTMLEncode(const String &str)
{
#ifndef _UNICODE
	GlobalizeString ustr(str);
	const wchar_t *lpustr = ustr.toUnicode();
#else
	const wchar_t *lpustr = str.GetConstData();
#endif //_UNICODE
	auto num = __indexOfHtmlEncodingChars(lpustr, str.Length, 0);
	if (num == -1) {
		return str;
	}

	auto pch            = lpustr + num;
	auto charsRemaining = str.Length - num;

	String dst(num + charsRemaining * RLIB_COUNTOF_STR("&#hhhh;"));
	if (num > 0) {
		dst.append(str, num);
	}

	while (charsRemaining > 0) {
		if (pch[0] <= L'>') {
			switch (pch[0])
			{
			case L'&':
				StringAppend(dst, _T("&amp;"));
				break;

			case L'\'':
				StringAppend(dst, _T("&#39;"));
				break;

			case L'"':
				StringAppend(dst, _T("&quot;"));
				break;

			case L'<':
				StringAppend(dst, _T("&lt;"));
				break;

			case L'>':
				StringAppend(dst, _T("&gt;"));
				break;

			default:
				dst.append(pch, 1);
				break;
			}
		} else {
			if ((pch[0] >= L'\x00a0') && (pch[0] <= L'\x00ff')) {
				/*
				 *	&# decimal ;
				 */
				StringAppend(dst, _T("&#"));
				dst.append(Int32(static_cast<int>(pch[0])).ToString());
				StringAppend(dst, _T(";"));
				/*
				 *	&#x hexadecimal ;
				 */
// 				StringAppend(dst, _T("&#x"));
// 				dst.append(Int32(static_cast<int>(pch[0])).ToString(16));
// 				StringAppend(dst, _T(";"));
			} else {
				dst.append(pch, 1);
			}
		}
		
		--charsRemaining;
		++pch;
	}

	return dst;

}

//-------------------------------------------------------------------------

String HttpUtility::HTMLDecode(const String &str)
{
	auto first_encode = str.IndexOf(_T("&"));
	if (first_encode == -1) {
		return str;
	}

#ifdef _UNICODE
	auto lpstr = str.GetConstData() + first_encode;
#else
	// output buffer, unicode format
	ManagedObject<BufferedStream> buffer =
		new BufferedStream((str.Length - first_encode) * RLIB_SIZEOF(wchar_t));
	if (buffer.IsNull()) {
		return str;
	} //if

	// convert to unicode encoding
	auto __lpustr = GlobalizeString::ConvertToWideChar(str.GetConstData() + first_encode,
													   str.Length - first_encode);
	if (__lpustr == nullptr) {
		return str;
	} //if
	AutoFinalize<void *> __finalizer([](void *__lpustr)
	{
		String::Collect(__lpustr);
	}, __lpustr);
	RLIB_RENAME(__lpustr, lpstr);
#endif // _UNICODE

	// result buffer
	String dst(str.Length);
	dst.append(str, first_encode);

	wchar_t numeric[8];
	auto last_pstr = lpstr;
	while ((lpstr = wcsstr(last_pstr, L"&")) != nullptr) {
#ifdef _UNICODE
		dst.append(last_pstr, lpstr - last_pstr);
#else
		buffer->Write(last_pstr, (lpstr - last_pstr) * RLIB_SIZEOF(wchar_t));
#endif // _UNICODE

		lpstr += RLIB_COUNTOF_STR("&");
		if (lpstr[0] != L'#'){
			if (StringStartWith_4_W(lpstr, L"amp;")) {
#ifdef _UNICODE
				StringAppend(dst, _T("&"));
#else
				RLIB_StreamWriteW(buffer, "&");
#endif // _UNICODE
				last_pstr = lpstr + RLIB_COUNTOF_STR("amp;");
			} else if (StringStartWith_4_W(lpstr - RLIB_COUNTOF_STR("&"), L"&lt;")) {
#ifdef _UNICODE
				StringAppend(dst, _T("<"));
#else
				RLIB_StreamWriteW(buffer, "<");
#endif // _UNICODE
				last_pstr = lpstr + RLIB_COUNTOF_STR("lt;");
			} else if (StringStartWith_4_W(lpstr - RLIB_COUNTOF_STR("&"), L"&gt;")) {
#ifdef _UNICODE
				StringAppend(dst, _T(">"));
#else
				RLIB_StreamWriteW(buffer, ">");
#endif // _UNICODE
				last_pstr = lpstr + RLIB_COUNTOF_STR("gt;");
			} else {
				if (StringStartWith_4_W(lpstr, L"quot") &&
					lpstr[RLIB_COUNTOF_STR("quot")] == L';') {
#ifdef _UNICODE
					StringAppend(dst, _T("\""));
#else
					RLIB_StreamWriteW(buffer, "\"");
#endif // _UNICODE
					last_pstr = lpstr + RLIB_COUNTOF_STR("quot;");
				} else {
#ifdef _UNICODE
					StringAppend(dst, _T("&"));
#else
					RLIB_StreamWriteW(buffer, "&");
#endif // _UNICODE
					last_pstr = lpstr;
				} //if
			} //if
			continue;
		} //if

		lpstr += RLIB_COUNTOF_STR("#");
		auto next_pstr = wcsstr(lpstr, L";");
		auto identifier_len = (next_pstr - lpstr);
		if (next_pstr == nullptr || identifier_len > 7) {
			//TODO
#ifdef _UNICODE
			StringAppend(dst, _T("&#"));
#else
			RLIB_StreamWriteW(buffer, "&#");
#endif // _UNICODE
			last_pstr = lpstr;
			continue;
		}
		memcpy(numeric, lpstr, (next_pstr - lpstr) * sizeof(wchar_t));
		numeric[(next_pstr - lpstr)] = L'\0';
		numeric[0] = static_cast<wchar_t>(lpstr[0] != L'x' ?
										  Int32::TryParse(numeric) : Int32::TryParse(numeric + 1, 16));
#ifdef _UNICODE
		dst.append(numeric, 1);
#else
		buffer->Write(numeric, 1 * sizeof(wchar_t));
#endif // _UNICODE

		last_pstr = next_pstr + 1;
	}

	if (last_pstr[0] != L'\0') {
#ifdef _UNICODE
		dst.append(last_pstr, str.Length - (last_pstr - str.GetConstData()));
#else
		buffer->Write(last_pstr,
					  (str.Length - (first_encode + last_pstr - reinterpret_cast<wchar_t *>(__finalizer.argv))) * RLIB_SIZEOF(wchar_t));
#endif // _UNICODE
	} //if

#ifndef _UNICODE
	buffer->Position = 0;
	auto pdata = Text::Encoder::WideCharTo(ASCIIEncoding, *buffer);
	if (pdata != nullptr) {
		dst.append(reinterpret_cast<LPCTSTR>(pdata->ObjectData),
				   pdata->Capacity / RLIB_SIZEOF(TCHAR));
		delete pdata;
	}
#endif // _UNICODE
	return dst;
}

//-------------------------------------------------------------------------

static void __u_to_unicode(wchar_t *lpstr, intptr_t length)
{
	wchar_t uchar;
	auto  lpstart   = lpstr;
	auto  lpbefore  = lpstr;
	auto  lpnext    = reinterpret_cast<RLIB_TYPE(lpstr)>(nullptr);
	while ((lpstart = wcsstr(lpstart, L"\\")) != nullptr) {
		if (lpstart[1] != L'u' && lpstart[1] != L'U'){
			++lpstart;
			continue;
		} //if
#if defined(_DEBUG) && defined(_WINDOWS)
		assert(IsBadWritePtr(lpstart + RLIB_COUNTOF_STR(L"\\u"), sizeof(L"hhhh") - sizeof(uchar)) == FALSE);
#endif // defined(_DEBUG) && defined(_WINDOWS)

		if (!lpnext) {
			lpnext = lpstart;
		} //if

		// aa\uhhhhaa\uhhhh
		if (lpbefore != lpstr && lpbefore != lpstart) {
			// copy non-u required
			memcpy(lpnext, lpbefore, (lpstart - lpbefore) * sizeof(wchar_t));
			lpnext += (lpstart - lpbefore);
		} //if

		lpstart   += RLIB_COUNTOF_STR(L"\\uhhhh");
		uchar      = lpstart[0];
		lpstart[0] = L'\0';

		*reinterpret_cast<int *>(lpnext) = Int32::TryParse(lpstart - RLIB_COUNTOF_STR(L"hhhh"), 16);
		lpnext += RLIB_COUNTOF_STR(L"\\");
		
		lpstart[0] = uchar;
		lpbefore   = lpstart;
	}
	if (lpbefore != lpstr) {
		assert(lpnext != nullptr);
		// copy the rest part required
		memcpy(lpnext, lpbefore, (length - (lpbefore - lpstr) + 1) * sizeof(wchar_t));
	} else {
		if (lpnext) lpnext[0] = L'\0';
	} //if
}

//-------------------------------------------------------------------------

String HttpUtility::UnicodeEscapesDecode(const String &str)
{
#ifndef _UNICODE
	GlobalizeString ustr(str);
	wchar_t *lpustr = ustr.toUnicode();
	__u_to_unicode(lpustr, ustr.sizeofUnicode() / RLIB_SIZEOF(wchar_t));
	return lpustr;
#else
	String dst(str);
	__u_to_unicode(dst, dst.GetLength());
	return dst;
#endif //_UNICODE
}

//-------------------------------------------------------------------------

bool HttpUtility::IsUrlSafeChar(TCHAR ch)
{
	if ((((ch >= _T('a')) && (ch <= _T('z'))) || ((ch >= _T('A')) && (ch <= _T('Z')))) ||
		((ch >= _T('0')) && (ch <= _T('9')))) {
		return true;
	}
	switch (ch) 
	{
	case _T('('):
	case _T(')'):
	case _T('*'):
	case _T('-'):
	case _T('.'):
	case _T('_'):
	case _T('!'):
		return true;
	}
	return false;
}