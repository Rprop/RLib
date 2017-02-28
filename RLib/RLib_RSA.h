/********************************************************************
	Created:	2016/03/01  23:12
	Filename: 	RLib_RSA.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_CryptographyBase.h"

#if !(defined _USE_RSA) && !(defined _DISABLE_RSA)
#define _USE_RSA

namespace System
{
	namespace Security
	{
		namespace Cryptography
		{
			/// <summary>
			/// A suite of routines for performing RSA public-key computations.
			/// http://www.ohdave.com/rsa/RSA.js
			/// </summary>
			class RLIB_THREAD_SAFE RLIB_API RSAKeyPair
			{
			private:
				void *_handle;
			public:
				/// <summary>
				/// RSAKeyPair initiation
				/// </summary>
				/// <param name="encryptionExponent">public exponent</param>
				/// <param name="decryptionExponent">private exponent</param>
				/// <param name="modulus">public modulus</param>
				RSAKeyPair(const char *encryptionExponent, const char *decryptionExponent, const char *modulus);
				~RSAKeyPair();
				RLIB_DECLARE_DYNCREATE;
			public:
				/// <summary>
				/// Do an RSA public key operation.
				/// The input and output buffers must be large enough(eg. 128 bytes if RSA - 1024 is used).
				/// This function does NOT take care of message padding.
				/// Also, be sure to set input[0] = 0 or assure that input is smaller than N
				/// </summary>
				/// <param name="input">input buffer</param>
				/// <param name="output">output buffer</param>
				bool Encrypt(const void *input, void *output);
				/// <summary>
				/// Do an RSA public key operation.
				/// The temp and output buffers must be large enough(eg. 128 bytes if RSA - 1024 is used).
				/// This routine first pads the plaintext text to the same length as the encryption key for proper encryption.
				/// </summary>
				/// <param name="input">input buffer</param>
				/// <param name="size">size of input buffer</param>
				/// <param name="temp">temp buffer used to save padded data</param>
				/// <param name="output">output buffer</param>
				bool EncryptJS(const void *input, size_t size, void *temp, void *output);
				/// <summary>
				/// size(N) in chars
				/// </summary>
				size_t GetKeySize();
				RLIB_PROPERTY_GET(size_t KeySize, GetKeySize);
			};
		}
	}
}

#endif