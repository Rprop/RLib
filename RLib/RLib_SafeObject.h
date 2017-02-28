/********************************************************************
	Created:	2015/02/01  21:51
	Filename: 	RLib_SafeObject.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_Object.h"
#include "RLib_Threading.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		template<class R, class LOCKER> class SafeObjectHelper
		{
		private:
			mutable R      *lpobj;
			mutable LOCKER *lplocker;

		public:
			SafeObjectHelper(const SafeObjectHelper &src) = delete;
			SafeObjectHelper &operator=(const SafeObjectHelper &src) = delete;
			RLIB_INLINE SafeObjectHelper(const SafeObjectHelper &&src) {
				trace(!"never get here!");
			}
			RLIB_INLINE SafeObjectHelper(R *obj, LOCKER *locker) {
				this->lpobj = obj;
				this->lplocker = locker;
				this->lplocker->Lock();
			}
			RLIB_INLINE ~SafeObjectHelper() {
				if (this->lplocker != nullptr) {
					this->lplocker->UnLock();
					this->lplocker = nullptr;
				} //if
			}
			RLIB_INLINE R *operator -> () {
				return this->lpobj;
			}
		};
		/// <summary>
		/// encapsulates a object whose access is guaranteed to be thread-safe
		/// </summary>
		template<class R, class LOCKER = Threading::AtomicLock>
		class RLIB_THREAD_SAFE SafeObject : private Object<R>, private LOCKER
		{
		private:
			SafeObject(const class SafeObject &) = delete;
			SafeObject& operator=(const class SafeObject &) = delete;
			R *SuppressFinalize();

		public:
			SafeObject() : Object<R>() {}
			SafeObject(R *ptr) : Object<R>(ptr) {}

		public:
			SafeObject& operator = (R *ptr) {
				assert(this->ObjectPtr == nullptr);
				this->ObjectPtr = ptr;
				return *this;
			}
			void Finalize() {
				if (this->ObjectPtr != nullptr) {
					delete this->ObjectPtr;
					this->ObjectPtr = nullptr;
				} //if
			}

		public:
			SafeObjectHelper<R, LOCKER> operator -> () {
				return SafeObjectHelper<R, LOCKER>(this->ObjectPtr, static_cast<LOCKER *>(this));
			}
// 			operator R *() {
// 				this->SyncRoot.Lock();
// 				return this->ObjectPtr;
// 			}
			/// <summary>
			/// Gets an object that can be used to synchronize access to the BitArray
			/// </summary>
			LOCKER &GetSyncRoot() { return *this; }
			/// <summary>
			/// Gets an object that can be used to synchronize access to the BitArray
			/// </summary>
			RLIB_PROPERTY_GET(LOCKER &SyncRoot, GetSyncRoot);

		public:
			/// <summary>
			/// Gets the Type of the current instance
			/// </summary>
			typedef R Type;
			/// <summary>
			/// Gets the Type of the current locker
			/// </summary>
			typedef LOCKER SyncType;
		};
		/// <summary>
		/// encapsulates a object so as to make it synchronizable
		/// </summary>
		template<class R, class LOCKER = Threading::Monitor>
		class RLIB_THREAD_SAFE Synchronizable : public R, public LOCKER
		{
		public:
			using R::R;
			using LOCKER::LOCKER;		
			Synchronizable() = default;
			Synchronizable(R &&r) : R(r) {}
			Synchronizable(const R &r) : R(r) {}
			Synchronizable(class Synchronizable &&) = default;
			Synchronizable(const class Synchronizable &) = default;
			RLIB_DECLARE_DYNCREATE;

		public:
			Synchronizable& operator = (class Synchronizable &&) = default;
			Synchronizable& operator = (const class Synchronizable &) = default;

		public:
			/// <summary>
			/// Gets an object that can be used to synchronize access to the BitArray
			/// </summary>
			LOCKER &GetSyncRoot() { return *this; }
			/// <summary>
			/// Gets an object that can be used to synchronize access to the BitArray
			/// </summary>
			RLIB_PROPERTY_GET(LOCKER &SyncRoot, GetSyncRoot);

		public:
			/// <summary>
			/// Gets the Type of the current instance
			/// </summary>
			typedef R Type;
			/// <summary>
			/// Gets the Type of the current locker
			/// </summary>
			typedef LOCKER SyncType;
		};
	}
}