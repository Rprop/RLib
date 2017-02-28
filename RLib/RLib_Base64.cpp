/********************************************************************
	Created:	2012/06/06  21:52
	Filename: 	RLib_Base64.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Base64.h"
#include "RLib_Fundamental.h"
#include "RLib_Object.h"
#include "RLib_GlobalizeString.h"
#include "support/polarssl/library/polarssl/base64.h"
using namespace System::Security::Cryptography;

#ifdef _USE_BASE64

//-------------------------------------------------------------------------

int Base64::encode(unsigned char *dst, size_t *dlen,
						const unsigned char *src, size_t slen)
{
	int ret = base64_encode(dst, dlen, src, slen);
	switch (ret)
	{
	case POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL:
		{
			return -1;
		}
	default:
		{
			return 0;
		}
	}
}

//-------------------------------------------------------------------------

int Base64::decode(OPTIONAL unsigned char *dst, size_t *dlen,
						const unsigned char *src, size_t slen)
{
	int ret = base64_decode(dst, dlen, src, slen);
	switch (ret)
	{
	case POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL:
		{
			return -1;
		}
	case POLARSSL_ERR_BASE64_INVALID_CHARACTER:
		{
			return -2;
		}
	default:
		{
			return 0;
		}
	}
}

//-------------------------------------------------------------------------

String Base64::ToBase64String(const void *lpdata, intptr_t bytes)
{
	size_t out_size = 0;

	// get the required size
	intptr_t result = Base64::encode(nullptr, &out_size,
									 static_cast<const unsigned char *>(lpdata),
									 static_cast<size_t>(bytes));
	assert(result == -1);

	ManagedMemoryBlock<unsigned char> data_out(out_size);
	if (data_out.IsNull()) {
		return Nothing;
	} //if

	result = encode(data_out, &out_size, 
					static_cast<const unsigned char *>(lpdata), 
					static_cast<size_t>(bytes));
	assert(result == 0);

	result = static_cast<intptr_t>(out_size / sizeof(char)); // length
	return String(result).copy(data_out.ToAny<char>(), result);
}

//-------------------------------------------------------------------------

ManagedMemoryBlock<unsigned char> Base64::FromBase64String(const String &base64String, 
														   OUT intptr_t *lpbytes /* = nullptr */)
{
	GlobalizeString globalizeString(base64String);
	
	auto src_ptr   = reinterpret_cast<unsigned char *>(globalizeString.toGBK()); 
	auto src_bytes = globalizeString.sizeofGBK();
	
	size_t nbytes = 0UL;
	Base64::decode(nullptr, &nbytes, src_ptr, static_cast<size_t>(src_bytes));
	if (nbytes > 0) {
		ManagedMemoryBlock<unsigned char> buffer(nbytes);
		if (!buffer.IsNull()) {
			if (Base64::decode(buffer, &nbytes, src_ptr, static_cast<size_t>(src_bytes)) == 0) {
				if (lpbytes) *lpbytes = static_cast<intptr_t>(nbytes);
				return buffer;
			} //if
		} //if
	} //if

	if (lpbytes) *lpbytes = 0;
	return nullptr;
}

#endif // _USE_BASE64