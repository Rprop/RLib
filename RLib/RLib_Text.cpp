/********************************************************************
Created:	2012/02/05  22:43
Filename: 	RLib_Text.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Text.h"
#include "RLib_Object.h"
#if (WINVER > _WIN32_WINNT_WIN7)
# include <stringapiset.h>
#else
# include <WinNls.h>
#endif
#include <limits.h>

//-------------------------------------------------------------------------

using namespace System::IO;
using namespace System::Text;

//  Bytes	    Encoding Form
// 	00 00 FE FF	UTF-32, big-endian
// 	FF FE 00 00	UTF-32, little-endian
// 	FE FF	    UTF-16, big-endian
// 	FF FE	    UTF-16, little-endian
// 	EF BB BF	UTF-8
const unsigned char UTF8_LEAD[]    = { 0xefU, 0xbbU, 0xbfU };
const unsigned char UTF16_LEAD[]   = { 0xffU, 0xfeU };
const unsigned char UTF16BE_LEAD[] = { 0xfeU, 0xffU };
const unsigned char UTF32_LEAD[]   = { 0xffU, 0xfeU, 0x00U, 0x00U };
const unsigned char UTF32BE_LEAD[] = { 0x00U, 0x00U, 0xfeU, 0xffU };

#define RLIB_MB_CONVERSION_FLAGS 0 // MB_ERR_INVALID_CHARS // error on invalid chars
#define RLIB_WC_CONVERSION_FLAGS 0

RLIB_ALIGN(1) typedef struct
{
    char High;
    char Low;
} UNICHAR, *LPUNICHAR;

//-------------------------------------------------------------------------

Encoding Encoder::DetectEncodingFromByteOrderMarks(const void *ptr,
												   OUT intptr_t *lpbytes /* = nullptr */)
{
	auto &p = *reinterpret_cast<const unsigned char **>(&ptr);

    if (RLIB_CMP_SHORT_BYTE(p, UTF8_LEAD)) {
        if (lpbytes) *lpbytes = sizeof(UTF8_LEAD);
 
        return UTF8Encoding;
	} //if
    
	if (RLIB_CMP_SHORT(p, UTF16_LEAD)) {
        if (lpbytes) *lpbytes = sizeof(UTF16_LEAD);
        
        return UTF16Encoding;
	} //if
    
	if (RLIB_CMP_SHORT(p, UTF16BE_LEAD)) {
        if (lpbytes) *lpbytes = sizeof(UTF16BE_LEAD);

        return UTF16BEEncoding;
    } //if

    if (lpbytes) *lpbytes = 0;

    // failed to detect
    return UnknownEncoding;
}

//-------------------------------------------------------------------------

