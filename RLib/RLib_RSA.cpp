/********************************************************************
	Created:	2016/03/01  23:12
	Filename: 	RLib_RSA.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_RSA.h"
#include "support/polarssl/library/polarssl/rsa.h"
#include "support/polarssl/library/polarssl/entropy.h"
#include "support/polarssl/library/polarssl/ctr_drbg.h"
using namespace System::Security::Cryptography;

#ifdef _USE_RSA
typedef struct
{
	entropy_context  entropy;
	ctr_drbg_context ctr_drbg;
} RSA_RNG;
typedef struct {
	rsa_context rsa;
	RSA_RNG    *rng;
} RSA, *LPRSA;
#define this_handle static_cast<LPRSA>(this->_handle)

//-------------------------------------------------------------------------

RSAKeyPair::RSAKeyPair(const char *encryptionExponent, const char *decryptionExponent,
					   const char *modulus)
{
	this->_handle    = RLIB_GlobalAllocObj(RSA, 1);
	this_handle->rng = nullptr;
	rsa_init(&this_handle->rsa, RSA_PKCS_V15, NULL);
	int r;
	r = mpi_read_string(&this_handle->rsa.N, 16, modulus);
	assert(r == 0);
	this_handle->rsa.len = mpi_size(&this_handle->rsa.N);

	if (encryptionExponent) {
		r = mpi_read_string(&this_handle->rsa.E, 16, encryptionExponent);
		assert(r == 0);
#if _DEBUG
		assert(rsa_check_pubkey(&this_handle->rsa) == 0);
#endif // _DEBUG

	} //if
	if (decryptionExponent) {
		r = mpi_read_string(&this_handle->rsa.D, 16, decryptionExponent);
		assert(r == 0);
#if _DEBUG
		assert(rsa_check_privkey(&this_handle->rsa) == 0);
#endif // _DEBUG
	} //if
}

//-------------------------------------------------------------------------

RSAKeyPair::~RSAKeyPair()
{
	if (this_handle->rng != nullptr) {
		entropy_free(&this_handle->rng->entropy);
		ctr_drbg_free(&this_handle->rng->ctr_drbg);
		RLIB_GlobalCollect(this_handle->rng);
	} //if
	rsa_free(&this_handle->rsa);
	RLIB_GlobalCollect(this_handle);
}

//-------------------------------------------------------------------------

bool RSAKeyPair::Encrypt(const void *input, void *output)
{
	return rsa_public(&this_handle->rsa, static_cast<const unsigned char *>(input), static_cast<unsigned char *>(output)) == 0;
}

//-------------------------------------------------------------------------

bool RSAKeyPair::EncryptJS(const void *input, size_t size, void *temp, void *output)
{
	/*
	* The various padding schemes employed by this routine, and as presented to
	* RSA for encryption, are shown below.  Note that the RSA encryption done
	* herein reverses the byte order as encryption is done:
	*
	*      Plaintext In
	*      ------------
	*      d5 d4 d3 d2 d1 d0
	*
	*      OHDave
	*      ------
	*      d5 d4 d3 d2 d1 d0 00 00 00 /.../ 00 00 00 00 00 00 00 00
	*
	*      NoPadding
	*      ---------
	*      00 00 00 00 00 00 00 00 00 /.../ 00 00 d0 d1 d2 d3 d4 d5
	*
	*      PKCS1Padding
	*      ------------
	*      d0 d1 d2 d3 d4 d5 00 p0 p1 /.../ p2 p3 p4 p5 p6 p7 02 00
	*                            \------------  ------------/
	*                                         \/
	*                             Minimum 8 bytes pad length
	*/
	assert(size <= this->KeySize); // the plaintext should be broken up into chunks, which is not supported now
	auto lpinput = static_cast<const unsigned char *>(input);
	auto lptemp  = static_cast<unsigned char *>(temp) + this->KeySize;
	memset(temp, 0, this->KeySize - size);
	while (size > 0) {
		// padding
		(--lptemp)[0] = (lpinput++)[0];
		--size;
	}
	return this->Encrypt(temp, output);
}

//-------------------------------------------------------------------------

size_t RSAKeyPair::GetKeySize(){
	return this_handle->rsa.len;
}

#endif // _USE_RSA
