/********************************************************************
Created:	2011/10/15  8:09
Filename: 	Threading.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Threading.h"
#include "RLib_GlobalizeString.h"
#include "RLib_StringHelper.h"
#include "RLib_Exception.h"
#include "native/RLib_Native.h"
#if RLIB_DISABLE_NATIVE_API
#include <synchapi.h>
#endif // RLIB_DISABLE_NATIVE_API

using namespace System::Threading;

//-------------------------------------------------------------------------

WaitStatus WaitHandle::WaitOne(HANDLE handle, bool bAlertable, DWORD millisecondsTimeout)
{
    LARGE_INTEGER timeo;
	timeo.QuadPart  = static_cast<LONGLONG>(UInt32x32To64(millisecondsTimeout, 10000)) * -1;

	NTSTATUS status = NtWaitForSingleObject(handle, static_cast<BOOLEAN>(bAlertable),
											millisecondsTimeout != INFINITE ? &timeo : NULL);
	switch (status) 
	{
	case STATUS_ACCESS_DENIED:
		return WAIT_ACCESS_DENIED;
	case STATUS_ALERTED:
		return WAIT_ALERTED;
	case STATUS_INVALID_HANDLE:
		return WAIT_INVALID_HANDLE;
	case STATUS_SUCCESS:
		return WAIT_SUCCESS;
	case STATUS_TIMEOUT:
		return WAIT_TIMEOUTED;
	case STATUS_USER_APC:
		return WAIT_USER_APC;
	}

	Exception::SetLastException(status, true);
	return static_cast<WaitStatus>(status);
}

//-------------------------------------------------------------------------

WaitStatus WaitHandle::Wait(ULONG nCount, HANDLE *pHandles, bool bWaitAll, 
							bool bAlertable, DWORD millisecondsTimeout)
{
	LARGE_INTEGER timeo; // 100 ns
	timeo.QuadPart  = static_cast<LONGLONG>(UInt32x32To64(millisecondsTimeout, 10000)) * -1;

    NTSTATUS status = NtWaitForMultipleObjects(nCount, pHandles,
											   bWaitAll ? WaitAll : WaitAny, 
											   static_cast<BOOLEAN>(bAlertable),
											   millisecondsTimeout != INFINITE ? &timeo: NULL);
	switch(status)
	{
	case STATUS_ACCESS_DENIED:
		return WAIT_ACCESS_DENIED;
	case STATUS_ALERTED:
		return WAIT_ALERTED;
	case STATUS_INVALID_HANDLE:
		return WAIT_INVALID_HANDLE;
	case STATUS_SUCCESS:
			return WAIT_SUCCESS;
	case STATUS_TIMEOUT:
			return WAIT_TIMEOUTED;
	case STATUS_USER_APC:
			return WAIT_USER_APC;
	}

	Exception::SetLastException(status, true);
	return static_cast<WaitStatus>(status);
}

//-------------------------------------------------------------------------

WaitHandle::~WaitHandle()
{
	if (this->m_name.Buffer != nullptr){
		String::Collect(this->m_name.Buffer);
	} //if

	NTSTATUS status = NtClose(this->Handle);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
	} //if
}

//-------------------------------------------------------------------------

WaitStatus WaitHandle::WaitOne(DWORD millisecondsTimeout /* = INFINITE */)
{
	return WaitHandle::WaitOne(this->Handle, TRUE, millisecondsTimeout);
}

//-------------------------------------------------------------------------

Mutex::Mutex(bool bOwner /* = false */, const String &szName /* = Nothing */)
{
    //
    // If bOwner is true, Mutant is created with non-signaled state,
    // Caller should call NtReleaseMutant after program initialization.
    // 

    NTSTATUS status;

	if (szName.IsNullOrEmpty()) {
		this->m_name.Buffer = nullptr;
		status = NtCreateMutant(&this->Handle, MUTANT_ALL_ACCESS, NULL,
								static_cast<BOOLEAN>(bOwner));
	} else {
		GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
		RtlInitUnicodeString(&this->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

		InitializeObjectAttributes(&this->m_obj, &this->m_name,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);
	
		status = NtCreateMutant(&this->Handle, MUTANT_ALL_ACCESS,
								&this->m_obj, 
								static_cast<BOOLEAN>(bOwner));
	} //if

	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
	} //if
}

//-------------------------------------------------------------------------

LONG Mutex::ReleaseMutex()
{
    LONG previousCount;
    NTSTATUS status = NtReleaseMutant(this->Handle, &previousCount);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		return -1;
	} //if
    return previousCount;
}

//-------------------------------------------------------------------------