bool Encoder::WriteByteOrderMarks(IO::Stream &output, Encoding codepage)
{
	if (codepage == UnknownEncoding){
		codepage = GetCurrentEncoding();
	} //if

	switch (codepage) 
	{
	case UTF8Encoding:
		output.Write(UTF8_LEAD, sizeof(UTF8_LEAD));
		return true;
	case UTF16Encoding:
		output.Write(UTF16_LEAD, sizeof(UTF16_LEAD));
		return true;
	case UTF16BEEncoding:
		output.Write(UTF16BE_LEAD, sizeof(UTF16BE_LEAD));
	case ASCIIEncoding:
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------

Encoding Encoder::GetCurrentEncoding()
{
#ifdef _UNICODE
	return UTF16Encoding;
#else 
	return ASCIIEncoding;
#endif // _UNICODE
}

//-------------------------------------------------------------------------

bool Encoder::IsTextUnicode(_In_ const void *lpv, _In_ intptr_t size)
{
	UNREFERENCED_PARAMETER(lpv);
	UNREFERENCED_PARAMETER(size);
	trace(!"not supported!");
	return false;
	/*
	// refer to https://msdn.microsoft.com/en-us/library/windows/desktop/dd318672(v=vs.85).aspx
	//int iResult = IS_TEXT_UNICODE_ODD_LENGTH | IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK;
	return ::IsTextUnicode(lpv, size, nullptr) != 0;
	*/
}

//-------------------------------------------------------------------------

bool Encoder::IsTextUtf8(_In_ const void *lpv, _In_ intptr_t size)
{
	int cchBuffer = MultiByteToWideChar(UTF8Encoding,           // convert from codepage
										MB_ERR_INVALID_CHARS,   // conversion flags
										static_cast<LPCCH>(lpv),// source data
										static_cast<int>(size), // total length of source data, in char's (= bytes)
										NULL,                   // unused - no conversion done in this step
										0                       // request size of destination buffer, in wchar_t's
										);
	return cchBuffer > 0;
}

//-------------------------------------------------------------------------

static BufferedStream *__convert_to_current_encoding(LPCSTR pdata, intptr_t size,
													 Encoding codepage)
{
	switch (codepage)
	{
	case ASCIIEncoding:
		{
#ifdef _UNICODE
			// ansi/gb2312 to utf-16
			return Encoder::ToWideChar(pdata, size, ASCIIEncoding);
#else 
			// ansi/gb2312 already, copy it
			auto output_stream = new BufferedStream(size);
			if (output_stream != nullptr) {
				output_stream->Write(pdata, size);
				output_stream->Position = 0;
			} //if
			return output_stream;
#endif // _UNICDOE
		}
	case UTF16Encoding:
		{
#ifdef _UNICODE
			// utf-16 already, copy it
			auto output_stream = new BufferedStream(size);
			if (output_stream != nullptr) {
				output_stream->Write(pdata, size);
				output_stream->Position = 0;
			} //if
			return output_stream;
#else 
			// utf-16 to ansi/gb2312
			return Encoder::WideCharTo(pdata, size, ASCIIEncoding);
#endif // _UNICDOE
		}
	//case UTF16BEEncoding:
	//case UTF8Encoding:
	default:
		break;
	}

	// to utf-16
	auto output_stream = Encoder::ToWideChar(pdata, size, codepage);
#ifndef _UNICODE
	// to ansi/gb2312
	if (output_stream != nullptr) {
		auto final_stream = Encoder::WideCharTo(codepage, *output_stream);
		delete output_stream;
		output_stream     = final_stream;
	} //if
#endif // _UNICODE
	return output_stream;
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::ToCurrentEncoding(LPCVOID lpdata, intptr_t size,
										   Encoding codepage, 
										   bool detectEncodingFromByteOrderMarks /* = true */)
{
	if (size <= 0) return nullptr;

	LPCSTR &pdata = reinterpret_cast<LPCSTR &>(lpdata);

	// check codepage
	if (codepage == UnknownEncoding)
	{
		if (size <= 3 || detectEncodingFromByteOrderMarks == false){
			goto __try_convert; // too few characters in the buffer for meaningful analysis or check disabled
		} //if

		intptr_t mark_size;
		codepage     = DetectEncodingFromByteOrderMarks(pdata, &mark_size);
		if (codepage == UnknownEncoding) {
			goto __try_convert; // BOM not found
		} //if

		pdata += mark_size;
		size  -= mark_size;
	} //if

	return __convert_to_current_encoding(pdata, size, codepage);

__try_convert: // could not identify encoding, try to convert
#ifdef _UNICODE
	auto output_stream = Encoder::ToWideChar(pdata, size,
											 Encoder::IsTextUtf8(pdata, static_cast<int>(size)) ? UTF8Encoding : ASCIIEncoding);
	if (output_stream == nullptr) {
		// assume utf-16
		output_stream = new BufferedStream(size);
		if (output_stream != nullptr) {
			output_stream->Write(pdata, size);
			output_stream->Position = 0;
		} //if
	} //if
#else 
	if (Encoder::IsTextUtf8(pdata, size)) {
		return __convert_to_current_encoding(pdata, size, UTF8Encoding);
	} //if
	#pragma todo("ignores Unicode")
	// assume ansi/gb2312
	auto output_stream = new BufferedStream(size);
	if (output_stream != nullptr){
		output_stream->Write(pdata, size);
		output_stream->Position = 0;
	} //if	
#endif // _UNICODE
	
	return output_stream;
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::ToCurrentEncoding(Encoding codepage, 
										   const IO::Stream &stream, intptr_t length /* = -1 */, 
										   bool detectEncodingFromByteOrderMarks /* = true */)
{
	StreamReader reader(&stream, length);
	return ToCurrentEncoding(reader.BufferedData, reader.Capacity, 
							 codepage,
							 detectEncodingFromByteOrderMarks);
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::ToWideChar(LPCVOID _pdata, intptr_t size, Encoding codepage)
{
    assert(size > 0);

	LPCSTR &pdata = reinterpret_cast<LPCSTR &>(_pdata);

	if (codepage == UTF8Encoding) {
		if (RLIB_CMP_SHORT_BYTE(pdata, UTF8_LEAD)) {
			pdata += sizeof(UTF8_LEAD);
			size  -= sizeof(UTF8_LEAD);
		} //if
	} else if (codepage == UTF16BEEncoding) {
		if (RLIB_CMP_SHORT(pdata, UTF16BE_LEAD)) {
			pdata += sizeof(UTF16BE_LEAD);
			size  -= sizeof(UTF16BE_LEAD);
		} //if

        auto lpUtf16 = new BufferedStream(size);
		if (lpUtf16 != nullptr) {	
			auto lpUtf16_le = reinterpret_cast<LPUNICHAR>(lpUtf16->ObjectData);
			auto lpUtf16_be = reinterpret_cast<LPUNICHAR>(const_cast<LPSTR>(pdata));

			assert((size % sizeof(wchar_t)) == 0);
			lpUtf16->Length   = size;
			lpUtf16->Position = 0;
			size              = size / RLIB_SIZEOF(wchar_t);

			// swaps hi/low bits
			while (--size >= 0) {
				lpUtf16_le[size].High = lpUtf16_be[size].Low;
				lpUtf16_le[size].Low  = lpUtf16_be[size].High;
			}
		} //if

        return lpUtf16;
	} //if

	// Get size of destination buffer, in wchar_t's
	int cchBuffer = MultiByteToWideChar(codepage,                 // convert from codepage
										RLIB_MB_CONVERSION_FLAGS, // conversion flags
										pdata,                    // source data
										static_cast<int>(size),   // total length of source data, in char's (= bytes)
										NULL,                     // unused - no conversion done in this step
										0                         // request size of destination buffer, in wchar_t's
										);
	if (cchBuffer == 0) {
		return nullptr;
	} //if

    // Allocate destination buffer to store data
    auto pdata_out = new BufferedStream(cchBuffer * RLIB_SIZEOF(wchar_t));
	if (pdata_out != nullptr)
	{
		// Do the conversion
		int result = MultiByteToWideChar(codepage,                                        // convert from codepage
										 RLIB_MB_CONVERSION_FLAGS,                        // conversion flags
										 pdata,                                           // source data
										 static_cast<int>(size),                          // total length of source data, in char's (= bytes)
										 reinterpret_cast<LPWSTR>(pdata_out->ObjectData), // destination buffer
										 cchBuffer                                        // size of destination buffer, in wchar_t's
										 );
		if (result == 0) {
			delete pdata_out;
			return nullptr;
		} //if

		pdata_out->Length   = result * RLIB_SIZEOF(wchar_t); //stream should in bytes
		pdata_out->Position = 0; //stream rule
	} //if

	return pdata_out;
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::ToWideChar(Encoding codepage, const IO::Stream &stream, intptr_t length /* = -1 */)
{
	StreamReader reader(&stream, length);
	return ToWideChar(reader.BufferedData, reader.Capacity, codepage);
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::WideCharTo(LPCVOID _pdata, intptr_t size, Encoding codepage)
{
	assert(size > 0);

	LPCWSTR &pdata = reinterpret_cast<LPCWSTR &>(_pdata);

	if (RLIB_CMP_SHORT(pdata, UTF16_LEAD)) {
		pdata += sizeof(UTF16_LEAD);
		size  -= sizeof(UTF16_LEAD);
	} //if

	assert((size % sizeof(wchar_t)) == 0);
    size /= sizeof(wchar_t); // in wchar_t

	if (codepage == UTF16BEEncoding)
	{
		auto lpUtf16F = new BufferedStream(size * RLIB_SIZEOF(wchar_t));
		if (lpUtf16F != nullptr)
		{
			auto lpUtf16_be = reinterpret_cast<LPUNICHAR>(lpUtf16F->ObjectData);
			auto lpUtf16_le = reinterpret_cast<LPUNICHAR>(const_cast<LPWSTR>(pdata));

			lpUtf16F->Length   = size * RLIB_SIZEOF(wchar_t); // in bytes
			lpUtf16F->Position = 0;

			// swaps hi/low bits
			while (--size >= 0) {
				lpUtf16_be[size].High = lpUtf16_le[size].Low;
				lpUtf16_be[size].Low  = lpUtf16_le[size].High;
			}
		} //if
		
		return lpUtf16F;
	} //if

	const DWORD dwConversionFlags = RLIB_MB_CONVERSION_FLAGS;
#if (RLIB_WC_CONVERSION_FLAGS != WC_ERR_INVALID_CHARS) && (RLIB_WC_CONVERSION_FLAGS != 0) && (WINVER > _WIN32_WINNT_WIN7)
	// Note that this flag only applies when CodePage is specified as 
	// CP_UTF8 or 54936 (for Windows Vista and later)
	if (codepage == CP_UTF8 || codepage == 54936) {
		dwConversionFlags = WC_ERR_INVALID_CHARS;
	} //if
#endif

	// Get size of destination ANSI buffer, in char's (= bytes)
	int cbBuffer = WideCharToMultiByte(codepage,                 // convert to codepage
									   dwConversionFlags,        // specify conversion behavior
									   pdata,                    // source data
									   static_cast<int>(size),   // total source string length, in wchar_t's
									   NULL,                     // unused - no conversion required in this step
									   0,                        // request buffer size
									   NULL, nullptr             // unused
									   );
	if (cbBuffer == 0) {
		return nullptr;
	} //if

	// Allocate destination buffer for ANSI string
	auto pdata_out = new BufferedStream(cbBuffer * RLIB_SIZEOF(char)); 
	// sizeof(char) = 1 unsigned char
	if (pdata_out != nullptr)
	{
		// Do the conversion from UTF-16 to ANSI
		int result = WideCharToMultiByte(codepage,                                       // convert to codepage
										 dwConversionFlags,                              // specify conversion behavior
										 pdata,                                          // source data
										 static_cast<int>(size),                         // total source string length, in wchar_t's
										 reinterpret_cast<LPSTR>(pdata_out->ObjectData), // destination buffer
										 cbBuffer,                                       // destination buffer size, in bytes
										 NULL, nullptr                                   // unused
										 );
		if (result == 0) {
			delete pdata_out;
			return nullptr;
		} //if

		pdata_out->Length = result * RLIB_SIZEOF(char);
		pdata_out->Position = 0;
	} //if

	return pdata_out;
}

//-------------------------------------------------------------------------

BufferedStream *Encoder::WideCharTo(Encoding codepage, const IO::Stream &stream,
									intptr_t length /* = -1 */)
{
	StreamReader reader(&stream, length);
	return WideCharTo(reader.BufferedData, reader.Capacity, codepage);
}

//-------------------------------------------------------------------------

bool Encoder::WriteTextStream(OUT IO::Stream &outputStream, 
							  IN const IO::Stream &textStream, 
							  intptr_t bytesToWrite /* = -1 */, 
							  bool useBOM /* = true */, 
							  Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	StreamReader reader(&textStream, bytesToWrite);
	if (reader.BufferedData == nullptr) {
		return false;
	} //if

	return Encoder::WriteTextStream(outputStream, 
									reader.BufferedData, reader.Capacity,
									useBOM, 
									codepage);
}

//-------------------------------------------------------------------------

bool Encoder::WriteTextStream(OUT IO::Stream &outputStream, 
							  IN LPCVOID textData, 
							  intptr_t bytesToWrite, 
							  bool useBOM /* = true */, 
							  Text::Encoding codepage /* = Text::UnknownEncoding */)
{
	if (codepage == Text::UnknownEncoding) {
		codepage = GetCurrentEncoding();
	} //if

	ManagedObject<IO::BufferedStream> stream;
	switch (codepage) 
	{
	case System::Text::ASCIIEncoding:
		{
#ifdef _UNICODE
			// utf-16 to ascii
			stream = Text::Encoder::WideCharTo(textData, bytesToWrite, Text::ASCIIEncoding);
			if (stream.IsNull()) return false;

			outputStream.Write(stream->CurrentPtr, stream->MaxReadSize);
#else 
			outputStream.Write(textData, bytesToWrite);
#endif // _UNICODE
		}
		break;
	case System::Text::UTF16Encoding:
		{
#ifdef _UNICODE
			if (useBOM) {
				outputStream.Write(UTF16_LEAD, sizeof(UTF16_LEAD));
			} //if
			outputStream.Write(textData, bytesToWrite);
#else 
			// ascii to utf-16
			stream = Text::Encoder::ToWideChar(textData, bytesToWrite, Text::ASCIIEncoding);
			if (stream.IsNull()) return false;

			if (useBOM) {
				outputStream.Write(UTF16_LEAD, sizeof(UTF16_LEAD));
			} //if
			outputStream.Write(stream->CurrentPtr, stream->MaxReadSize);
#endif // _UNICODE
		}
		break;
	case System::Text::UTF8Encoding:
		{
#ifdef _UNICODE
			// utf-16 to utf-8
			stream = Text::Encoder::WideCharTo(textData, bytesToWrite, codepage);
#else 
			// ascii to utf-8
			ManagedObject<IO::BufferedStream> tmp_stream = Text::Encoder::ToWideChar(textData, bytesToWrite,
																					 Text::ASCIIEncoding);
			if (tmp_stream.IsNull()) return false;

			stream = Text::Encoder::WideCharTo(codepage, *tmp_stream);
#endif // _UNICODE

			if (stream.IsNull()) return false;

			if (useBOM) {
				outputStream.Write(UTF8_LEAD, sizeof(UTF8_LEAD));
			} //if
			outputStream.Write(stream->CurrentPtr, stream->MaxReadSize);
		}
		break;
	case System::Text::UTF16BEEncoding:
		{
#ifdef _UNICODE
			// utf-16 to utf-16f
			stream = Text::Encoder::WideCharTo(textData, bytesToWrite, codepage);
#else 
			// ascii to utf-16f
			ManagedObject<IO::BufferedStream> tmp_stream = Text::Encoder::ToWideChar(textData, bytesToWrite,
																					 Text::ASCIIEncoding);
			if (tmp_stream.IsNull()) return false;

			stream = Text::Encoder::WideCharTo(codepage, *tmp_stream);
#endif // _UNICODE

			if (stream.IsNull()) return false;
			if (useBOM) {
				outputStream.Write(UTF16BE_LEAD, sizeof(UTF16BE_LEAD));
			} //if
			outputStream.Write(stream->CurrentPtr, stream->MaxReadSize);
		}
		break;
	default:
		{
			trace(!"not supported");
			return false;
		}
	}
	return true;
}
