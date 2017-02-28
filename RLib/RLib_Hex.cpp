/********************************************************************
	Created:	2012/06/06  21:52
	Filename: 	RLib_Base64.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Hex.h"
#include "RLib_GlobalizeString.h"
#include "RLib_Fundamental.h"
using namespace System::Security::Cryptography;

//-------------------------------------------------------------------------

const extern char b64map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const static char b64pad[] = "=";

//-------------------------------------------------------------------------

String Hex::hex2b64(String h)
{
	String b64;
	auto lpout = new IO::BufferedStream(256);
	if (lpout != nullptr) {
		int i;
		int c;
		char t[2];

		for (i = 0; i + 3 <= h.Length; i += 3) {
			c    = Int32::TryParse(h.Substring(i, 3), 16);
			t[0] = b64map[c >> 6];
			t[1] = b64map[c & 63];
			lpout->Write(t, 2);
		}
		if ((i + 1) == h.Length) {
			c    = Int32::TryParse(h.Substring(i, 1), 16);
			t[0] = b64map[c << 2];
			lpout->Write(&t[0], 1);
		} else if ((i + 2) == h.Length) {
			c    = Int32::TryParse(h.Substring(i, 2), 16);
			t[0] = b64map[c >> 2];
			t[1] = b64map[(c & 3) << 4];
			lpout->Write(t, 2);
		}
		while ((lpout->Length & 3) > 0) lpout->Write(b64pad, 1);

		b64.copy(reinterpret_cast<LPCSTR>(lpout->ObjectData), lpout->Length);
		delete lpout;
	} //if

	return b64;
}

//-------------------------------------------------------------------------

String Hex::b64tohex(String s_)
{
	String b64;
	auto lpout = new IO::BufferedStream(RLIB_DEFAULT_LENGTH);
	if (lpout != nullptr) {
		GlobalizeString u_s(s_);
		auto s_length = u_s.sizeofGBK();
		LPCSTR s      = u_s.toGBK(), p;

		char t[2];
		intptr_t i, v;
		int k = 0; // b64 state, 0-3
		intptr_t slop = 0;
		for (i = 0; i < s_length; ++i) {
			if (s[i] == b64pad[0]) break;

			p = strchr(b64map, s[i]);
			if (p == nullptr) continue;

			v = p - b64map;

			if (k == 0) {
				t[0] = static_cast<char>(v >> 2);
				lpout->Write(&t[0], 1);
				slop = v & 3;
				k    = 1;
			} else if (k == 1) {
				t[0] = static_cast<char>((slop << 2) | (v >> 4));
				lpout->Write(&t[0], 1);
				slop = v & 0xf;
				k    = 2;
			} else if (k == 2) {
				t[0] = static_cast<char>(slop);
				t[1] = static_cast<char>(v >> 2);
				lpout->Write(t, 2);
				slop = v & 3;
				k    = 3;
			} else {
				t[0] = static_cast<char>((slop << 2) | (v >> 4));
				t[1] = static_cast<char>(v & 0xf);
				lpout->Write(t, 2);
				k    = 0;
			} //if
		}
		if (k == 1) {
			t[0] = static_cast<char>(slop << 2);
			lpout->Write(&t[0], 1);
		} //if

		b64.copy(reinterpret_cast<LPCSTR>(lpout->GetObjectData()), lpout->Length);
		delete lpout;
	} //if
	return b64;
}

//-------------------------------------------------------------------------

int Hex::hexencode(unsigned char *buf, int buf_size, const void *src,
	int src_size)
{
	const unsigned char *rp = reinterpret_cast<const unsigned char *>(src);
	const unsigned char *ep = rp + src_size;
	src_size = buf_size;
	for (; rp < ep; ++rp)
	{
		int num = *rp >> 4;
		if (num < 10) {
			*(buf++) = static_cast<unsigned char>('0' + num);
		} else {
			*(buf++) = static_cast<unsigned char>('a' + num - 10);
		} //if
		--buf_size;
		if (buf_size <= 1) {
			break;
		} //if

		num = *rp & 0x0f;
		if (num < 10) {
			*(buf++) = static_cast<unsigned char>('0' + num);
		} else {
			*(buf++) = static_cast<unsigned char>('a' + num - 10);
		} //if
		--buf_size;
		if (buf_size <= 1) {
			break;
		} //if
	}
	*buf = '\0';
	return src_size - (++buf_size);
}

//-------------------------------------------------------------------------

int Hex::hexdecode(unsigned char *buf, int buf_size, const void *src_, int src_size)
{
	int buf_size_ori = buf_size;

	const unsigned char *src = reinterpret_cast<const unsigned char *>(src_);
	for (int i = 0; i < src_size; i += 2)
	{
		while (src[i] >= '\0' && src[i] <= ' ') ++i;
		
		int num = 0;
		int c = src[i];
		if (c == '\0') break;
		if (c >= '0' && c <= '9') {
			num = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			num = c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			num = c - 'A' + 10;
		} else if (c == '\0') {
			break;
		} //if
		c = src[i + 1];
		if (c >= '0' && c <= '9') {
			num = num * 0x10 + c - '0';
		} else if (c >= 'a' && c <= 'f') {
			num = num * 0x10 + c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			num = num * 0x10 + c - 'A' + 10;
		} else if (c == '\0') {
			break;
		} //if
		*(buf++) = static_cast<unsigned char>(num);

		--buf_size;
		if (buf_size <= 1) {
			break;
		} //if
	}
	if (buf_size >= 1) {
		*buf = '\0';
		++buf_size;
	} //if
	return buf_size_ori - buf_size;
}

//-------------------------------------------------------------------------

intptr_t Hex::str_encode(LPTSTR lpdata, intptr_t chars_count, const void *lpsrc, intptr_t bytes_size)
{
	assert(chars_count >= (bytes_size * 2 + 1));
	const unsigned char *rp = reinterpret_cast<const unsigned char *>(lpsrc);
	
	intptr_t i = 0, j = 0;
	for (; i < bytes_size && j < chars_count; ++i, j += 2) {
		if ((rp[i] & 0xFF) > 0x0Fu) {
			_itot_s(rp[i] & 0xFF, &lpdata[j], 2 + 1, 16);
		} else {
			lpdata[j] = _T('0');
			_itot_s(rp[i] & 0xFF, &lpdata[j + 1], 1 + 1, 16);
		} //if
	}
	return j;
}

//-------------------------------------------------------------------------

intptr_t Hex::str_decode(void *lpout, intptr_t bytes_size, LPCTSTR lpdata, intptr_t chars_count)
{
	assert((chars_count % 2 == 0) && bytes_size >= (chars_count / 2));
	unsigned char *rp = reinterpret_cast<unsigned char *>(lpout);

	TCHAR k[3];
	intptr_t i = 0, j = 0;
	k[2]  = _T('\0');
	for (; i < bytes_size && j < chars_count; ++i, j += 2) {
		k[0] = lpdata[j];
		k[1] = lpdata[j + 1];
		rp[i] = static_cast<unsigned char>(_tcstol(k, nullptr, 16));
	}
	return i;
}