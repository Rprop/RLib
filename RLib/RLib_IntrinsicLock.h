/********************************************************************
	Created:	2016/07/16  21:59
	Filename: 	RLib_IntrinsicLock.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_Interlocked.h"

#define __RLIB_LOCK_IS_FREE  0L
#define __RLIB_LOCK_IS_TAKEN 1L

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		/// <summary>
		/// using Interlocked* intrinsics to do manual synchronization
		/// </summary>
		class RLIB_API IntrinsicLock
		{
		public:
			static void lock(volatile long *pl);
			static bool tryLock(volatile long *pl);
			static void unlock(volatile long *pl);
			static void reentrant_lock(volatile long *pl);
			static bool reentrant_tryLock(volatile long *pl);
			static void reentrant_unlock(volatile long *pl);
		};
		/// <summary>
		/// using atomic types to synchronize memory accesses among different threads
		/// </summary>
		class AtomicLock
		{
		protected:
			mutable volatile long l;
		public:
			RLIB_INLINE AtomicLock() : l(__RLIB_LOCK_IS_FREE) {
				assert((reinterpret_cast<uintptr_t>(&l) % sizeof(l)) == 0);
			}
			RLIB_INLINE ~AtomicLock() {
				assert(this->l == __RLIB_LOCK_IS_FREE);
			}
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			RLIB_INLINE void Lock() const
			{
				_Acquires_lock_(this->l);
				IntrinsicLock::lock(&this->l);
			}
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			RLIB_INLINE bool TryLock() const
			{
				_Acquires_lock_(this->l);
				return IntrinsicLock::tryLock(&this->l);
			}
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			RLIB_INLINE void UnLock() const
			{
				_Releases_lock_(this->l);
				IntrinsicLock::unlock(&this->l);
			}

		public: // compatible with Monitor
			RLIB_INLINE AtomicLock(_In_ DWORD) : AtomicLock() {}
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			RLIB_INLINE void Enter() const {
				return this->Lock();
			}
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			RLIB_INLINE bool TryEnter() const {
				return this->TryLock();
			}
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			RLIB_INLINE void Exit() const {
				return this->UnLock();
			}
		};
		/// <summary>
		/// A ReentrantLock is owned by the thread last successfully locking, but not yet unlocking it.
		/// A thread invoking lock will return, successfully acquiring the lock, when the lock is not owned by another thread.
		/// The method will return immediately if the current thread already owns the lock.
		/// </summary>
		class ReentrantLock : private IntrinsicLock
		{
		protected:
			mutable volatile long l;
#ifdef _DEBUG
			mutable volatile long c;
#endif // _DEBUG
		public:
			RLIB_INLINE ReentrantLock() : l(__RLIB_LOCK_IS_FREE) {
#ifdef _DEBUG
				this->c = 0;
#endif // _DEBUG
				assert(__RLIB_LOCK_IS_FREE == 0);
				assert((reinterpret_cast<uintptr_t>(&l) % sizeof(l)) == 0);
			}
			RLIB_INLINE ~ReentrantLock() {
				assert(this->l == __RLIB_LOCK_IS_FREE);
			}
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			RLIB_INLINE void Lock() const
			{
				_Acquires_lock_(this->l);
				IntrinsicLock::reentrant_lock(&this->l);
#ifdef _DEBUG
				Interlocked::Increment(&this->c);
#endif // _DEBUG			
			}
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			RLIB_INLINE bool TryLock() const
			{
				_Acquires_lock_(this->l);
#ifdef _DEBUG
				if (IntrinsicLock::reentrant_tryLock(&this->l)) {
					Interlocked::Increment(&this->c);
					return true;
				} //if
				return false;
#else
				return IntrinsicLock::reentrant_tryLock(&this->l);
#endif // _DEBUG	
				
			}
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			RLIB_INLINE void UnLock() const
			{
				_Releases_lock_(this->l);
				IntrinsicLock::reentrant_unlock(&this->l);
#ifdef _DEBUG
				Interlocked::Decrement(&this->c);
#endif // _DEBUG	
			}
			/// <summary>
			/// Queries the number of holds on this lock by the current thread. 
			/// A thread has a hold on a lock for each lock action that is not matched by an unlock action.
			/// The hold count information is typically only used for testing and debugging purposes.
			/// </summary>
			RLIB_INLINE long GetHoldCount() {
#ifdef _DEBUG
				return this->c;
#else
				return 0;
#endif // _DEBUG	
			}
			/// <summary>
			/// Queries the number of holds on this lock by the current thread. 
			/// A thread has a hold on a lock for each lock action that is not matched by an unlock action.
			/// The hold count information is typically only used for testing and debugging purposes.
			/// </summary>
			RLIB_PROPERTY_GET(long HoldCount, GetHoldCount);

		public: // compatible with Monitor
			RLIB_INLINE ReentrantLock(_In_ DWORD) : ReentrantLock() {}
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			RLIB_INLINE void Enter() const {
				return this->Lock();
			}
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			RLIB_INLINE bool TryEnter() const {
				return this->TryLock();
			}
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			RLIB_INLINE void Exit() const {
				return this->UnLock();
			}
		};
	}
}