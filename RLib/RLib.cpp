/********************************************************************
	Created:	2012/04/21  14:48
	Filename: 	RLib.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#if VSVER > 120
# pragma warning(disable: 5032)
// detected #pragma warning(push) with no corresponding #pragma warning(pop)
#endif // VSVER > 120

#include "RLib_GlobalInit.h"

#ifdef _DEBUG

//-------------------------------------------------------------------------

intptr_t __cdecl __crtMessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	intptr_t (APIENTRY *pfnMessageBox)(HWND, LPCTSTR, LPCTSTR, UINT) = NULL;
	HWND     (APIENTRY *pfnGetActiveWindow)(void) = NULL;
	HWND     (APIENTRY *pfnGetLastActivePopup)(HWND) = NULL;

	HWND hWndParent = NULL;

	if (NULL == pfnMessageBox)
	{
		HMODULE hlib = GetModuleHandle(_T("user32.dll"));
		if (hlib == NULL) hlib = LoadLibrary(_T("user32.dll"));

		if (NULL == hlib || NULL == (pfnMessageBox =
			(intptr_t (APIENTRY *)(HWND, LPCTSTR, LPCTSTR, UINT))(LPVOID)
#ifndef _UNICODE
			GetProcAddress(hlib, "MessageBoxA")
#else
			GetProcAddress(hlib, "MessageBoxW")
#endif // _UNICODE
			)) {
			return 0;
		} //if
		
		pfnGetActiveWindow = (HWND (APIENTRY *)(void))
			(LPVOID)GetProcAddress(hlib, "GetActiveWindow");

		pfnGetLastActivePopup = (HWND (APIENTRY *)(HWND))
			(LPVOID)GetProcAddress(hlib, "GetLastActivePopup");
	} //if

	if (pfnGetActiveWindow)
		hWndParent = (*pfnGetActiveWindow)();

	if (hWndParent != NULL && pfnGetLastActivePopup)
		hWndParent = (*pfnGetLastActivePopup)(hWndParent);

	return (*pfnMessageBox)(hWndParent, lpText, lpCaption, uType);
}

//-------------------------------------------------------------------------

void RLIB_API __rlib_alert(LPCTSTR exp, LPCTSTR file, LPCTSTR func, LPCTSTR line)
{
	TCHAR strexp[RLIB_DEFAULT_MAX_BUFFER_SIZE * 4];
	strexp[0] = _T('\0');
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), _T("Assertion failed!") RLIB_NEWLINE);
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE _T("File: "));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), file);
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE _T("Function: "));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), func);
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE _T("Line: "));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), line);
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE RLIB_NEWLINE _T("Expression: "));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), exp != nullptr ? exp : _T("NULL"));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE RLIB_NEWLINE _T("For information on how your program can cause an assertion\nfailure, see the Visual C++ documentation on asserts"));
	_tcscat_s(strexp, RLIB_COUNTOF(strexp), RLIB_NEWLINE RLIB_NEWLINE _T("(Press Retry to debug - JIT must be enabled, or generate dump)"));

	auto nCode = __crtMessageBox(strexp,
								 _T("RLib C++ Runtime Library"),
								 MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
	if (nCode == IDABORT) {
		AppBase::Exit(ERROR_ASSERTION_FAILURE);
	} //if

	// Retry: call the debugger 
	if (nCode == IDRETRY) {
		__debugbreak();
		// return to user code
		return;
	} //if

	// Ignore: continue execution
	if (nCode == IDIGNORE) return;
}

#else

# ifndef RLIB_LIB
extern "C" void __cdecl __security_init_cookie(void) {}
#  ifdef _M_IX86
extern "C" void __fastcall __security_check_cookie(uintptr_t) {}
#  else
extern "C" void __cdecl __security_check_cookie(uintptr_t) {}
#  endif
# endif // !RLIB_LIB

#endif // _DEBUG

//-------------------------------------------------------------------------

HMODULE __current_native_handle;

//-------------------------------------------------------------------------

#ifdef RLIB_LIB
int __stdcall RLib_Init(_In_ void *_DllHandle, _In_ unsigned long _Reason)
#else
int __stdcall DllMain(_In_ HMODULE _DllHandle, _In_ unsigned long _Reason, _In_opt_ void *)
#endif // RLIB_LIB
{
	if (_Reason == DLL_PROCESS_ATTACH) {
		__current_native_handle = static_cast<HMODULE>(_DllHandle);
	} else if (_Reason == DLL_PROCESS_DETACH) {
#ifndef _ENABLE_RTL_CONV
		RT2::freelocale();
#endif // _ENABLE_RTL_CONV	
	} //if

	return TRUE;
}