/********************************************************************
	Created:	2016/07/30  14:37
	Filename: 	RLib_WindowsIdentity.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_WindowsIdentity.h"
#include "RLib_Exception.h"
#include "RLib_Fundamental.h"
#include "RLib_StringHelper.h"
using namespace System::Security::Principal;

//-------------------------------------------------------------------------

static bool __query_token_info(HANDLE token, 
							   TOKEN_INFORMATION_CLASS tokenClass,
							   _Out_ PTOKEN_USER tokenInfo, ULONG tokenInfoLength)
{
	// https://msdn.microsoft.com/en-us/library/ff567055(v=vs.85).aspx
	ULONG returnLength;
	NTSTATUS status = NtQueryInformationToken(token,
											  tokenClass,
											  tokenInfo,
											  tokenInfoLength,
											  &returnLength);
	if (!NT_SUCCESS(status)) {
		assert(status != STATUS_BUFFER_TOO_SMALL);
		Exception::SetLastException(status, false);
		return false;
	} //if
	
	NtClose(token);

	return true;
}
//-------------------------------------------------------------------------

WindowsIdentity::WindowsIdentity(HANDLE token)
{
	if (!__query_token_info(token, TokenUser, &this->m_user, sizeof(this->m_data))) {
		memset(&this->m_data, 0, sizeof(this->m_data));
	} //if
}

//-------------------------------------------------------------------------

WindowsIdentity *WindowsIdentity::GetCurrent()
{
	// https://msdn.microsoft.com/en-us/library/ff567024(v=vs.85).aspx
	HANDLE   token;
	NTSTATUS status = NtOpenProcessTokenEx(NtCurrentProcess(),
										   TOKEN_READ, OBJ_KERNEL_HANDLE, &token);
	if (!NT_SUCCESS(status)) {
		Exception::SetLastException(status, false);
		return nullptr;
	} //if

	return new WindowsIdentity(token);
}

//-------------------------------------------------------------------------

WindowsIdentity *WindowsIdentity::GetCurrent(bool ifImpersonating)
{
	// https://msdn.microsoft.com/en-us/library/ff567032(v=vs.85).aspx
	HANDLE   token;
	NTSTATUS status = NtOpenThreadTokenEx(NtCurrentThread(),
										  TOKEN_READ, TRUE, OBJ_KERNEL_HANDLE, &token);
	if (!NT_SUCCESS(status)) {
		if (ifImpersonating) {
			Exception::SetLastException(status, false);
			return nullptr;
		} //if

		status = NtOpenProcessTokenEx(NtCurrentProcess(),
									  TOKEN_READ, OBJ_KERNEL_HANDLE, &token);
		if (!NT_SUCCESS(status)) {
			Exception::SetLastException(status, false);
			return nullptr;
		} //if
	} //if

	return new WindowsIdentity(token);
}

//-------------------------------------------------------------------------

PISID WindowsIdentity::GetUser()
{
	return static_cast<PISID>(this->m_user.User.Sid);
}

//-------------------------------------------------------------------------

String WindowsIdentity::GetCurrentUser()
{
	auto lpIdentity = WindowsIdentity::GetCurrent();
	if (!lpIdentity) return _R(".DEFAULT");

	PISID  lpSID = static_cast<PISID>(lpIdentity->User);
	String szSID;
	szSID.reserve(RLIB_MAX_LONG_LENGTH * (lpSID->SubAuthorityCount + 3), false);
	szSID.copyf(_T("S-%u-%u"), lpSID->Revision, lpSID->SubAuthorityCount);
	
	TCHAR ID[RLIB_MAX_LONG_LENGTH + 2] = _T("-");
	for (intptr_t i = 0; i < lpSID->SubAuthorityCount; ++i) {
		_ultot_s(lpSID->SubAuthority[i], ID + 1, RLIB_COUNTOF(ID) - 1, 10);
		szSID.append(ID);
	} //for

	delete lpIdentity;
	return szSID;
}