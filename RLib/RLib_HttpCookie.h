/********************************************************************
	Created:	2016/06/30  22:39
	Filename: 	RLib_HttpCookie.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_String.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		/// <summary>
		/// Http Cookies Management
		/// </summary>
		class RLIB_API HttpCookie
		{
		public:
			static String GetValue(_In_ const String &cookie, _In_ const String name);
			static void Remove(_Inout_ String &cookie, _In_ const String name);
			static void EnumAll(_In_ const char *headers, _Inout_ String &cookie);
		};
	}
}