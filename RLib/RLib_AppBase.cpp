/********************************************************************
Created:	2011/08/15  10:05
Filename: 	RLib_AppBase.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_AppBase.h"
#include "RLib_Helper.h"
#include "windows/RLib_NativeModule.h"
#include "native/RLib_Native.h"
#include <windows.h>

#ifdef _UNICODE
# define DBGHELP_TRANSLATE_TCHAR
#else
# include <stdio.h> // sprintf_s
# include "RLib_StringConvHelper.h"
#endif // _UNICODE
#pragma warning(push)
#pragma warning(disable:4091) // 'typedef ': ignored on left of '' when no variable is declared
#include <Dbghelp.h>
//#pragma comment(lib, "DbgHelp")
#pragma warning(pop)

#define EXCEPTION_MS_VC                  static_cast<NTSTATUS>(0x406D1388UL)
#define EXCEPTION_UNCAUGHT_CXX_EXCEPTION static_cast<NTSTATUS>(0xE06D7363UL)

static RLIB_ALIGN(64) AppBase::SystemInformation __sysInfo = { 0 };
TCHAR    __startupPath[RLIB_MAX_PATH]   = { 0 };
TCHAR    __imageFileName[RLIB_MAX_PATH] = { 0 };
intptr_t __startupPathLength;
intptr_t __imageFileNameLength;

//-------------------------------------------------------------------------

AppBase::SystemInformation *AppBase::GetSystemInfo()
{
#if RLIB_DISABLE_NATIVE_API
	if (__sysInfo.dwPageSize == 0) {
#ifndef _WIN64
		::GetSystemInfo(&__sysInfo);
#else
		::GetNativeSystemInfo(&__sysInfo);
#endif // !_WIN64
	} //if
#else
	if (__sysInfo.PhysicalPageSize == 0) {
		NTSTATUS r = NtQuerySystemInformation(SystemBasicInformation, &__sysInfo, sizeof(__sysInfo), nullptr);
		assert(r == STATUS_SUCCESS);
		UNREFERENCED_PARAMETER(r);
	} //if
#endif // RLIB_DISABLE_NATIVE_API
    return &__sysInfo;
}

//-------------------------------------------------------------------------

void AppBase::Exit(DWORD code /* = 0 */)
{
#if WINVER <= _WIN32_WINNT_WIN7 || defined(RLIB_DISABLE_NATIVE_API)
	::ExitProcess(code);
#else
	RtlExitUserProcess(code);
#endif // WINVER <= _WIN32_WINNT_WIN7 || defined(RLIB_DISABLE_NATIVE_API)
}

//-------------------------------------------------------------------------

void AppBase::ExitThread(DWORD code /* = 0 */)
{
	::ExitThread(code);
}

//-------------------------------------------------------------------------

void AppBase::RtlExitThread(DWORD code /* = 0 */)
{
#if !RLIB_DISABLE_NATIVE_API
//	@warning generates exception on win7
	RtlFreeThreadActivationContextStack();
	RtlExitUserThread(code);
#else
	trace(!"not implemented");
	UNREFERENCED_PARAMETER(code);
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

DWORD AppBase::GetCurrentProcessId()
{ 
#if RLIB_DISABLE_NATIVE_API
	return ::GetCurrentProcessId();
#else
	return static_cast<DWORD>(reinterpret_cast<intptr_t>(NtCurrentTeb()->ClientId.UniqueProcess));
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

DWORD AppBase::GetCurrentThreadId()
{ 
#if RLIB_DISABLE_NATIVE_API
	return ::GetCurrentThreadId();
#else
	return static_cast<DWORD>(reinterpret_cast<intptr_t>(NtCurrentTeb()->ClientId.UniqueThread));
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

LPCWSTR AppBase::GetAbsolutePath()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->ImagePathName.Buffer;
}

//-------------------------------------------------------------------------

unsigned short AppBase::LengthOfAbsolutePath()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->ImagePathName.Length;
}

//-------------------------------------------------------------------------

LPCWSTR AppBase::GetCurrentPath()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->CurrentDirectory.DosPath.Buffer;
}

//-------------------------------------------------------------------------

unsigned short AppBase::LengthOfCurrentPath()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->CurrentDirectory.DosPath.Length;
}

//-------------------------------------------------------------------------

LPCWSTR AppBase::GetProcessCommandLine()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->CommandLine.Buffer;
}

//-------------------------------------------------------------------------

