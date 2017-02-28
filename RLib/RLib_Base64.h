/********************************************************************
	Created:	2012/06/06  21:51
	Filename: 	RLib_Base64.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_CryptographyBase.h"
#if !(defined _USE_BASE64) && !(defined _DISABLE_BASE64)
#define _USE_BASE64
#include "RLib_String.h"
#include "RLib_Object.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			class RLIB_API RLIB_THREAD_SAFE Base64 : public CryptographyBase
			{
			public:
				/**
				* \brief          Encode a buffer into base64 format
				*
				* \param dst      destination buffer
				* \param dlen     size of the buffer
				* \param src      source buffer
				* \param slen     amount of data to be encoded
				*
				* \return         0 if successful, or -1 if BUFFER_TOO_SMALL.
				*                 *dlen is always updated to reflect the amount
				*                 of data that has (or would have) been written.
				*
				* \note           Call this function with *dlen = 0 to obtain the
				*                 required buffer size in *dlen
				*/
				static int encode(unsigned char *dst, size_t *dlen,
								  const unsigned char *src, size_t slen);
				/**
				* \brief          Decode a base64-formatted buffer
				*
				* \param dst      destination buffer (can be NULL for checking size)
				* \param dlen     size of the buffer
				* \param src      source buffer
				* \param slen     amount of data to be decoded
				*
				* \return         0 if successful, or -1 BUFFER_TOO_SMALL, or -2 if the input data
				*                 is not correct. *dlen is always updated to reflect the amount
				*                 of data that has (or would have) been written.
				*
				* \note           Call this function with *dst = NULL or *dlen = 0 to obtain
				*                 the required buffer size in *dlen
				*/
				static int decode(OPTIONAL unsigned char *dst, size_t *dlen,
								  const unsigned char *src, size_t slen);

			public:
				/// <summary>
				/// Converts the value of an array of 8-bit unsigned integers 
				/// to its equivalent string representation that is encoded with base-64 digits
				/// </summary>
				static String ToBase64String(const void *lpdata, intptr_t bytes);
				/// <summary>
				/// Converts the specified string, which encodes binary data as base-64 digits,
				/// to an equivalent 8-bit unsigned integer array
				/// </summary>
				static ManagedMemoryBlock<unsigned char> FromBase64String(const String &base64String,
																		  OUT intptr_t *lpbytes = nullptr);
			};
		}
	}
}

#endif /* _USE_BASE64 */
