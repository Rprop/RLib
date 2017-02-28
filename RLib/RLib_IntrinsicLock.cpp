/********************************************************************
	Created:	2016/07/16  22:01
	Filename: 	RLib_IntrinsicLock.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"
#include "RLib_IntrinsicLock.h"
using namespace System::Threading;

//-------------------------------------------------------------------------

void IntrinsicLock::lock(volatile long *pl)
{
	_Acquires_lock_(pl);

	while (Interlocked::Exchange(pl, __RLIB_LOCK_IS_TAKEN) != __RLIB_LOCK_IS_FREE) {
		// spin!
	}
	// At this point, the lock is acquired.
}

//-------------------------------------------------------------------------

bool IntrinsicLock::tryLock(volatile long *pl)
{
	_Acquires_lock_(pl);
	return Interlocked::Exchange(pl, __RLIB_LOCK_IS_TAKEN) == __RLIB_LOCK_IS_FREE;
}

//-------------------------------------------------------------------------

void IntrinsicLock::unlock(volatile long *pl)
{
	_Releases_lock_(pl);
#ifdef _DEBUG
	assert(Interlocked::Exchange(pl, __RLIB_LOCK_IS_FREE) == __RLIB_LOCK_IS_TAKEN);
#else
	Interlocked::Exchange(pl, __RLIB_LOCK_IS_FREE);
#endif // _DEBUG
}

//-------------------------------------------------------------------------

void IntrinsicLock::reentrant_lock(volatile long *pl)
{
	_Acquires_lock_(pl);
	long current_thread_id = static_cast<long>(AppBase::GetCurrentThreadId());
	assert(current_thread_id != __RLIB_LOCK_IS_FREE);

	// If l == __RLIB_LOCK_IS_FREE, it is set to __RLIB_LOCK_IS_TAKEN
	// atomically, so only 1 caller gets the lock.
	// If l == current_thread_id,
	// the result is current_thread_id, and the while loop break.
	// Otherwise, the while loop keeps spinning.
	long ret_val;
	do {
		// spin!
		ret_val = Interlocked::CompareExchange(pl, current_thread_id, __RLIB_LOCK_IS_FREE);
	} while (ret_val != __RLIB_LOCK_IS_FREE && ret_val != current_thread_id);

	// At this point, the lock is acquired.
}

//-------------------------------------------------------------------------

bool IntrinsicLock::reentrant_tryLock(volatile long *pl)
{
	_Acquires_lock_(pl);
	long current_thread_id = static_cast<long>(AppBase::GetCurrentThreadId());
	assert(current_thread_id != __RLIB_LOCK_IS_FREE);

	long ret_val = Interlocked::CompareExchange(pl, current_thread_id, __RLIB_LOCK_IS_FREE);
	return ret_val == __RLIB_LOCK_IS_FREE || ret_val == current_thread_id;
}

//-------------------------------------------------------------------------

void IntrinsicLock::reentrant_unlock(volatile long *pl)
{
	_Releases_lock_(pl);
#ifdef _DEBUG
	long current_thread_id = static_cast<long>(AppBase::GetCurrentThreadId());
	assert(current_thread_id != __RLIB_LOCK_IS_FREE);
	assert(Interlocked::Exchange(pl, __RLIB_LOCK_IS_FREE) == current_thread_id);
#else
	Interlocked::Exchange(pl, __RLIB_LOCK_IS_FREE);
#endif // _DEBUG
}