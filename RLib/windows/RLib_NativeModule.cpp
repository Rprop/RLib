/********************************************************************
	Created:	2016/12/08  11:49
	Filename: 	RLib_NativeModule.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"
#include "RLib_NativeModule.h"
using namespace System::Runtime;

//-------------------------------------------------------------------------

HMODULE Marshal::LoadNativeModule(_In_opt_ LPCWSTR lpModuleFileName)
{
	UNICODE_STRING ustring;
	RtlInitUnicodeString(&ustring, lpModuleFileName);

	PVOID handle = nullptr;
	NTSTATUS status;
	status = LdrLoadDll(NULL, NULL, &ustring, &handle);
	assert(status == STATUS_SUCCESS);
	return static_cast<HMODULE>(handle);
}

//-------------------------------------------------------------------------

void Marshal::UnLoadNativeModule(_In_ HMODULE hModule)
{
	NTSTATUS status;
	status = LdrUnloadDll(hModule);
	assert(status == STATUS_SUCCESS);
}

//-------------------------------------------------------------------------

HMODULE Marshal::GetNativeModule(_In_opt_ LPCWSTR lpModuleName /* = nullptr */,
								 _In_opt_ LPWSTR lpModulePath /* = nullptr */)
{
	if (lpModuleName == nullptr) {
		extern HMODULE __current_native_handle; // defined in RLib.cpp
		return __current_native_handle;
	} //if

	UNICODE_STRING ustring;
	RtlInitUnicodeString(&ustring, lpModuleName);

	PVOID handle = nullptr;
	NTSTATUS status;
	status = LdrGetDllHandle(lpModulePath, NULL, &ustring, &handle);
	return static_cast<HMODULE>(handle);
}

//-------------------------------------------------------------------------

LPVOID Marshal::GetNativeMethod(_In_ HMODULE hModule,
								_In_opt_ LPCSTR lpProcedureName, _In_opt_ ULONG nProcedureNumber /* = 0 */)
{
	ANSI_STRING astring;
	RtlInitAnsiString(&astring, const_cast<LPSTR>(lpProcedureName)); // const cast

	LPVOID address;
	NTSTATUS status;
	status = LdrGetProcedureAddress(hModule, &astring, nProcedureNumber, &address);
	return status == STATUS_SUCCESS ? address : nullptr;
}