unsigned short AppBase::LengthOfProcessCommandLine()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ProcessParameters->CommandLine.Length;
}

//-------------------------------------------------------------------------

bool AppBase::IsDebuggerPresent()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->BeingDebugged != 0;
}

//-------------------------------------------------------------------------

LPCTSTR AppBase::GetStartupPath()
{
	return __startupPath;
}

//-------------------------------------------------------------------------

intptr_t AppBase::LengthOfStartupPath()
{
	return __startupPathLength;
}

//-------------------------------------------------------------------------

LPCTSTR AppBase::GetImageFileName()
{
	return __imageFileName;
}

//-------------------------------------------------------------------------

intptr_t AppBase::LengthOfImageFileName()
{
	return __imageFileNameLength;
}

//-------------------------------------------------------------------------

LPVOID AppBase::GetImageBaseAddress()
{
	return static_cast<PEB *>(AppBase::GetPEBAddress())->ImageBaseAddress;
}

//-------------------------------------------------------------------------

void *AppBase::GetPEBAddress()
{
#ifdef _M_X64
	return NtCurrentPeb();
// 	PROCESS_BASIC_INFORMATION processBasicInfo;
// 	ULONG returnLength;
// 	NTSTATUS statusQuery;
// 	statusQuery = NtQueryInformationProcess(AppBase::GetCurrentProcess(),
// 											ProcessBasicInformation,
// 											&processBasicInfo, sizeof(processBasicInfo),
// 											&returnLength);
// 	assert(statusQuery == STATUS_SUCCESS);
// 	return processBasicInfo.PebBaseAddress;
#else 
	PPEB lppeb;
	__asm {
		mov eax, fs: [30h] 
		mov lppeb, eax
	}
	return lppeb;
#endif 
}

//-------------------------------------------------------------------------

bool AppBase::GenerateCrashDump(LPEXCEPTION_POINTERS exceptionInfo)
{
	TCHAR szAddr[16] = { 0 };
	_i64tot_s(reinterpret_cast<intptr_t>(exceptionInfo->ExceptionRecord->ExceptionAddress),
			  szAddr, RLIB_COUNTOF(szAddr), 10);
	TCHAR szPath[RLIB_MAX_PATH] = { 0 };
#if !RLIB_DISABLE_NATIVE_API
	_tcscat_s(szPath, RLIB_COUNTOF(szPath), _T("\\??\\"));
	LPCTSTR lpStartupPath = AppBase::GetStartupPath();
	if (lpStartupPath[0] == _T('\\') && lpStartupPath[1] == _T('\\')) {
		_tcscat_s(szPath, RLIB_COUNTOF(szPath), _T("UNC\\"));
		lpStartupPath += RLIB_COUNTOF_STR(_T("\\\\"));
	} //if
	_tcscat_s(szPath, RLIB_COUNTOF(szPath), lpStartupPath);
#else
	_tcscat_s(szPath, RLIB_COUNTOF(szPath), AppBase::GetStartupPath());
#endif // !RLIB_DISABLE_NATIVE_API
	_tcscat_s(szPath, RLIB_COUNTOF(szPath), szAddr);
	_tcscat_s(szPath, RLIB_COUNTOF(szPath), _T("_crash.dmp"));

	HANDLE hFile;
#if RLIB_DISABLE_NATIVE_API
	hFile = CreateFile(szPath, GENERIC_ALL,
					   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

#else
	UNICODE_STRING ustring;
# ifdef _UNICODE
	RtlInitUnicodeString(&ustring, szPath);
# else
	RtlCreateUnicodeStringFromAsciiz(&ustring, szPath);
# endif // _UNICODE

	// others parameters
	OBJECT_ATTRIBUTES obj;
	IO_STATUS_BLOCK   isb;
	InitializeObjectAttributes(&obj, &ustring,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL, NULL);

	NTSTATUS status = NtCreateFile(&hFile,
								   GENERIC_WRITE | SYNCHRONIZE,
								   &obj, &isb, NULL,
								   FILE_ATTRIBUTE_NORMAL,
								   FILE_SHARE_READ | FILE_SHARE_WRITE,
								   FILE_OVERWRITE_IF,
								   FILE_SYNCHRONOUS_IO_NONALERT, // the I/O Manager maintains the current file position
								   NULL, NULL);
	UNREFERENCED_PARAMETER(status);

#endif // RLIB_DISABLE_NATIVE_API

	BOOL resultDump = FALSE;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
		loExceptionInfo.ExceptionPointers = exceptionInfo;
		loExceptionInfo.ThreadId          = AppBase::GetCurrentThreadId();
		loExceptionInfo.ClientPointers    = TRUE;

		HMODULE dbghelp = Runtime::Marshal::LoadNativeModule(L"dbghelp.dll");
		if (dbghelp != NULL) {
			resultDump = PInvoke(dbghelp, MiniDumpWriteDump)(AppBase::GetCurrentProcess(),
															 AppBase::GetCurrentProcessId(),
															 hFile,
															 static_cast<MINIDUMP_TYPE>(MiniDumpWithPrivateReadWriteMemory |
																						MiniDumpWithDataSegs |
																						MiniDumpWithHandleData |
																						MiniDumpWithIndirectlyReferencedMemory |
																						MiniDumpWithProcessThreadData |
																						MiniDumpWithThreadInfo),
															 &loExceptionInfo, NULL, NULL);
			Runtime::Marshal::UnLoadNativeModule(dbghelp);
		} //if

#if RLIB_DISABLE_NATIVE_API
		CloseHandle(hFile);
#else 
		NtClose(hFile);
#endif // RLIB_DISABLE_NATIVE_API
	} //if	

#if !RLIB_DISABLE_NATIVE_API
# ifndef _UNICODE
	RtlFreeUnicodeString(&ustring);
# endif // !_UNICODE
#endif // !RLIB_DISABLE_NATIVE_API

	return resultDump != FALSE;
}

