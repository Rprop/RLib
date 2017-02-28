/********************************************************************
	Created:	2016/02/02  14:55
	Filename: 	RLib_AES.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AES.h"
#include "support/polarssl/library/polarssl/aes.h"
using namespace System::Security::Cryptography;

#ifdef _USE_AES

//-------------------------------------------------------------------------

AES::AES(const void *lpkey, intptr_t key_bits)
{
	this->_aes_handle = RLIB_GlobalAllocObj(aes_context, 1);
	aes_init(static_cast<aes_context *>(this->_aes_handle));
	int r;
	r = aes_setkey_enc(static_cast<aes_context *>(this->_aes_handle),
					   static_cast<const unsigned char *>(lpkey), 
					   static_cast<unsigned int>(key_bits));
	assert(r == 0);
	r = aes_setkey_dec(static_cast<aes_context *>(this->_aes_handle),
					   static_cast<const unsigned char *>(lpkey),
					   static_cast<unsigned int>(key_bits));
	assert(r == 0);
}

//-------------------------------------------------------------------------

AES::~AES()
{
	aes_free(static_cast<aes_context *>(this->_aes_handle));
	RLIB_GlobalCollect(this->_aes_handle);
}

//-------------------------------------------------------------------------

bool AES::Encrypt(const void *lpdata, OUT void *lpout)
{
	return aes_crypt_ecb(static_cast<aes_context *>(this->_aes_handle), AES_ENCRYPT,
						 static_cast<const unsigned char *>(lpdata),
						 static_cast<unsigned char *>(lpout)) == 0;
}

//-------------------------------------------------------------------------

bool AES::Decrypt(const void *lpdata, OUT void *lpout)
{
	return aes_crypt_ecb(static_cast<aes_context *>(this->_aes_handle), AES_DECRYPT,
						 static_cast<const unsigned char *>(lpdata),
						 static_cast<unsigned char *>(lpout)) == 0;
}

//-------------------------------------------------------------------------

bool AES::encrypt_cbc(const void *lpdata, intptr_t datasize,
					  const void *lpkey, intptr_t key_bits, const void *lpiv, void *lpout)
{
	aes_context aes_enc;
	aes_init(&aes_enc);
	int r = aes_setkey_enc(&aes_enc, 
						   static_cast<const unsigned char *>(lpkey), 
						   static_cast<unsigned int>(key_bits));
	if (r == 0) {
		unsigned char iv[16];
		::memcpy(iv, lpiv, 16);

		r = aes_crypt_cbc(&aes_enc, AES_ENCRYPT, 
						  static_cast<size_t>(datasize),
						  iv,
						  static_cast<const unsigned char *>(lpdata),
						  static_cast<unsigned char *>(lpout));
	} //if

	aes_free(&aes_enc);
	return r == 0;
}

//-------------------------------------------------------------------------

bool AES::decrypt_cbc(const void *lpdata, intptr_t datasize,
					  const void *lpkey, intptr_t key_bits, const void *lpiv, void *lpout)
{
	aes_context aes_enc;
	aes_init(&aes_enc);
	int r = aes_setkey_dec(&aes_enc, 
						   static_cast<const unsigned char *>(lpkey), 
						   static_cast<unsigned int>(key_bits));
	if (r == 0) {
		unsigned char iv[16];
		::memcpy(iv, lpiv, 16);

		r = aes_crypt_cbc(&aes_enc, AES_DECRYPT,
						  static_cast<size_t>(datasize),
						  iv,
						  static_cast<const unsigned char *>(lpdata),
						  static_cast<unsigned char *>(lpout));
	} //if

	aes_free(&aes_enc);
	return r == 0;
}

#endif // _USE_AES
