/********************************************************************
	Created:	2016/06/30  22:39
	Filename: 	RLib_HttpCookie.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_HttpCookie.h"
#include "RLib_StringHelper.h"
#include "RLib_StringConvHelper.h"
#include "RLib_Utility.h"
using namespace System::Net;

//-------------------------------------------------------------------------

static String NormalizeCookieValue(_In_ const String &cv)
{
	auto delimiter = cv.IndexOf(_T(";"));
	if (delimiter != -1) {
		return cv.Substring(0, delimiter);
	} //if
	return cv;
}

//-------------------------------------------------------------------------

static String NormalizeCookieName(_In_ const String &cn)
{
	auto delimiter = cn.IndexOf(_T("="));
	if (delimiter != -1) {
		return cn.Substring(0, delimiter);
	} //if
	return cn;
}

//-------------------------------------------------------------------------

String HttpCookie::GetValue(_In_ const String &cookie, _In_ const String name)
{
	intptr_t offset, end_offset = 0;
	while ((offset = cookie.IndexOf(name, end_offset)) != -1) {
		// back check
		if (offset != 0) {
			TCHAR prec = cookie[offset - 1];
			if (prec != _T(' ') && prec != _T(';')) {
				end_offset = offset + name.Length;
				continue;
			} //if
		} //if

		// forward check
		TCHAR forc = cookie[offset + name.Length];
		if (offset + name.Length != cookie.Length && forc != _T('=') && forc != _T(' ')) {
			end_offset = offset + name.Length;
			continue;
		} //if

		end_offset = cookie.IndexOf(_T(";"), offset);
		if (end_offset == -1) {
			return cookie.Substring(offset + name.Length + 1);
		} //if

		offset = offset + name.Length + 1;
		assert(end_offset >= offset);
		return cookie.Substring(offset, end_offset - offset);
	}
	return Nothing;
}

//-------------------------------------------------------------------------

void HttpCookie::Remove(_Inout_ String &cookie, _In_ const String name)
{
	intptr_t offset, end_offset = 0;
	while ((offset = cookie.IndexOf(name, end_offset)) != -1)
	{
		// back check
		if (offset != 0) {
			TCHAR prec = cookie[offset - 1];
			if (prec != _T(' ') && prec != _T(';')) {
				end_offset = offset + name.Length;
				continue;
			} //if
		} //if

		// forward check
		TCHAR forc = cookie[offset + name.Length];
		if (offset + name.Length != cookie.Length && forc != _T('=') && forc != _T(' ')) {
			end_offset = offset + name.Length;
			continue;
		} //if

		end_offset = cookie.IndexOf(_T(";"), offset);
		if (end_offset == -1) {
			if (offset == 0) {
				cookie = Nothing;
				break;
			} //if
			cookie = cookie.Substring(0, offset).trimEnd().trimEnd(_T(';'));
		} else {
			if (offset != 0) {
				cookie = cookie.Substring(0, offset).trimEnd().trimEnd(_T(';')) + cookie.Substring(end_offset);
			} else {
				cookie = cookie.Substring(end_offset + 1).trimStart();
			} //if	
		} //if
		end_offset = 0;
	}
}

//-------------------------------------------------------------------------

static void EnumAllCookieValue(_In_ const String &headers, _Inout_ String &cookie)
{
	intptr_t index, limiter = 0;
	while ((index = headers.IndexOfNoCase(_T("Set-Cookie: "), limiter)) != -1) {
		if (index > 0 && headers[index - 1] != _T('\n')) {
			limiter = index + RLIB_COUNTOF_STR(_T("Set-Cookie: "));
			continue;
		} //if
		limiter = headers.IndexOf(_T(";"), index += RLIB_COUNTOF_STR(_T("Set-Cookie: ")));
		limiter = limiter == -1 ? headers.IndexOf(_T("\r\n"), index) : limiter;
		limiter = limiter == -1 ? headers.Length : limiter; // last line

		String &&ck = headers.Substring(index, limiter - index);
		HttpCookie::Remove(cookie, NormalizeCookieName(ck));
		cookie.append(RLIB_STR_LEN(_T("; ")));
		cookie.append(ck);

		if (limiter == headers.Length) break;
	}

	if (limiter > 0) cookie.trimStart(_T(';')).trimStart();
}

//-------------------------------------------------------------------------

void HttpCookie::EnumAll(_In_ const char *headers, _Inout_ String &cookie)
{
	LOCAL_NEWW(ckt, RLIB_DEFAULT_BUFFER_SIZE);

	const char *p = headers, *limiter;
	while ((p = Utility::stristr(p, "Set-Cookie: ")) != nullptr) {
		if (p != headers && p[-1] != '\n') { // first line
			p += RLIB_COUNTOF_STR("Set-Cookie: ");
			continue;
		} //if

		limiter = strstr(p += RLIB_COUNTOF_STR("Set-Cookie: "), ";");
		if (limiter == nullptr) {
			limiter = strstr(p, "\r\n");
			if (limiter == nullptr) { // last line
				limiter = p + strlen(p);
			} //if
		} //if

#ifdef _UNICODE
		auto cksize = RT2::MultiByteToUnicode(ckt, sizeof(ckt) - sizeof(TCHAR), p, (limiter - p) * sizeof(char));	
#else
		auto cksize = RLIB_MIN(RLIB_COUNTOF(ckt) - 1, limiter - p) * sizeof(char);
		memcpy(ckt, p, cksize);
#endif // _UNICODE
		ckt[cksize / sizeof(TCHAR)] = _T('\0');

		auto delimiter = _tcsstr(ckt, _T("="));
		if (delimiter) delimiter[0] = '\0';
		HttpCookie::Remove(cookie, StringReference(ckt));
		if (delimiter) delimiter[0] = '=';
		if (!cookie.IsNullOrEmpty()) cookie.append(RLIB_STR_LEN(_T("; ")));
		cookie.append(ckt);
	}
}