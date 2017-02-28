/********************************************************************
	Created:	2012/06/06  21:51
	Filename: 	RLib_Base64.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HEX
#define _USE_HEX
#include "RLib_CryptographyBase.h"
#include "RLib_String.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			class RLIB_API RLIB_THREAD_SAFE Hex : public CryptographyBase
			{
			public:
				/// <summary>
				/// Converts a hex to base64 string
				/// </summary>
				static String hex2b64(String h);
				/// <summary>
				/// Converts a base64 string to hex
				/// </summary>
				static String b64tohex(String s);

			public:
				/// <summary>
				/// Encodes a serial object by hexadecimal encoding into null-terminated string
				/// </summary>
				/// <returns>chars written, = length of lpdata</returns>
				static intptr_t str_encode(LPTSTR lpdata, intptr_t chars_count, const void *lpsrc, intptr_t bytes_size);
				/// <summary>
				/// Decodes a string encoded by hexadecimal encoding into hex
				/// </summary>
				/// <returns>bytes written</returns>
				static intptr_t str_decode(void *lpout, intptr_t bytes_size, LPCTSTR lpdata, intptr_t chars_count);

			public:
				/// <summary>
				/// Encodes a serial object by hexadecimal encoding
				/// </summary>
				/// <returns>bytes written, include '\0'</returns>
				static int hexencode(unsigned char *buf, int buf_size, const void *src, int src_size);
				/// <summary>
				/// Decodes a string encoded by hexadecimal encoding
				/// </summary>
				/// <returns>bytes written, include '\0' or not</returns>
				static int hexdecode(unsigned char *buf, int buf_size, const void *src, int src_size);
			};
		}
	}
}

#endif /* _USE_HEX */
