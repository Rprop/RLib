/********************************************************************
	Created:	2014/07/01  14:31
	Filename: 	RLib_Path.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_StringHelper.h"
#include "RLib_Path.h"
using namespace System::IO;

//-------------------------------------------------------------------------

static void __collapse_path( TCHAR *path, intptr_t mark )
{
	/*
	 *	Get rid of . and .. components in the path
	 */
	TCHAR *p, *next;

	// collapse duplicate backslashes
	next = path + mark;
	for (p = next; *p; p++) if (*p != _T('\\') || next[-1] != _T('\\')) *next++ = *p;
	*next = 0;

	p = path + mark;
	while (*p)
	{
		if (*p == _T('.'))
		{
			switch(p[1])
			{
			case _T('\\'): /* .\ component */
				next = p + 2;
				memmove( p, next, (TLEN(next) + 1) * sizeof(TCHAR) );
				continue;
			case 0:  /* final . */
				if (p > path + mark) --p;
				*p = 0;
				continue;
			case _T('.'):
				if (p[2] == _T('\\'))  /* ..\ component */
				{
					next = p + 3;
					if (p > path + mark) {
						--p;
						while (p > path + mark && p[-1] != _T('\\')) --p;
					} //if
					memmove( p, next, (TLEN(next) + 1) * sizeof(TCHAR) );
					continue;
				}
				else if (!p[2])  /* final .. */
				{
					if (p > path + mark) {
						--p;
						while (p > path + mark && p[-1] != _T('\\')) --p;
						if (p > path + mark) p--;
					} //if
					*p = 0;
					continue;
				} //if
				break;
			}
		}
		// skip to the next component
		while (*p && *p != _T('\\')) ++p;
		if (*p == _T('\\')) {
			// remove last dot in previous dir name
			if (p > path + mark && p[-1] == '.')
				memmove(p - 1, p, (TLEN(p) + 1) * sizeof(TCHAR));
			else
				++p;
		} //if
	}
	// remove trailing spaces and dots (yes, Windows really does that, don't ask)
	while (p > path + mark && (p[-1] == _T(' ') || p[-1] == _T('.'))) --p;
	*p = 0;
}

//-------------------------------------------------------------------------

String Path::ToBackslash(const String &path)
{
	return path.Replace(_T("/"), _T("\\"));
}

//-------------------------------------------------------------------------

String Path::ToForwardslash(const String &path)
{
	return path.Replace(_T("\\"), _T("/"));
}

//-------------------------------------------------------------------------

String Path::ToNtPath(const String &path)
{
	/*
	 *	RtlDosPathNameToNtPathName
	 */

	// ignore nt path
	if (path.Length > RLIB_COUNTOF_STR(_T("\\??\\")) &&
		StringStartWith_4(path.GetConstData(), _T("\\??\\"))) {
		return path;
	} //if

	// canonicalize path
	String &&dos_path = Path::ToBackslash(path);
	String   nt_path(path.Length + 32);
	intptr_t mark = 0;
	auto     lpdos_path = dos_path.GetConstData();

    // NT prefix
	nt_path = _R("\\??\\");

	// check if absolute path
	if (dos_path.Length >= RLIB_COUNTOF_STR(_T("?:"))) {
		// MS-DOS to NT path
		if (lpdos_path[1] == _T(':')) {
			assert(lpdos_path[2] == _T('\\') || lpdos_path[2] == _T('\0'));
			// drive name length = 1
			nt_path.append(dos_path);
			if (lpdos_path[2] == _T('\0')) {
				nt_path.append(RLIB_STR_LEN(_T("\\")));
				return nt_path;
			} //if
			mark = RLIB_COUNTOF_STR(_T("?:\\"));
			goto __last_process;
		} //if

		  // UNC path, eg. \\server\share
		if (StringStartWith_2(lpdos_path, _T("\\\\"))) {
			nt_path += _R("UNC");
			// remove one backslash
			nt_path.append(lpdos_path + RLIB_COUNTOF_STR(_T("\\")),
						   dos_path.Length - RLIB_COUNTOF_STR(_T("\\")));
			goto __last_process;
		} //if
	} //if
    
	// relative path
	auto lpStartupPath       = AppBase::GetStartupPath();
	auto lpStartupPathLength = AppBase::LengthOfStartupPath();
	if (lpStartupPathLength > 2 &&
		StringStartWith_2(lpStartupPath, _T("\\\\"))) {
		nt_path += _R("UNC");
		// remove one backslash
		nt_path += &lpStartupPath[RLIB_COUNTOF_STR(_T("\\"))];
	} else {
		nt_path.append(lpStartupPath, lpStartupPathLength);
	} //if

	if (dos_path.StartsWith(_T('\\'))) {
		nt_path.append(lpdos_path + RLIB_COUNTOF_STR(_T("\\")),
					   dos_path.Length - RLIB_COUNTOF_STR(_T("\\")));
	} else {
		nt_path += dos_path;
	} //if

__last_process: 
	if (mark == 0) {
		mark = nt_path.IndexOf(_T(":\\"), 4) + 2;
		if (mark == (-1 + 2)) {
			mark = nt_path.IndexOf(_T("UNC\\"), 4) + 4;
			mark = nt_path.IndexOf(_T("\\"), mark) + 1;
			assert(mark != (-1 + 1));
		} //if
	} //if

	__collapse_path(nt_path, mark);

	return nt_path;
}

//-------------------------------------------------------------------------

