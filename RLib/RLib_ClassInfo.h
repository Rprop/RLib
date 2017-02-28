/********************************************************************
	Created:	2016/08/08  22:08
	Filename: 	RLib_ClassInfo.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_Utility.h"

namespace System
{
	/// <summary>
	/// The namespaces contain types that enable features such as serialization and deserialization, and versioning.
	/// </summary>
	namespace Runtime
	{
		template<class T> class ClassInfo
		{
		private:
			static constexpr LPCTSTR __name() {
				return RLIB_FUNCTION;
			}

		public:
			static void getName(_Out_ TCHAR classname[], _In_ intptr_t maxcharcount) {
				constexpr auto lpstart = Utility::compiler_tcschr(__name(), _T('<')) + 1;
				constexpr auto lpend   = Utility::compiler_tcsrchr(lpstart, _T('>'));

				auto nlen = RLIB_MIN(lpend - lpstart, maxcharcount - 1);
				memcpy(classname, lpstart, nlen * sizeof(TCHAR));
				classname[nlen] = _T('\0');
			}
			static constexpr intptr_t getNameLength() {
				return Utility::compiler_tcsrchr(__name(), _T('>')) - (Utility::compiler_tcschr(__name(), _T('<')) + 1);
			}
			static constexpr intptr_t getSize() {
				return sizeof(T);
			}
			static constexpr intptr_t getAlignSize() {
				return Utility::round_up(sizeof(T), __alignof(T));
			}
		};
	}
}