Mutex *Mutex::OpenExisting(const String &szName)
{
    Mutex *lpmutex = new Mutex(nullptr);
	if (!lpmutex) {
		trace(!"create Mutex failed!");
		return nullptr;
	} //if

	GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
    RtlInitUnicodeString(&lpmutex->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

    InitializeObjectAttributes(&lpmutex->m_obj, &lpmutex->m_name,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);

    NTSTATUS status = NtOpenMutant(&lpmutex->Handle, MUTANT_ALL_ACCESS, &lpmutex->m_obj);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		delete lpmutex;
		lpmutex = nullptr;
	} //if

    return lpmutex;
}

//-------------------------------------------------------------------------

Event::Event(EventType eType, bool bInitialState, const String &szName /* = Nothing */)
{
    NTSTATUS status;

	if (szName.IsNullOrEmpty()) {
		this->m_name.Buffer = nullptr;
		status = NtCreateEvent(&this->Handle, EVENT_ALL_ACCESS, NULL, static_cast<EVENT_TYPE>(eType),
							   static_cast<BOOLEAN>(bInitialState));
	} else {
		GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
		RtlInitUnicodeString(&this->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

		InitializeObjectAttributes(&this->m_obj, &this->m_name,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);

		status = NtCreateEvent(&this->Handle, EVENT_ALL_ACCESS, &this->m_obj,
							   static_cast<EVENT_TYPE>(eType),
							   static_cast<BOOLEAN>(bInitialState));
	} //if

	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
	} //if
}

//-------------------------------------------------------------------------

LONG Event::SetSignal()
{
    long previousState;
    NTSTATUS status = NtSetEvent(this->Handle, &previousState);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		previousState = -1;
	} //if
    return previousState;
}

//-------------------------------------------------------------------------

LONG Event::Pulse()
{
	LONG previousState;
	NTSTATUS status = NtPulseEvent(this->Handle, &previousState);
	// Function sets event to signaled state, releases all (or one - dependly of EVENT_TYPE)
	// waiting threads, and resets event to non-signaled state. 
	// If they're no waiting threads, NtPulseEvent just clear event state.
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		previousState = -1;
	} //if
	return previousState;
}

//-------------------------------------------------------------------------

void Event::Clear()
{
    NTSTATUS status = NtClearEvent(this->Handle);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
	} //if
}

//-------------------------------------------------------------------------

Event *Event::OpenExisting(const String &szName)
{
    Event *lpevt = new Event;
	if (!lpevt) {
		trace(!"create event failed!");
		return nullptr;
	} //if

	GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
	RtlInitUnicodeString(&lpevt->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

    InitializeObjectAttributes(&lpevt->m_obj, &lpevt->m_name,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);

    NTSTATUS status = NtOpenEvent(&lpevt->Handle, EVENT_ALL_ACCESS, &lpevt->m_obj);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		delete lpevt;
		lpevt = nullptr;
	} //if

    return lpevt;
}

//-------------------------------------------------------------------------

Semaphore::Semaphore(ULONG nInitialCount, ULONG nMaximumCount, const String &szName /* = Nothing */)
{
	//
	// If Owner is true, Mutant is created with non-signaled state,
	// Caller should call NtReleaseMutant after program initialization.
	// 

	NTSTATUS status;
	if (szName.IsNullOrEmpty()) {
		this->m_name.Buffer = nullptr;
		status = NtCreateSemaphore(&this->Handle, SEMAPHORE_ALL_ACCESS, NULL,
								   nInitialCount, nMaximumCount);
	} else {
		GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
		RtlInitUnicodeString(&this->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

		InitializeObjectAttributes(&this->m_obj, &this->m_name,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);

		status = NtCreateSemaphore(&this->Handle, SEMAPHORE_ALL_ACCESS, &this->m_obj,
								   nInitialCount, nMaximumCount);
	} //if

	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
	} //if
}

//-------------------------------------------------------------------------

ULONG Semaphore::Release(ULONG releaseCount /* = 1 */)
{
	ULONG previousCount;
	NTSTATUS status = NtReleaseSemaphore(this->Handle, releaseCount, &previousCount);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		return ULONG_MAX;
	} //if
	return previousCount;
}

//-------------------------------------------------------------------------

Semaphore *Semaphore::OpenExisting(const String &szName)
{
	Semaphore *lpsep = new Semaphore;
	if (!lpsep) {
		trace(!"create semaphore failed!");
		return nullptr;
	} //if

	GlobalizeString u_name(_R("\\BaseNamedObjects\\") + szName);
	RtlInitUnicodeString(&lpsep->m_name, u_name.SuppressFinalize(u_name.toUnicode()));

	InitializeObjectAttributes(&lpsep->m_obj, &lpsep->m_name, 
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, nullptr);

	NTSTATUS status = NtOpenSemaphore(&lpsep->Handle, SEMAPHORE_ALL_ACCESS,
									  &lpsep->m_obj);
	if (status != STATUS_SUCCESS) {
		Exception::SetLastException(status, true);
		delete lpsep;
		lpsep = nullptr;
	} //if

	return lpsep;
}