//-------------------------------------------------------------------------

static bool __capture_stack(HMODULE dbghelp, LPTSTR backtrace, uintptr_t count,
							LPCTSTR prefix /* = nullptr */)
{
	#define MAX_SYMBOL_NAME_LEN 128

	// Initialize dbghelp library.
	if (!PInvoke(dbghelp, SymInitialize)(NtCurrentProcess(), NULL, TRUE)) {
		return false;
	} //if

	TCHAR cdata[RLIB_MAX_PATH];
	union
	{
		SYMBOL_INFO   symbol;
		unsigned char sdata[sizeof(SYMBOL_INFO) + MAX_SYMBOL_NAME_LEN * sizeof(TCHAR)];
	};

	backtrace[0]        = _T('\0');
	symbol.SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol.MaxNameLen   = MAX_SYMBOL_NAME_LEN;

	// Initialize IMAGEHLP_LINE64 structure (file name & line number).
	IMAGEHLP_LINE source_info = { 0 };
	source_info.SizeOfStruct  = sizeof(source_info);

	// Initialize STACKFRAME64 structure.
	CONTEXT    context;
	RtlCaptureContext(&context);
	STACKFRAME stackframe = { 0 };
	DWORD      machineType;
#ifndef _WIN64
	machineType                 = IMAGE_FILE_MACHINE_I386;
	stackframe.AddrPC.Offset    = context.Eip;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrFrame.Offset = context.Ebp;
#else
	machineType                 = IMAGE_FILE_MACHINE_AMD64;
	stackframe.AddrPC.Offset    = context.Rip;
	stackframe.AddrStack.Offset = context.Rsp;
	stackframe.AddrFrame.Offset = context.Rbp;
#endif // !_WIN64
	stackframe.AddrPC.Mode    = AddrModeFlat;
	stackframe.AddrStack.Mode = AddrModeFlat;
	stackframe.AddrFrame.Mode = AddrModeFlat;

	// Enumerate call stack frame.
	while (PInvoke(dbghelp, StackWalk)(machineType, NtCurrentProcess(), NtCurrentThread(), 
									   &stackframe, &context, NULL,
									   PInvoke(dbghelp, SymFunctionTableAccess), 
									   PInvoke(dbghelp, SymGetModuleBase), NULL)) {
		if (stackframe.AddrFrame.Offset == 0) {
			break;
		} //if
		
		// Address
		_sntprintf_s(cdata, _countof(cdata), _TRUNCATE, _T("%s0x%p"), prefix ? prefix : _T(""), reinterpret_cast<LPVOID>(stackframe.AddrPC.Offset));
		_tcscat_s(backtrace, count, cdata);

		// Module
		auto first    = &NtCurrentPeb()->Ldr->InMemoryOrderModuleList;
		auto iterator = first;
		while ((iterator = iterator->Flink) != first) 
		{
			auto table = CONTAINING_RECORD(iterator, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
#ifndef _WIN64
			auto base  = reinterpret_cast<DWORD>(table->DllBase);
#else
			auto base  = reinterpret_cast<DWORD64>(table->DllBase);
#endif // !_WIN64
			if (stackframe.AddrPC.Offset < (base + table->SizeOfImage) && stackframe.AddrPC.Offset >= base) {
				_tcscat_s(backtrace, count, _T(" "));
#ifdef _UNICODE				
				_tcscat_s(backtrace, count, table->BaseDllName.Buffer);
#else
				RT2::UnicodeToMultiByte(cdata, _countof(cdata) - 1, 
										table->BaseDllName.Buffer, wcslen(table->BaseDllName.Buffer) * sizeof(wchar_t));
				cdata[_countof(cdata) - 1] = _T('\0');
				_tcscat_s(backtrace, _countof(backtrace), cdata);
#endif // _UNICODE
				break;
			} //if
		}
		_tcscat_s(backtrace, count, _T("!"));

		// Get symbol.
		if (PInvoke(dbghelp, SymFromAddr)(NtCurrentProcess(), stackframe.AddrPC.Offset, NULL, &symbol)) {
			_tcscat_s(backtrace, count, symbol.Name);

			// Get source information.
			DWORD displacement;
			if (PInvoke(dbghelp, SymGetLineFromAddr)(NtCurrentProcess(), stackframe.AddrPC.Offset, &displacement, &source_info)) {
				_sntprintf_s(cdata, _countof(cdata), _TRUNCATE, _T(" Line %ld"), source_info.LineNumber);
				_tcscat_s(backtrace, count, cdata);
			} else {
				// GetLastError() == ERROR_INVALID_ADDRESS
				_tcscat_s(backtrace, count, _T(" [External Code]"));
			} //if
		} //if

		_tcscat_s(backtrace, count, RLIB_NEWLINE);
	}

	PInvoke(dbghelp, SymCleanup)(NtCurrentProcess());

	return true;
}

//-------------------------------------------------------------------------

LPCTSTR AppBase::CaptureStack(LPTSTR buffer, uintptr_t count, LPCTSTR prefix /* = nullptr */)
{
	buffer[0] = _T('\0');
	HMODULE dbghelp = Runtime::Marshal::LoadNativeModule(L"dbghelp.dll");
	if (dbghelp != NULL) {
		__capture_stack(dbghelp, buffer, count, prefix);
		Runtime::Marshal::UnLoadNativeModule(dbghelp);
	} //if
	return buffer;
}

//-------------------------------------------------------------------------

PVOID AppBase::AddExceptionHandler(ULONG firstHandler,
								   LONG(NTAPI *exceptionHandler)(LPEXCEPTION_POINTERS exceptionInfo))
{
	return AddVectoredExceptionHandler(firstHandler, exceptionHandler);
}

//-------------------------------------------------------------------------

bool AppBase::IsFatalException(LPEXCEPTION_POINTERS exceptionInfo)
{
	//
	// Note: There is a slightly modified layout for HRESULT values below,
	//        after the heading "COM Error Codes".
	//
	//  Values are 32 bit values laid out as follows:
	//
	//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
	//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
	//  +-+-+-+-+-+---------------------+-------------------------------+
	//  |S|R|C|N|r|    Facility         |               Code            |
	//  +-+-+-+-+-+---------------------+-------------------------------+
	//
	//  where
	//
	//      S - Severity - indicates success/fail
	//
	//          0 - Success
	//          1 - Fail (COERROR)
	//
	//      R - reserved portion of the facility code, corresponds to NT's
	//              second severity bit.
	//
	//      C - reserved portion of the facility code, corresponds to NT's
	//              C field.
	//
	//      N - reserved portion of the facility code. Used to indicate a
	//              mapped NT status value.
	//
	//      r - reserved portion of the facility code. Reserved for internal
	//              use. Used to indicate HRESULT values that are not status
	//              values, but are instead message ids for display strings.
	//
	//      Facility - is the facility code
	//
	//      Code - is the facility's status code
	//
	struct HRESULT_INFO
	{
#if RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN
		int  code : 16;
		int  facility : 11;
		bool r : 1;
		bool N : 1;
		bool C : 1;
		bool R : 1;
		bool S : 1;
#else
		bool S : 1;
		bool R : 1;
		bool C : 1;
		bool N : 1;
		bool r : 1;
		int  facility : 11;
		int  code : 16;
#endif // RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN
	};
	struct WIN32ERR_INFO
	{
#if RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN
		unsigned int code : 16;
		unsigned int facility : 12;
		unsigned int R : 1;
		unsigned int C : 1;
		unsigned int Sev : 2;
#else
		unsigned int Sev : 2;
		unsigned int C : 1;
		unsigned int R : 1;
		unsigned int facility : 12;
		unsigned int code : 16;
#endif // RLIB_BYTE_ORDER == RLIB_LITTLE_ENDIAN
	};

	static NTSTATUS exclusive_exceptions[] = {
		EXCEPTION_MS_VC, EXCEPTION_UNCAUGHT_CXX_EXCEPTION,
		EXCEPTION_GUARD_PAGE, EXCEPTION_BREAKPOINT, EXCEPTION_SINGLE_STEP,
		CONTROL_C_EXIT
	};
	static NTSTATUS exclusive_exception_ranges[] = {
		DBG_REPLY_LATER - 1, DBG_COMMAND_EXCEPTION + 1,
		RPC_S_INVALID_STRING_BINDING, RPC_X_BAD_STUB_DATA,
		RPC_S_NO_INTERFACES, RPC_S_INVALID_OBJECT
	};

	NTSTATUS s = static_cast<NTSTATUS>(exceptionInfo->ExceptionRecord->ExceptionCode);
	intptr_t i = 0;
	for (; i < RLIB_COUNTOF(exclusive_exceptions); ++i)
	{
		if (s == exclusive_exceptions[i]) return false;
	} //for
	for (i = 0; i < RLIB_COUNTOF(exclusive_exception_ranges); i += 2) {
		if (s >= exclusive_exception_ranges[i] && s <= exclusive_exception_ranges[i + 1]) {
			return false;
		} //if
	} //for

	// debugger first
	if (AppBase::IsDebuggerPresent()) {
		return false;
	} //if

	// test if ERROR
	auto win32err = reinterpret_cast<WIN32ERR_INFO *>(&exceptionInfo->ExceptionRecord->ExceptionCode);
	return win32err->Sev == 3u;
}

//-------------------------------------------------------------------------

void AppBase::PrintException(LPEXCEPTION_POINTERS exceptionInfo)
{
#ifdef _DEBUG
	static TCHAR __exceptionCode[16] = { _T("0x") };
	LPCTSTR exceptionCode = __exceptionCode, exceptionDescription;

	[&]()
	{
		switch (exceptionInfo->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
			exceptionCode = _T("EXCEPTION_ACCESS_VIOLATION");
			exceptionDescription = _T("The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			exceptionCode = _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			exceptionDescription = _T("The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
			break;
		case EXCEPTION_BREAKPOINT:
			exceptionCode = _T("EXCEPTION_BREAKPOINT");
			exceptionDescription = _T("A breakpoint was encountered.");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			exceptionCode = _T("EXCEPTION_DATATYPE_MISALIGNMENT");
			exceptionDescription = _T("The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			exceptionCode = _T("EXCEPTION_FLT_DENORMAL_OPERAND");
			exceptionDescription = _T("One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			exceptionCode = _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
			exceptionDescription = _T("The thread tried to divide a floating-point value by a floating-point divisor of zero.");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			exceptionCode = _T("EXCEPTION_FLT_INEXACT_RESULT");
			exceptionDescription = _T("The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			exceptionCode = _T("EXCEPTION_FLT_INVALID_OPERATION");
			exceptionDescription = _T("This exception represents any floating-point exception not included in this list.");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			exceptionCode = _T("EXCEPTION_FLT_OVERFLOW");
			exceptionDescription = _T("The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			exceptionCode = _T("EXCEPTION_FLT_STACK_CHECK");
			exceptionDescription = _T("The stack overflowed or underflowed as the result of a floating-point operation.");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			exceptionCode = _T("EXCEPTION_FLT_UNDERFLOW");
			exceptionDescription = _T("The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			exceptionCode = _T("EXCEPTION_ILLEGAL_INSTRUCTION");
			exceptionDescription = _T("The thread tried to execute an invalid instruction.");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			exceptionCode = _T("EXCEPTION_IN_PAGE_ERROR");
			exceptionDescription = _T("The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			exceptionCode = _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
			exceptionDescription = _T("The thread tried to divide an integer value by an integer divisor of zero.");
			break;
		case EXCEPTION_INT_OVERFLOW:
			exceptionCode = _T("EXCEPTION_INT_OVERFLOW");
			exceptionDescription = _T("The result of an integer operation caused a carry out of the most significant bit of the result.");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			exceptionCode = _T("EXCEPTION_INVALID_DISPOSITION");
			exceptionDescription = _T("An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			exceptionCode = _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
			exceptionDescription = _T("The thread tried to continue execution after a noncontinuable exception occurred.");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			exceptionCode = _T("EXCEPTION_PRIV_INSTRUCTION");
			exceptionDescription = _T("The thread tried to execute an instruction whose operation is not allowed in the current machine mode.");
			break;
		case EXCEPTION_SINGLE_STEP:
			exceptionCode = _T("EXCEPTION_SINGLE_STEP");
			exceptionDescription = _T("A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			exceptionCode = _T("EXCEPTION_STACK_OVERFLOW");
			exceptionDescription = _T("The thread used up its stack.");
			break;
		case DBG_CONTROL_C:
			exceptionCode = _T("DBG_CONTROL_C");
			exceptionDescription = _T("CTRL+C is input to a console process that handles CTRL+C signals and is being debugged");
			break;
		case EXCEPTION_UNCAUGHT_CXX_EXCEPTION:
			exceptionCode = _T("EXCEPTION_UNCAUGHT_CXX_EXCEPTION");
			exceptionDescription = _T("an exception that is not in its dynamic-exception-specification");
			break;
		case STATUS_FATAL_APP_EXIT:
			exceptionCode = _T("STATUS_FATAL_APP_EXIT");
			exceptionDescription = _T("Unknown, usually when abort or terminate is explicitly called");
			break;
		default:
			{
				_i64tot_s(static_cast<__int64>(exceptionInfo->ExceptionRecord->ExceptionCode),
						  __exceptionCode + 2, RLIB_COUNTOF(__exceptionCode) - 2, 16);
				exceptionDescription = _T("Unknown Exception");
			}
			break;
		}
	}();

	TCHAR __exception[RLIB_DEFAULT_MAX_BUFFER_SIZE * 3], __backtrace[RLIB_DEFAULT_MAX_BUFFER_SIZE * 2];
	__exception[0] = _T('\0');
	_tcscat_s(__exception, RLIB_COUNTOF(__exception), exceptionDescription);

	_tcscat_s(__exception, RLIB_COUNTOF(__exception), _T("("));
	_tcscat_s(__exception, RLIB_COUNTOF(__exception), exceptionCode);
	_tcscat_s(__exception, RLIB_COUNTOF(__exception), _T(")") RLIB_NEWLINE RLIB_NEWLINE);
	_tcscat_s(__exception, RLIB_COUNTOF(__exception), AppBase::CaptureStack(__backtrace, RLIB_COUNTOF(__backtrace)));
	_tcscat_s(__exception, RLIB_COUNTOF(__exception), _T("Press ignore to generate dump..."));
	
	alert(__exception);

#else

	UNREFERENCED_PARAMETER(exceptionInfo);

#endif // _DEBUG
}

//-------------------------------------------------------------------------

static LONG NTAPI DefaultExceptionHandler(LPEXCEPTION_POINTERS exceptionInfo)
{
	static volatile long __nested_break_point = 0L;
	if (!AppBase::IsFatalException(exceptionInfo)) {
		if (!__nested_break_point && !AppBase::IsDebuggerPresent() &&
			exceptionInfo->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
			// forces to generate dump file
			AppBase::GenerateCrashDump(exceptionInfo);
		} //if

		return EXCEPTION_CONTINUE_SEARCH;
	} //if

	__nested_break_point = 1L;
	AppBase::PrintException(exceptionInfo);
	__nested_break_point = 0L;
	AppBase::GenerateCrashDump(exceptionInfo);
	if (exceptionInfo->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE) {
		AppBase::Exit(exceptionInfo->ExceptionRecord->ExceptionCode);
	} //if
	return EXCEPTION_CONTINUE_SEARCH;
}

//-------------------------------------------------------------------------

void AppBase::EnableCrashDump()
{
	AppBase::AddExceptionHandler(FALSE, DefaultExceptionHandler);
}

//-------------------------------------------------------------------------

int AppBase::GetEnvironmentVersion()
{
	return static_cast<int>(RLIB_VER);
}

//-------------------------------------------------------------------------

LPCTSTR AppBase::GetBuildTimestamp()
{
	return _T(RLIB_COMPILE_TIMESTAMP);
}