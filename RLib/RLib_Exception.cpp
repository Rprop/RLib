/********************************************************************
	Created:	2014/06/30  18:26
	Filename: 	RLib_Exception.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Exception.h"
#include "RLib_StringHelper.h"
#include <windows.h>

#if !RLIB_DISABLE_NATIVE_API
#include "native/RLib_Native.h"
#endif // RLIB_DISABLE_NATIVE_API

//-------------------------------------------------------------------------

Exception::Exception()
{
	this->HResult    = STATUS_SUCCESS;
	this->Message[0] = _T('\0');
	//memset(this->Message, 0, sizeof(this->Message));
#ifdef _DEBUG
	this->Source     = nullptr;
	this->TargetSite = nullptr;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

Exception::Exception(const TCHAR *lpmsg, INT id /* = -1 */)
{
	intptr_t l       = TSIZE(RLIB_MIN(_tcslen(lpmsg), RLIB_COUNTOF_STR(this->Message) - 1));
	this->HResult    = id;
	memcpy(this->Message, lpmsg, static_cast<size_t>(l));
	this->Message[l] = _T('\0');
#ifdef _DEBUG
	this->Source     = nullptr;
	this->TargetSite = nullptr;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

#ifdef _DEBUG
void Exception::Set(INT _HResult, const TCHAR *_Message, const TCHAR *_Source,
					const TCHAR *_TargetSite)
#else 
void Exception::Set(INT _HResult, const TCHAR *_Message)
#endif // _DEBUG
{
	auto l           = TSIZE(RLIB_MIN(_tcslen(_Message), RLIB_COUNTOF(this->Message) - 1));
	this->HResult    = _HResult;
	memcpy(this->Message, _Message, static_cast<size_t>(l));
	this->Message[l] = _T('\0');
#ifdef _DEBUG
	this->Source     = _Source;
	this->TargetSite = _TargetSite;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

#ifdef _DEBUG
void Exception::SetDebugInfo(const TCHAR *_Source, const TCHAR *_TargetSite)
{
	this->Source     = _Source;
	this->TargetSite = _TargetSite;
}
#endif // _DEBUG

//-------------------------------------------------------------------------

void Exception::Ref(const Exception &Src)
{
	*this = Src;
}

//-------------------------------------------------------------------------

DWORD Exception::GetLastErrorId()
{
#if RLIB_DISABLE_NATIVE_API
	return ::GetLastError();
#else
	return RtlGetLastWin32Error();
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

bool Exception::FormatErrorMessage(OUT LPTSTR lptstr, DWORD nchar, IN DWORD err_code)
{
	// If the function succeeds, the return value is the number of TCHARs stored in the output buffer,
	// excluding the terminating null character.
	// If the function fails, the return value is zero
	auto dwLen = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL,
								 err_code,
								 NULL,
								 lptstr,
								 nchar, // in TCHARs
								 NULL);
	if (dwLen > 0) {
		--dwLen;
		// skips \r\n
		while (lptstr[dwLen] == _T('\r') || lptstr[dwLen] == _T('\n') || 
			   lptstr[dwLen] == _T('\t') || lptstr[dwLen] == _T(' ')) {
			if (dwLen == 0) break;
			--dwLen;
		}
		lptstr[dwLen + 1] = _T('\0');
		return true;
	} //if

	return false;
}

//-------------------------------------------------------------------------

bool Exception::FormatException(OUT Exception *lpex, IN DWORD err_code)
{
	lpex->ErrorNo = err_code;
	return Exception::FormatErrorMessage(lpex->Message, 
										 RLIB_COUNTOF(lpex->Message),
										 err_code);
}

//-------------------------------------------------------------------------

Exception *Exception::GetLastException()
{
	auto lpex = new Exception();
	if (lpex) {
		Exception::FormatException(lpex, Exception::GetLastErrorId());
	} //if
	return lpex;
}

//-------------------------------------------------------------------------

Exception *Exception::GetLastException(DWORD err_code)
{
	auto lpex = new Exception();
	if (lpex) {
		lpex->ErrorNo = err_code;
		Exception::FormatException(lpex, err_code);
	} //if
	return lpex;
}

//-------------------------------------------------------------------------

void Exception::SetLastErrorId(DWORD err_code)
{
#if RLIB_DISABLE_NATIVE_API
	::SetLastError(err_code);
#else
	RtlSetLastWin32Error(err_code);
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

void Exception::SetLastException(NTSTATUS status, bool bAlertInDebug /* = true */)
{
	UNREFERENCED_PARAMETER(bAlertInDebug);
	Exception::SetLastErrorId(RtlNtStatusToDosErrorNoTeb(status));

#ifdef _DEBUG
	if (bAlertInDebug) {
		auto ex = Exception::GetLastException();
		if (ex != nullptr) {
			alert(ex->Message);
			Exception::SetLastErrorId(ex->ErrorNo);
			delete ex;
		} //if
	} //if
#endif // _DEBUG
}