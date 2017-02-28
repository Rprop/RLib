/********************************************************************
	Created:	2015/06/14  9:16
	Filename: 	RLib_HttpUtility.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HTTP_UTILITY
#define _USE_HTTP_UTILITY
#include "RLib_String.h"

namespace System
{
	namespace Net
	{
		/// <summary>
		/// Provides methods for encoding and decoding URLs when processing Web requests
		/// </summary>
		class RLIB_API HttpUtility
		{
		public:
			/// <summary>
			/// Encodes a URL string using the specified encoding codepage
			/// </summary>
			static String UrlEncode(const String &str, Text::Encoding codepage = Text::UTF8Encoding);
			/// <summary>
			/// Converts a URL-encoded string into a decoded string, using the specified encoding codepage
			/// </summary>
			static String UrlDecode(const String &str, Text::Encoding codepage = Text::UTF8Encoding);
			/// <summary>
			/// Converts a string to an HTML-encoded string using numeric character reference (NCR) and partial character entity reference
			/// </summary>
			static String HTMLEncode(const String &str);
			/// <summary>
			/// Converts a string that has been HTML-encoded for HTTP transmission into a decoded string
			/// Function does not process all character entity reference, see http://dev.w3.org/html5/html-author/charref
			/// </summary>
			static String HTMLDecode(const String &str);
			/// <summary>
			/// Converts a string that has been unicode-encoded(\uhhhh) for HTTP transmission into a decoded string
			/// Function does not process \uhhhhhhhh which is exactly 8 hex digits
			/// </summary>
			static String UnicodeEscapesDecode(const String &str);

		public:
			static bool IsUrlSafeChar(TCHAR ch);
		};
	}
}

#endif // !_USE_HTTP_UTILITY
