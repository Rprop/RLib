/********************************************************************
	Created:	2016/02/02  14:53
	Filename: 	RLib_AES.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_CryptographyBase.h"

#if !(defined _USE_AES) && !(defined _DISABLE_AES)
#define _USE_AES

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			/// <summary>
			/// AES encryption algorithm
			/// </summary>
			class RLIB_THREAD_SAFE RLIB_API AES
			{
			private:
				void *_aes_handle;

			public:
				/// <summary>
				/// AES-ECB initiation
				/// </summary>
				/// <param name="lpkey">encryption/decryption key</param>
				/// <param name="key_bits">must be 128, 192 or 256 bits</param>
				AES(const void *lpkey, intptr_t key_bits);
				~AES();
				RLIB_DECLARE_DYNCREATE;

			public:
				/// <summary>
				/// AES-ECB block encryption
				/// </summary>
				/// <param name="lpdata">buffer holding the input data, 16 bytes</param>
				/// <param name="lpout">output buffer, 16 bytes</param>
				/// <returns>true if successful</returns>
				bool Encrypt(const void *lpdata, OUT void *lpout);
				/// <summary>
				/// AES-ECB block decryption
				/// </summary>
				/// <param name="lpdata">buffer holding the input data, 16 bytes</param>
				/// <param name="lpout">output buffer, 16 bytes</param>
				/// <returns>true if successful</returns>
				bool Decrypt(const void *lpdata, OUT void *lpout);

			public:
				/// <summary>
				/// AES-CBC buffer encryption
				/// </summary>
				/// <param name="lpdata">buffer holding the input data</param>
				/// <param name="datasize">should be a multiple of the block size (16 bytes)</param>
				/// <param name="lpkey">encryption key</param>
				/// <param name="key_bits">must be 128, 192 or 256 bits</param>
				/// <param name="lpiv">initialization vector, 16 bytes</param>
				/// <param name="lpout">output buffer, [datasize] bytes in total</param>
				/// <returns>true if successful</returns>
				static bool encrypt_cbc(const void *lpdata, intptr_t datasize, 
										const void *lpkey, intptr_t key_bits, 
										const void *lpiv, OUT void *lpout);
				/// <summary>
				/// AES-CBC buffer decryption
				/// </summary>
				/// <param name="lpdata">buffer holding the input data</param>
				/// <param name="datasize">should be a multiple of the block size (16 bytes)</param>
				/// <param name="lpkey">decryption key</param>
				/// <param name="key_bits">must be 128, 192 or 256 bits</param>
				/// <param name="lpiv">initialization vector, 16 bytes</param>
				/// <param name="lpout">output buffer, [datasize] bytes in total</param>
				/// <returns>true if successful</returns>
				static bool decrypt_cbc(const void *lpdata, intptr_t datasize,
										const void *lpkey, intptr_t key_bits,
										const void *lpiv, OUT void *lpout);
			};
		}
	}
}

#endif