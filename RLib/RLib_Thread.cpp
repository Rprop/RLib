/********************************************************************
Created:	2012/02/05  22:41
Filename: 	RLib_Thread.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Thread.h"
#include "RLib_Interlocked.h"
#include "native/RLib_Native.h"
#include <windows.h>

#define __change_state(p,s) Interlocked::Exchange(reinterpret_cast<volatile long *>(&p->m_state), static_cast<long>(s));
#pragma warning(push)
#pragma warning(disable:4702)

using namespace System::IO;
using namespace System::Threading;

//-------------------------------------------------------------------------

DWORD _stdcall Thread::__threadProcWrapper(Thread *lpMySelf)
{
	assert(lpMySelf != nullptr || !"thread exception");

	__change_state(lpMySelf, ThreadState::Running);

    DWORD retval = 0;

	switch (lpMySelf->m_startType) {
	case Runnable::Cpp:
		{
			reinterpret_cast<Runnable::CppThreadStart>(lpMySelf->m_startAddress)();
			break;
		}
	case Runnable::CppParameterized:
		{
			reinterpret_cast<Runnable::CppParameterizedThreadStart>(lpMySelf->m_startAddress)(lpMySelf->
																							  m_parameter);
			break;
		}
	case Runnable::StandardParameterized:
		{
			retval = reinterpret_cast<Runnable::StdParameterizedThreadStart>(lpMySelf->
																			 m_startAddress)(lpMySelf->m_parameter);
			break;
		}
	case Runnable::Standard:
		{
			retval = reinterpret_cast<Runnable::StdThreadStart>(lpMySelf->
																m_startAddress)();
			break;
		}
	default:
		RLIB_NODEFAULT("Invalid Executor!");
	}

    if (lpMySelf->IsBackground) {
		lpMySelf->IsSuppressChangeState = true;
        delete lpMySelf;
    } else {
		__change_state(lpMySelf, ThreadState::Stopped);
	} //if

    Thread::ExitThread(retval);


    return retval;
}

//-------------------------------------------------------------------------

Thread::Thread(HANDLE hThread, DWORD ThreadId, BOOL IsRunning)
{
    this->m_hThread      = hThread;
    this->m_ThreadId     = ThreadId;
    this->m_state        = IsRunning ? ThreadState::Running : ThreadState::Suspended;    
	this->m_startAddress = nullptr;
	this->m_parameter    = nullptr;
	this->m_startType    = Runnable::StandardParameterized;
	this->IsBackground   = false;
	this->IsSuppressChangeState = false;
}

//-------------------------------------------------------------------------

Thread::Thread(const Runnable &run)
{
	this->m_startAddress = run.callback;
    this->m_parameter    = run.state;
    this->m_startType    = Runnable::CppParameterized;
    this->thread_create();
}

//-------------------------------------------------------------------------

Thread::Thread(Runnable::StdParameterizedThreadStart lpRoutine, LPVOID lpParameter)
{
    this->m_startAddress = lpRoutine;
    this->m_parameter    = lpParameter;
    this->m_startType    = Runnable::StandardParameterized;
    this->thread_create();
}

//-------------------------------------------------------------------------

Thread::Thread(Runnable::StdThreadStart lpRoutine)
{
	this->m_startAddress = lpRoutine;
	this->m_parameter    = nullptr;
    this->m_startType    = Runnable::Standard;
    this->thread_create();
}

//-------------------------------------------------------------------------

Thread::Thread(Runnable::CppThreadStart lpRoutine)
{
	this->m_startAddress = lpRoutine;
	this->m_parameter    = nullptr;
    this->m_startType    = Runnable::Cpp;
    this->thread_create();
}

//-------------------------------------------------------------------------

Thread::Thread(Runnable::CppParameterizedThreadStart lpRoutine, LPVOID lpParameter)
{
	this->m_startAddress = lpRoutine;
	this->m_parameter    = lpParameter;
	this->m_startType    = Runnable::CppParameterized;
	this->thread_create();
}

//-------------------------------------------------------------------------

void Thread::SetParameter(LPVOID lpParameter)
{
	if (this->m_state != ThreadState::Unstarted) {
		trace(!"invalid state to set parameter");
		return;
	} //if
	this->m_parameter = lpParameter;
}

//-------------------------------------------------------------------------

Thread::~Thread()
{
#ifdef _DEBUG
	if(this->ThreadId == AppBase::GetCurrentThreadId() && !this->IsSuppressChangeState) {
		trace(!"If you want to do so, must set IsSuppressChangeState = true"
			  "and call ExitThread before thread return");
	} //if
#endif // _DEBUG
    this->Close();
}

//-------------------------------------------------------------------------

void Thread::thread_create()
{
	this->m_ThreadId = 0;
    // If the function fails, the return value is NULL
	this->m_hThread = CreateRemoteThread(AppBase::GetCurrentProcess(), NULL, 0,
										 reinterpret_cast<LPTHREAD_START_ROUTINE>(__threadProcWrapper), this,
										 CREATE_SUSPENDED, &this->m_ThreadId);
    if (this->m_hThread == nullptr) {
        Exception::FormatException(&this->m_error, Exception::GetLastErrorId());
        RLIB_SetExceptionInfo(this->m_error);
		alert(this->m_error.Message);
    } //if
    this->m_state               = ThreadState::Unstarted;
    this->IsBackground          = false;
	this->IsSuppressChangeState = false;
}

//-------------------------------------------------------------------------

bool Thread::Start()
{
    if (this->m_hThread == nullptr) {
        trace(!"thread was not created successfully");
        return false;
    } //if
    return this->Resume();
}

//-------------------------------------------------------------------------

bool Thread::Alert()
{
	if (Thread::AlertThread(this->m_hThread)) {
		__change_state(this, ThreadState::Running);
		return true;
	} //if
	Exception::FormatException(&this->m_error, Exception::GetLastErrorId());
	return false;
}

//-------------------------------------------------------------------------

bool Thread::Resume()
{
    if (this->m_state != ThreadState::Unstarted && 
		this->m_state != ThreadState::Suspended) {
        trace(!"invalid state to resume");
		return false;
    } //if

    NTSTATUS status = NtResumeThread(m_hThread, nullptr);
	if (status != STATUS_SUCCESS) {
		this->setException(status);
		RLIB_SetExceptionInfo(this->m_error);
		trace(!"failed to resume the thread");
		return false;
	} //if

	__change_state(this, ThreadState::Running);
    return true;
}

//-------------------------------------------------------------------------

bool Thread::Suspend()
{
    if (this->m_state != ThreadState::Running) {
        trace(!"invalid state to suspend");
		return false;
    } //if

	NTSTATUS status = NtSuspendThread(m_hThread, nullptr);
    if (status == STATUS_SUCCESS) {
		__change_state(this, ThreadState::Suspended);
        return true;
    } //if

    this->setException(status);
    RLIB_SetExceptionInfo(this->m_error);
    trace(!"failed to suspend the thread");
    return false;
}

//-------------------------------------------------------------------------

bool Thread::Abort(long exitstatus /* = -1 */)
{
	if (this->m_state != ThreadState::Running && 
		this->m_state != ThreadState::Suspended) {
		trace(!"invalid state to abort");
	} //if

    NTSTATUS status = NtTerminateThread(m_hThread, static_cast<NTSTATUS>(exitstatus));
	if (status == STATUS_SUCCESS) {
		__change_state(this, ThreadState::Aborted);
		return true;
	} //if

    this->setException(status);
    RLIB_SetExceptionInfo(this->m_error);
    trace(!"failed to abort the thread");
    return false;
}

