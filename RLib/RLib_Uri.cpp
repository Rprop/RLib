/********************************************************************
	Created:	2016/07/28  9:33
	Filename: 	RLib_Uri.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Uri.h"
#include "RLib_StringHelper.h"
#include "RLib_Fundamental.h"
using namespace System::Net;

//-------------------------------------------------------------------------

Uri::Uri(const String &url)
{
//	this->OriginalString = url;

	// parse scheme part
	intptr_t offsetScheme = url.IndexOf(_T("://"));
	if (offsetScheme == -1/* || offsetScheme > 5*/) {
		trace(!"invalid url");
		return;
	} //if
	this->Scheme = url.Substring(0, offsetScheme - 0).toUpper();

	// the rest part of url
	intptr_t beginOffset = offsetScheme + RLIB_COUNTOF_STR(_T("://"));
	const String &url_body = url;

	// parse host and port
	intptr_t portOffset  = url_body.IndexOf(_T(":"), beginOffset);
	intptr_t slashOffset = url_body.IndexOf(_T("/"), beginOffset);
	if (portOffset == -1 || (slashOffset != -1 && portOffset > slashOffset)) {
		// not found port or illegal, use default port
		this->Port = (offsetScheme != 5 || this->Scheme != _T("HTTPS")) ? 80u : 443u;
		// check path body
		if (slashOffset == -1) {
			// http:// rlib.cf
			this->Host         = url_body.Substring(beginOffset).toLower();
			this->PathAndQuery = _R("/");
		} else {
			// http:// rlib.cf /file?query=...
			this->Host         = url_body.Substring(beginOffset, slashOffset - beginOffset).toLower();
			this->PathAndQuery = url_body.Substring(slashOffset);
		} //if
	} else {
		this->Host = url_body.Substring(beginOffset, portOffset - beginOffset).toLower();
		// no slash
		// skip ':', so
		portOffset += 1;
		if (slashOffset == -1) {
			// http:// rlib.cf:port		
			this->Port = static_cast<USHORT>(Int32::TryParse(url_body.GetConstData() + portOffset));
			this->PathAndQuery = _R("/");
		} else {
			// http:// hostname:port /file?query=...
			String &&strPort   = url_body.Substring(portOffset, slashOffset - portOffset);
			this->Port         = static_cast<USHORT>(Int32::TryParse(strPort));
			this->PathAndQuery = url_body.Substring(slashOffset);
		} //if
	} //if
}

//-------------------------------------------------------------------------

String Uri::ProcessUri(const String &path, const Uri *lpfather)
{
	intptr_t offset = path.IndexOf(_T("://"));
	if (offset != -1 && offset < 8) {
		return path;
	} //if

	String url(128);
	url.append(lpfather->Scheme);
	url.append(RLIB_STR_LEN(_T("://")));
	url.append(lpfather->Host);
	url.append(RLIB_STR_LEN(_T(":")));
	url.append(Int32(lpfather->Port).ToString());

	if (!path.StartsWith(_T('/'))) {
		intptr_t slashOffset = lpfather->PathAndQuery.LastIndexOf(_T('/'));
		url.append(lpfather->PathAndQuery.GetConstData(), slashOffset + 1 - 0);
	} else {
		url.append(path);
	} //if

	return url;
}

//-------------------------------------------------------------------------

String Uri::GetTopDomain()
{
	const TCHAR *s = this->Host;
	const TCHAR *p = _tcsrchr(s, _T('.'));
	if (p != nullptr) {
		while (p > s && *(--p) != _T('.')) {}
		return p == s ? p : ++p;
	} //if

	return this->Host;
}

//-------------------------------------------------------------------------

static String __canonicalize_absolute_path(String path)
{
	auto hmark = path.IndexOf(_T('#')); // hash mark
	if (hmark != -1) path.substring(0, hmark);
	 
	auto smark = path.IndexOf(_T(';')); // semicolon
	if (smark != -1) path.substring(0, smark);

	return path.replace(_T("\\"), _T("/"));
}

//-------------------------------------------------------------------------

String Uri::GetFile()
{
	String &&path  = this->GetAbsolutePath();
	intptr_t fmark = path.LastIndexOf(_T('/'));
	if (fmark != -1) path.substring(fmark + 1);
	return path;
}

//-------------------------------------------------------------------------

String Uri::GetDirectory()
{
	String &&path  = this->GetAbsolutePath();
	intptr_t fmark = path.LastIndexOf(_T('/'));
	if (fmark >= 0) path.substring(0, fmark + 1);
	return path;
}

//-------------------------------------------------------------------------

String Uri::GetAbsolutePath()
{
	auto qmark = this->PathAndQuery.IndexOf(_T('?'));
	if (qmark != -1) {
		return __canonicalize_absolute_path(this->PathAndQuery.Substring(0, qmark));
	} //if
	return __canonicalize_absolute_path(this->PathAndQuery);
}

//-------------------------------------------------------------------------

String Uri::GetQueryString()
{
	auto qmark = this->PathAndQuery.IndexOf(_T('?')) + 1;
	if (qmark != 0) {
		return this->PathAndQuery.Substring(qmark);
	} //if
	return Nothing;
}

//-------------------------------------------------------------------------

String Uri::GetQueryString(const String &name)
{
	auto qmark = this->PathAndQuery.IndexOf(_T('?')) + 1;
	if (qmark != 0) {
		auto index = qmark;
		while ((index = this->PathAndQuery.IndexOf(name, index)) != -1) {
			// back check
			if (index != qmark && this->PathAndQuery[index - 1] != _T('&')) {
				index += name.Length;
				continue;
			} //if

			index += name.Length;
			if (this->PathAndQuery[index] != _T('=')) {
				continue;
			} //if

			auto limiter = this->PathAndQuery.IndexOf(_T('&'), index += 1) - index;
			return this->PathAndQuery.Substring(index, max(-1, limiter));
		}
	} //if
	return Nothing;
}

//-------------------------------------------------------------------------

bool Uri::IsSubDomain(const String &child, const String &parent)
{
	const TCHAR *p = child;
	const TCHAR *a = parent;
	while ((p = _tcschr(p, _T('.'))) != NULL) {
		if (_tcsicmp(a, ++p) == 0) return true;
	}

	return false;
}