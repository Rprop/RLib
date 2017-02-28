/********************************************************************
	Created:	2016/07/14  22:00
	Filename: 	RLib_Monitor.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_AppBase.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		/// <summary>
		/// Provides a mechanism that synchronizes access to objects.
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Monitor
		{
		private:
#if RLIB_DISABLE_NATIVE_API
			CRITICAL_SECTION     m_cs;
#else
			RTL_CRITICAL_SECTION m_cs;
#endif // RLIB_DISABLE_NATIVE_API

		public:
			Monitor(_In_ DWORD dwSpinCount = 4000);
			~Monitor();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			void Enter() const;
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			bool TryEnter() const;
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			void Exit() const;

		public: // compatible with IntrinsicLock
			/// <summary>
			/// Acquires an exclusive lock.
			/// </summary>
			void Lock() const {
				return this->Enter();
			}
			/// <summary>
			/// Attempts to acquire an exclusive lock.
			/// </summary>
			bool TryLock() const {
				return this->TryEnter();
			}
			/// <summary>
			/// Releases an exclusive lock.
			/// </summary>
			void UnLock() const {
				return this->Exit();
			}
		};
	}
}
