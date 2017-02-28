/********************************************************************
	Created:	2014/07/31  18:28
	Filename: 	RLib_UAC.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_UAC.h"

#ifdef _USE_UAC
#include <shlwapi.h>
#include "RLib_Exception.h"
#include "RLib_NativeModule.h"

using namespace System::Microsoft;

//-------------------------------------------------------------------------

bool UAC::IsRunAsAdmin()
{
	auto advapi32 = Runtime::Marshal::LoadNativeModule(L"ADVAPI32.dll");
	if (advapi32 != NULL) {
		// http://msdn.microsoft.com/en-us/library/aa375213(VS.85).aspx 
		// allocates and initializes a SID of the administrators group
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID pAdministratorsGroup            = NULL;
		BOOL fIsRunAsAdmin                   = FALSE;
		if (PInvoke(advapi32, AllocateAndInitializeSid)(&NtAuthority,
														2,
														SECURITY_BUILTIN_DOMAIN_RID,
														DOMAIN_ALIAS_RID_ADMINS,
														NULL, NULL, NULL, NULL, NULL, NULL,
														&pAdministratorsGroup)) {
			// determines whether the SID of administrators group is enabled in 
			// the primary access token of the process
			if (!PInvoke(advapi32, CheckTokenMembership)(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
				trace(!"CheckTokenMembership failed");
			} //if

			// centralized cleanup for all allocated resources
			PInvoke(advapi32, FreeSid)(pAdministratorsGroup);
		} //if

		Runtime::Marshal::UnLoadNativeModule(advapi32);

		return fIsRunAsAdmin != FALSE;
	} //if

	return false;
}

//-------------------------------------------------------------------------

DWORD UAC::RunAsAdministrator(LPCTSTR path /* = NULL */, LPCTSTR params /* = NULL */)
{
	DWORD dwRet  = ERROR_SUCCESS;
	auto shell32 = Runtime::Marshal::LoadNativeModule(L"SHELL32.dll");
	if (shell32 != NULL) {
//		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb           = _T("runas");
		sei.lpFile           = path != NULL ? path : AppBase::GetAbsolutePath();
		sei.nShow            = SW_NORMAL;
		sei.fMask            = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOZONECHECKS;
		sei.lpParameters     = params;
		if (!PInvoke(shell32, ShellExecuteEx)(&sei)) {
			dwRet = Exception::GetLastErrorId();
		} //if
//		CoUninitialize();
		Runtime::Marshal::UnLoadNativeModule(shell32);
	} //if

	return dwRet;
}

#endif // _USE_UAC