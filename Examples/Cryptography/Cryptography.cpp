/********************************************************************
	Created:	2016/07/09  20:41
	Filename: 	Cryptography.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdio.h>

using namespace System::Security::Cryptography;

//-------------------------------------------------------------------------

int main()
{
	// base64
	string b64str  = Base64::ToBase64String("RLib", 4);
	auto plaintext = Base64::FromBase64String(b64str);
	if (plaintext) {
		printf("%s" RLIB_NEWLINEA, plaintext.ToAny<char>());
	} //if

	// hex
	auto hex_b64_encode = Hex::hex2b64(_R("D1E429"));
	auto hex_b64_decode = Hex::b64tohex(hex_b64_encode);

	// md5
	auto md5_encode = MD5::GetHashCode(_R("RLib"));
	printf("%s\r\n", GlobalizeString(md5_encode).toGBK());

	return 0;
}