//-------------------------------------------------------------------------

WaitStatus Thread::Wait(DWORD millisecondsTimeout /* = INFINITE */)
{
	if (this->m_state == ThreadState::Running) {
		return WaitHandle::WaitOne(this->m_hThread, TRUE, millisecondsTimeout);
	} //if
    return WAIT_INVALID_HANDLE;
}

//-------------------------------------------------------------------------

bool Thread::Close()
{
	if (m_hThread != nullptr) {
		if (CloseHandle(m_hThread) == TRUE) {
			this->m_hThread = NULL;
			return true;
		} //if
		Exception::FormatException(&this->m_error, Exception::GetLastErrorId());
		RLIB_SetExceptionInfo(this->m_error);
	} //if
    return false;
}

//-------------------------------------------------------------------------

Thread::operator HANDLE()
{
    return m_hThread;
}

//-------------------------------------------------------------------------

const DWORD Thread::GetThreadId() const
{
    return m_ThreadId;
}

//-------------------------------------------------------------------------

void Thread::SetThreadName(LPCSTR name)
{
	this->SetThreadName(this->ThreadId, name);
}

//-------------------------------------------------------------------------

void Thread::SetThreadName(DWORD threadId, LPCSTR name)
{
#ifdef _DEBUG
	if (!AppBase::IsDebuggerPresent()) {
		return;
	} //if

	struct
	{
		DWORD  dwType;     // must be 0x1000
		LPCSTR szName;     // pointer to name (in user addr space)
		DWORD  dwThreadID; // thread ID (-1 = caller thread)
		DWORD  dwFlags;    // reserved for future use, must be zero
	} threadNameInfo;
	
	threadNameInfo.dwType     = 0x1000;
	threadNameInfo.szName     = name;
	threadNameInfo.dwThreadID = threadId;
	threadNameInfo.dwFlags    = 0;

	const DWORD EXCEPTION_MS_VC = 0x406D1388;
	RaiseException(EXCEPTION_MS_VC, 0, 4, 
				   reinterpret_cast<ULONG_PTR *>(&threadNameInfo));
#else
	UNREFERENCED_PARAMETER(threadId);
	UNREFERENCED_PARAMETER(name);
#endif // _DEBUG
}

