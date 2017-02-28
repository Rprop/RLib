/********************************************************************
	Created:	2016/02/02  16:31
	Filename: 	RLib_Cipher.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_CryptographyBase.h"

#if !(defined _USE_CIPHER) && !(defined _DISABLE_CIPHER)
#define _USE_CIPHER

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			/// <summary>
			/// Padding mode, for cipher modes that use padding
			/// </summary>
			enum class PaddingMode {
				PKCS7 = 0,     /**< PKCS7 padding (default)        */
				ONE_AND_ZEROS, /**< ISO/IEC 7816-4 padding         */
				ZEROS_AND_LEN, /**< ANSI X.923 padding             */
				ZEROS,         /**< zero padding (not reversible!) */
				NONE,          /**< never pad (full blocks only)   */
			};
			/// <summary>
			/// Block cipher padders
			/// </summary>
			class RLIB_THREAD_SAFE RLIB_API BlockCipherPadding
			{
			public:
				/// <summary>
				/// Adds the pad bytes to the passed in block, returning the number of bytes added
				/// </summary>
				static void Add(IN OUT void *lpdata, intptr_t data_len, intptr_t output_len, PaddingMode mode = PaddingMode::PKCS7);
				/// <summary>
				/// Gets the number of pad bytes present in the block
				/// </summary>
				static int Get(IN const void *input, intptr_t input_len, PaddingMode mode = PaddingMode::PKCS7);
			};
		}
	}
}

#endif //_USE_CIPHER
