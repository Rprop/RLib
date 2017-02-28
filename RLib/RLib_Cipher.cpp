/********************************************************************
	Created:	2016/02/02  16:31
	Filename: 	RLib_Cipher.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Cipher.h>
#include "support/polarssl/library/polarssl/cipher.h"
using namespace System::Security::Cryptography;

//-------------------------------------------------------------------------

#ifdef _USE_CIPHER

void BlockCipherPadding::Add(IN OUT void *lpdata, intptr_t data_len, intptr_t output_len, 
							 PaddingMode mode /* = PaddingMode::PKCS7 */)
{
	assert(static_cast<int>(POLARSSL_PADDING_PKCS7) == static_cast<int>(PaddingMode::PKCS7));
	assert(static_cast<int>(POLARSSL_PADDING_NONE) == static_cast<int>(PaddingMode::NONE));

	struct {
		cipher_info_t    info;
		cipher_context_t context;
	} u;
	u.info.mode           = POLARSSL_MODE_CBC;
	u.context.cipher_info = &u.info;
	auto ret = cipher_set_padding_mode(&u.context, static_cast<cipher_padding_t>(mode));
	assert(ret == 0);
	UNREFERENCED_PARAMETER(ret);
	u.context.add_padding(static_cast<unsigned char *>(lpdata), 
						  static_cast<size_t>(output_len), 
						  static_cast<size_t>(data_len));
}

//-------------------------------------------------------------------------

int BlockCipherPadding::Get(IN const void *input, intptr_t input_len,
							PaddingMode mode /* = PaddingMode::PKCS7 */)
{
	assert(static_cast<int>(POLARSSL_PADDING_PKCS7) == static_cast<int>(PaddingMode::PKCS7));
	assert(static_cast<int>(POLARSSL_PADDING_NONE) == static_cast<int>(PaddingMode::NONE));

	struct {
		cipher_info_t    info;
		cipher_context_t context;
	} u;
	u.info.mode           = POLARSSL_MODE_CBC;
	u.context.cipher_info = &u.info;
	auto ret = cipher_set_padding_mode(&u.context, static_cast<cipher_padding_t>(mode));
	assert(ret == 0);
	UNREFERENCED_PARAMETER(ret);

	auto lpdata = static_cast<unsigned char *>(const_cast<void *>(input));
	if (u.context.get_padding(lpdata, static_cast<size_t>(input_len), &u.context.unprocessed_len) == 0) {
		return static_cast<int>(u.context.unprocessed_len);
	} //if

	return -1;
}

#endif // _USE_CIPHER
