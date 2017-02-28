/********************************************************************
	Created:	2016/08/03  9:19
	Filename: 	RLib_StringInternal.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_AppBase.h"

namespace System
{
	struct InternalString
	{
		intptr_t refCount;
#pragma warning(push)
#pragma warning(disable:4200)
		TCHAR instanceString[0];
#pragma warning(pop)
	public:
		RLIB_FORCE_INLINE InternalString *tryInit() {
			assert(sizeof(*this) <= RLIB_ALIGNMENT);
			if (this) {
				this->refCount          = 0;
				this->instanceString[0] = _T('\0');
			} //if
			return this;
		}
		RLIB_FORCE_INLINE TCHAR *tryToStringPointer() {
			if (this) {
				assert(this->refCount >= 0);
				return this->instanceString;
			} //if
			return nullptr;
		}
		RLIB_FORCE_INLINE intptr_t increaseReference() {
			assert(this->refCount >= 0);
			return ++this->refCount;
		}
		RLIB_FORCE_INLINE intptr_t decreaseReference() {
			assert(this->refCount >= 0);
			if (this->refCount == 0) {
				RLIB_GlobalCollect(this);
				return -1;
			} //if
			return --this->refCount;
		}
		template<typename char_t> static InternalString *fromStringPtr(const char_t *pstr) {
			return reinterpret_cast<InternalString *>(const_cast<char *>(
				reinterpret_cast<const char *>(pstr) - sizeof(intptr_t)));
		}
	};
}