String Path::ToDosPath(const String &path, bool nt_processed /* = false */)
{
	String dos_path;
	if (!nt_processed) {
		dos_path = Path::ToNtPath(path).substring(RLIB_COUNTOF_STR(_T("\\??\\")));
	} else {
		dos_path = path.Substring(RLIB_COUNTOF_STR(_T("\\??\\")));
	} //if

	// \?\UNC\server\share\ path
	if (dos_path.Length >= RLIB_COUNTOF_STR(_T("UNC\\")) &&
		StringStartWith_4(dos_path.GetConstData(), _T("UNC\\"))) {
		// remove UNC mark
		dos_path.substring(RLIB_COUNTOF_STR(_T("UN")));
		dos_path[0] = _T('\\');
	} //if
	return dos_path;
}

//-------------------------------------------------------------------------

String Path::AddBackslash(const String &path)
{
	if (!path.EndsWith(_T('\\')) && !path.EndsWith(_T('/'))) {
		return path + _R("\\");
	} //if

	return path;
}

//-------------------------------------------------------------------------

String Path::RemoveBackslash(const String &path)
{
	if ((!path.EndsWith(_T('\\')) && !path.EndsWith(_T('/'))) || 
		Path::IsRoot(path)) {
		return path;
	} //if

	return path.Substring(0, path.Length - 1);
}

//-------------------------------------------------------------------------

String Path::QuoteSpaces(const String &path)
{
	if (path.Contains(_T(" "))) {
		return _R("\"") + path + _R("\"");
	} //if

	return path;
}

//-------------------------------------------------------------------------

bool Path::IsRoot(const String &path)
{
	if (!Path::IsRelative(path)) {
		auto lpszPath = path.GetConstData();
		if (*lpszPath == _T('\\')) {
			if (!lpszPath[1]) {
				return true; /* \ */
			} else if (lpszPath[1] == _T('\\')) {
				bool bSeenSlash = false;
				lpszPath += 2;

				// check for UNC root path
				while (*lpszPath) {
					if (*lpszPath == _T('\\')) {
						if (bSeenSlash) return false;
						bSeenSlash = true;
					} //if
					++lpszPath;
				}
				return true;
			} //if
		} else if (lpszPath[1] == _T(':') && lpszPath[2] == _T('\\') && lpszPath[3] == _T('\0')) {
			return true; /* X:\ */
		} //if
	} //if

	return false;
}

//-------------------------------------------------------------------------

bool Path::IsRelative(const String &path)
{
	if (path.Length >= RLIB_COUNTOF_STR(_T("?:"))) {
		auto lppath = path.GetConstData();

		// MS-DOS to NT path
		if (lppath[1] == _T(':')) {
			assert(lppath[2] == _T('\\') || lppath[2] == _T('\0'));
			return false;
		} //if

		// UNC path, eg. \\server\share
		if (StringStartWith_2(lppath, _T("\\\\"))) {
			return false;
		} //if
	} //if

	return true;
}

//-------------------------------------------------------------------------

Path::Path(const String &path)
{
	this->m_FullPath = Path::ToNtPath(path);

	if (_tsplitpath_s(this->DosPath,
					  this->m_PathInfo.Drive, RLIB_COUNTOF(this->m_PathInfo.Drive),
					  this->m_PathInfo.Dir, RLIB_COUNTOF(this->m_PathInfo.Dir),
					  this->m_PathInfo.Fname, RLIB_COUNTOF(this->m_PathInfo.Fname),
					  this->m_PathInfo.Ext, RLIB_COUNTOF(this->m_PathInfo.Ext)) != 0) {
		assert(!"split path failed!");
	} //if
}

//-------------------------------------------------------------------------

String Path::GetDosPath()
{
	return Path::ToDosPath(this->m_FullPath, true);
}

//-------------------------------------------------------------------------

String Path::GetDirectory()
{
	return StringReference(this->m_PathInfo.Drive) + StringReference(this->m_PathInfo.Dir);
}

//-------------------------------------------------------------------------

String Path::GetFileName()
{
	return StringReference(this->m_PathInfo.Fname) + StringReference(this->m_PathInfo.Ext);
}

//-------------------------------------------------------------------------

bool Path::IsRoot()
{
	return Path::IsRoot(this->DosPath);
}

//-------------------------------------------------------------------------

static String __relative_path_to(String &from, String &to)
{
	auto lpfrom = from.GetConstData();
	auto lpto   = to.GetConstData();

	intptr_t first = 0;
	while (lpfrom[first] == lpto[first]) {
		++first;
		if (lpfrom[first] == _T('\0')) {
			assert(from.Length <= to.Length);
			// C:\1\ + C:\1\2\3\ => 2\3\   
			return to.substring(from.Length);
		} //if
	}

	if (first < 3) {
		// C:\ + D:\ => D:\ 
		return to;
	} //if

	assert(lpfrom[first] != _T('\0'));
	if (lpto[first] == _T('\0')) {
		// C:\1\2\3\ + C:\1\ => ..\..\ 
		intptr_t depth = from.substring(to.Length).CountOf(_T("\\"));
		to.Empty();
		while (--depth >= 0) {
			to += _R("..\\");
		}
	} else {
		// D:\1\2\3\
		// D:\1\5\3\ 
		String &&comm = from.Substring(0, first - 0);
		comm.substring(0, comm.LastIndexOfRL(RLIB_STR_LEN(_T("\\"))) - 0);
		String &&temp = __relative_path_to(from, comm);
		while (lpto[--first] != _T('\\')) {
			if (first <= 0) break;
		}
		assert(first > 0);
		to = temp + to.substring(first + 1);
	} //if

	return to;
}

//-------------------------------------------------------------------------

String Path::RelativePathTo(const String &path_from, const String &path_to)
{
	String &&from = Path::ToDosPath(Path::AddBackslash(path_from));
	String &&to   = Path::ToDosPath(Path::AddBackslash(path_to));

	return __relative_path_to(from, to);
}