//-------------------------------------------------------------------------

bool Thread::GetIsAlive()const
{
    return(this->State == ThreadState::Running || this->State
		== ThreadState::Suspended);
}

//-------------------------------------------------------------------------

ThreadState Thread::GetState() const
{
	return this->m_state;
}

//-------------------------------------------------------------------------

WaitStatus Thread::Sleep(DWORD dwMilliseconds /* = INFINITE */,
						 bool bAlertable /* = true */)
{
	LARGE_INTEGER timeOut;

	if (dwMilliseconds == -1) {
		// If Sleep( -1 ) then delay for the longest possible integer
		// relative to now
		timeOut.LowPart  = 0x0;
		timeOut.HighPart = 0x80000000;
	} else {
		timeOut.QuadPart = static_cast<LONGLONG>(UInt32x32To64(dwMilliseconds, 10000));
		timeOut.QuadPart *= -1;
	} //if
	
	NTSTATUS status = NtDelayExecution(static_cast<BOOLEAN>(bAlertable), &timeOut);

	return status == STATUS_SUCCESS ? WAIT_TIMEOUTED : WAIT_ALERTED;
}

//-------------------------------------------------------------------------

bool Thread::AlertThread(HANDLE hThread)
{
	NTSTATUS status = NtAlertThread(hThread);
	if (status == STATUS_SUCCESS) {
		return true;
	} //if

	Exception::SetLastException(status, false);
	return false;
}

//-------------------------------------------------------------------------

ULONG Thread::AlertResumeThread(HANDLE hThread)
{
	ULONG suspendCount;
	NTSTATUS status = NtAlertResumeThread(hThread, &suspendCount);
	if (status == STATUS_SUCCESS) {
		return suspendCount;
	} //if

	Exception::SetLastException(status, false);
	return ULONG_MAX;
}

//-------------------------------------------------------------------------

ThreadPriority Thread::GetPriority() const
{
    ULONG returnLength;
	THREAD_BASIC_INFORMATION threadInfo = { 0 };
	NTSTATUS status = NtQueryInformationThread(this->m_hThread,
											   static_cast<THREADINFOCLASS>(ThreadPriorityInfo), 
											   &threadInfo, sizeof(threadInfo),
											   &returnLength);
	if (status == STATUS_SUCCESS) {
		return static_cast<ThreadPriority>(threadInfo.Priority);
	} //if
    this->setException(status);
	RLIB_SetExceptionInfo(this->m_error);
    return ThreadPriority::ErrorPriority;
}

//-------------------------------------------------------------------------

bool Thread::SetPriority(ThreadPriority newPriority)
{
	THREAD_BASIC_INFORMATION threadInfo = { 0 };
    threadInfo.Priority = static_cast<KPRIORITY>(newPriority);

	NTSTATUS status = NtSetInformationThread(this->m_hThread,
											 ThreadPriorityInfo,
											 &threadInfo, sizeof(threadInfo));
	if (status == STATUS_SUCCESS) {
		return true;
	} //if

    this->setException(status);
    RLIB_SetExceptionInfo(this->m_error);
    return false;
}

//-------------------------------------------------------------------------

ThreadException *Thread::GetLastException()
{
    return &this->m_error;
}

//-------------------------------------------------------------------------

void Thread::ExitThread(DWORD code /* = 0 */)
{
    AppBase::ExitThread(code);
}

//-------------------------------------------------------------------------

Thread *Thread::GetCurrentThread()
{
	auto lpthread = new Thread(AppBase::GetCurrentThread(),
							   AppBase::GetCurrentThreadId(),
							   FALSE);
	if (lpthread != nullptr) lpthread->IsSuppressChangeState = true;
	return lpthread;
}


//-------------------------------------------------------------------------

void Thread::setException(NTSTATUS status) const
{
    Exception::FormatException(&this->m_error, RtlNtStatusToDosErrorNoTeb(status));
	Exception::SetLastErrorId(RtlNtStatusToDosErrorNoTeb(status));
}

#pragma warning(pop)