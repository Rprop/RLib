/********************************************************************
	Created:	2016/07/14  22:05
	Filename: 	RLib_Monitor.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Monitor.h"
#include "native/RLib_Native.h"
#if RLIB_DISABLE_NATIVE_API
#include <synchapi.h>
#endif // RLIB_DISABLE_NATIVE_API
using namespace System::Threading;

//-------------------------------------------------------------------------

Monitor::Monitor(_In_ DWORD dwSpinCount /* = 4000 */)
{
#if RLIB_DISABLE_NATIVE_API
#if (WINVER <= _WIN32_WINNT_WIN7)
	BOOL status = InitializeCriticalSectionAndSpinCount(&this->m_cs, dwSpinCount);
#else
	BOOL status = InitializeCriticalSectionEx(&this->m_cs, dwSpinCount,
											  RTL_CRITICAL_SECTION_FLAG_NO_DEBUG_INFO);
#endif
	assert(status == TRUE);
#else
	NTSTATUS status = RtlInitializeCriticalSectionAndSpinCount(&this->m_cs, dwSpinCount);
	assert(status == STATUS_SUCCESS);
#endif // RLIB_DISABLE_NATIVE_API

	UNREFERENCED_PARAMETER(status);
}

//-------------------------------------------------------------------------

Monitor::~Monitor()
{
#if RLIB_DISABLE_NATIVE_API
	DeleteCriticalSection(&this->m_cs);
#else
	RtlDeleteCriticalSection(&this->m_cs);
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

void Monitor::Enter() const
{
	_Acquires_lock_(this->m_cs);
#if RLIB_DISABLE_NATIVE_API
	EnterCriticalSection(const_cast<PCRITICAL_SECTION>(&this->m_cs));
#else
	RtlEnterCriticalSection(const_cast<PRTL_CRITICAL_SECTION>(&this->m_cs));
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

bool Monitor::TryEnter() const
{
	_Acquires_lock_(this->m_cs);
#if RLIB_DISABLE_NATIVE_API
	return TryEnterCriticalSection(const_cast<PCRITICAL_SECTION>(&this->m_cs)) == TRUE;
#else
	return RtlTryEnterCriticalSection(const_cast<PRTL_CRITICAL_SECTION>(&this->m_cs)) == TRUE;
#endif // RLIB_DISABLE_NATIVE_API
}

//-------------------------------------------------------------------------

void Monitor::Exit() const
{
	_Releases_lock_(this->m_cs);
#if RLIB_DISABLE_NATIVE_API
	LeaveCriticalSection(const_cast<PCRITICAL_SECTION>(&this->m_cs));
#else
	RtlLeaveCriticalSection(const_cast<PRTL_CRITICAL_SECTION>(&this->m_cs));
#endif // RLIB_DISABLE_NATIVE_API
}
