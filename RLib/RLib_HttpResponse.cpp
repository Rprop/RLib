/********************************************************************
Created:	2011/02/18  9:29
Filename: 	RLib_HttpResponse.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_HttpRequest.h"
#include "RLib_Fundamental.h"
#include "RLib_DeflateStream.h"
#include "RLib_StringHelper.h"
#include "RLib_GZipStream.h"
#include "RLib_Object.h"
#include "RLib_UnmanagedStream.h"
#include "RLib_Utility.h"
#include "RLib_HttpCookie.h"

using namespace System::IO;
using namespace System::IO::Compression;
using namespace System::Net;

//-------------------------------------------------------------------------

HttpException *HttpResponse::GetLastException()
{
    return &this->m_error;
}

//-------------------------------------------------------------------------

static ResponseStream *rlib_parse_chunk_data(char *lp_chunk_data, intptr_t nsize)
{
	auto pstream = new ResponseStream(RLIB_ROUNDUP(nsize));
	if (pstream == nullptr) {
		return nullptr;
	} //if

	// to small data size, just ignore it
	if (nsize < 16) {
		pstream->Write(lp_chunk_data, nsize);
	} else {
		// get chunked data size
		const int crlf        = RLIB_COUNTOF_STR("\r\n");
		int block_size        = 0;		
		LPSTR pDataEnd        = lp_chunk_data + nsize;
		LPSTR pBlockDataBegin = lp_chunk_data, pBlockDataEnd = lp_chunk_data;
		while (pBlockDataBegin < pDataEnd) {
			pBlockDataEnd = Utility::memstr(pBlockDataBegin, pDataEnd - pBlockDataBegin, RLIB_STR_LEN("\r\n"));
			if (pBlockDataEnd == nullptr) {
				trace(!"invalid chunked data");
				break;
			} //if
			//pBlockDataEnd[0] = '\0';
			block_size = Int32::TryParse(pBlockDataBegin, 16);
			//pBlockDataEnd[0] = '\r';
			if (block_size <= 0) {
				break;
			} //if
			pstream->Write(pBlockDataEnd + crlf, block_size);
			pBlockDataBegin = pBlockDataEnd + crlf + block_size + crlf;

			// skip CRLF
// 			crlf = 0;
// 			while (*pBlockDataBegin == '\r' || *pBlockDataBegin == '\n') {
// 				++crlf;
// 				++pBlockDataBegin;
// 			}
		}
	} //if

	return pstream;
}

//-------------------------------------------------------------------------

RLIB_INLINE void SetDefaultProperty(HttpResponse *_t)
{
	_t->StatusCode        = -1;
	_t->ProtocolVer       = _R("INVALID");
	_t->StatusDescription = _R("ERROR");
}

//-------------------------------------------------------------------------

void HttpResponse::SaveResponseBody(MemoryStream *pResponseData)
{
	this->m_response = new ResponseStream(pResponseData->MaxReadSize);
	if (this->m_response != nullptr) {
		this->m_response->Write(pResponseData->CurrentPtr, pResponseData->MaxReadSize);
		this->ContentLength = pResponseData->MaxReadSize;
	} else {
		trace(!"cannot save response body");
		this->ContentLength = 0;
	} //if
}

//-------------------------------------------------------------------------

char *HttpResponse::ParseResponseStatus(char *pData, intptr_t nSize, IO::MemoryStream *responseData)
{
	assert(nSize >= RLIB_COUNTOF_STR("HTTP/"));
	if (!StringStartWith_4_A(pData, "HTTP") || pData[RLIB_COUNTOF_STR("HTTP")] != '/') {
		// servers may not echo "HTTP/..."
		SetDefaultProperty(this);
		this->SaveResponseBody(responseData);

		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, 
						  _T("missing HTTP response message"));
		return nullptr;
	} //if

	pData += RLIB_COUNTOF_STR("HTTP/");
	nSize -= RLIB_COUNTOF_STR("HTTP/");

	char *pEndPoint  = pData + nSize;
	char *pNextBlank = Utility::memstr(pData, nSize, RLIB_STR_LEN(" "));
	if (pNextBlank  == nullptr) {
		SetDefaultProperty(this);
		this->SaveResponseBody(responseData);

		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, 
						  _T("missing space when parsing status-line"));
		return nullptr;
	} //if

	// HTTP response version
	assert((pNextBlank - pData) == 3);
	this->ProtocolVer.copy(pData, pNextBlank - pData);

	pData      = pNextBlank + RLIB_COUNTOF_STR(" ");
	pNextBlank = Utility::memstr(pData, pEndPoint - pData, RLIB_STR_LEN(" "));
	if (pNextBlank == nullptr) {
		this->StatusCode        = -1;
		this->StatusDescription = _R("ERROR");
		this->SaveResponseBody(responseData);

		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, 
						  _T("missing space when parsing status-line"));
		return nullptr;
	} //if

	// HTTP response status code
	assert(pNextBlank - pData <= 3);
	this->StatusCode = Int32::TryParse(pData);

	pData      = pNextBlank + RLIB_COUNTOF_STR(" ");
	pNextBlank = Utility::memstr(pData, pEndPoint - pData, RLIB_STR_LEN("\r\n"));
	if (pNextBlank == nullptr) {
		this->StatusDescription = _R("ERROR");
		this->SaveResponseBody(responseData);

		RLIB_SetException(this->m_error, RLIB_EXCEPTION_ID, 
						  _T("missing CRLF when parsing status-line"));
		return nullptr;
	} //if

	// HTTP response status text
	assert((pNextBlank - pData) > 0);
	this->StatusDescription.copy(pData, pNextBlank - pData);

	return pNextBlank + RLIB_COUNTOF_STR("\r\n");
}

//-------------------------------------------------------------------------

HttpResponse *HttpResponse::Create(IO::MemoryStream *responseData, bool ignoreStatus,
								   bool ignoreHeaders)
{
    HttpResponse *_t = new HttpResponse;
	if (_t == nullptr) {
		// exception was set in HttpRequest.GetResponse()
		return nullptr;
	} //if
    
	intptr_t size = responseData->MaxReadSize;
	char *lpdata  = reinterpret_cast<char *>(responseData->CurrentPtr);
	char *lpend   = lpdata + size;
	assert(size > 0);
	assert(lpdata != nullptr);
	
	char *pHeadersBegin;
	if (ignoreStatus) {
		pHeadersBegin = Utility::memstr(lpdata, size, RLIB_STR_LEN("\r\n"));
		assert(pHeadersBegin < lpend);
		if (pHeadersBegin != nullptr) {
			pHeadersBegin += RLIB_COUNTOF_STR(_T("\r\n"));
		} else {
			RLIB_SetException(_t->m_error, RLIB_EXCEPTION_ID, 
							  _T("missing CRLF when parsing status-line"));
			return _t;
		} //if
	} else {
		pHeadersBegin = _t->ParseResponseStatus(lpdata, size, responseData);
		assert(pHeadersBegin < lpend);
		if (pHeadersBegin == nullptr) {
			return _t;
		} //if
	} //if

	// response headers end-delimiter
	char *pHeadersEnd = Utility::memstr(pHeadersBegin, lpend - pHeadersBegin, RLIB_STR_LEN("\r\n\r\n"));
	assert(pHeadersEnd < lpend);
	if (pHeadersEnd == nullptr) {
		RLIB_SetException(_t->m_error, RLIB_EXCEPTION_ID, 
						  _T("missing end-delimiter when parsing response headers"));
		return _t;
	} //if

	// response body
	auto pBodyBegin   = pHeadersEnd + RLIB_COUNTOF_STR("\r\n\r\n");
	intptr_t bodySize = lpend - pBodyBegin;
	assert(pBodyBegin <= lpend);
	assert(bodySize >= 0);

	if (!ignoreHeaders) {
		// save all headers
		_t->Headers.WriteByteArray(pHeadersBegin, pHeadersEnd - pHeadersBegin);	
		// set headers property
		_t->ContentType   = _t->Headers.Get(RLIB_STR_LEN("Content-Type"));
	} else if (bodySize > 0) {
		// save all headers, otherwise the body may not be handled correctly
		_t->Headers.WriteByteArray(pHeadersBegin, pHeadersEnd - pHeadersBegin);
	} //if

	if (bodySize > 0) {
		assert(_t->m_response == nullptr);
		if (_t->Headers.Get(RLIB_STR_LEN("Transfer-Encoding")).Contains(_T("chunked"))) {		
			_t->m_response = rlib_parse_chunk_data(reinterpret_cast<char *>(pBodyBegin),
												   bodySize);
		} else {
			_t->m_response = new ResponseStream(bodySize);
			if (_t->m_response != nullptr) {
				_t->m_response->Write(pBodyBegin, bodySize);
			} //if
		} //if

		if (_t->m_response != nullptr) {
			_t->m_response->Position = 0;
			_t->ContentLength        = _t->m_response->Length;
		} else {
			trace(!"failed to instantiate ResponseStream");
			RLIB_SetException(_t->m_error, RLIB_EXCEPTION_ID,
							  _T("failed to instantiate ResponseStream"));
		} //if

	} else {
		_t->ContentLength = 0;
	} //if

	return _t;
}

//-------------------------------------------------------------------------

HttpResponse *HttpResponse::Create(IO::MemoryStream *responseData, DecompressionMethod automaticDecompression,
								   bool ignoreStatus, bool ignoreHeaders)
{
    HttpResponse *response = HttpResponse::Create(responseData, ignoreStatus, ignoreHeaders);
    if (response != nullptr)
    {
		if (response->m_response == nullptr || response->ContentLength <= 2) {
			goto __return;
		} else {
			auto &&contentEncoding = response->Headers.Get("Content-Encoding");
			if (contentEncoding.IsNull() ||
				(!contentEncoding.ContainsNoCase(_T("gzip")) &&
				 !contentEncoding.ContainsNoCase(_T("deflate")))) {
				goto __return;
			} //if
		} //if

		assert(response->m_response->Position == 0);
		assert(response->m_response->Length >= response->ContentLength);
		auto lpdata = reinterpret_cast<unsigned char *>(response->m_response->ObjectData);
		if (lpdata == nullptr) {
			goto __return;
		} //if

		switch (automaticDecompression)
		{
		case DecompressionMethod::Auto:
		case DecompressionMethod::GZip:
			{
				// gzip format
				if (lpdata[0] == 0x1FU && lpdata[1] == 0x8BU/* && lpdata[2] == 0x08U*/) {
					unsigned long buffer_size = static_cast<unsigned long>(response->ContentLength * 6);
					ManagedObject<BufferedStream> uncompressedStream = new BufferedStream(static_cast<intptr_t>(buffer_size));
					if (uncompressedStream.IsNotNull()) {
						auto decompressResult = GZip::Decompress(lpdata,
																 static_cast<unsigned long>(response->ContentLength),
																 static_cast<unsigned char *>(uncompressedStream->ObjectData),
																 &buffer_size,
																 RLIB_MAX_WBITS + 32);
						if (decompressResult >= 0) {
							uncompressedStream->Length = static_cast<intptr_t>(buffer_size);
							response->m_response->ExChange(*uncompressedStream);
							break;
						} //if
					} //if
				} //if
				if (automaticDecompression != DecompressionMethod::Auto) break;
			}
		case DecompressionMethod::Deflate:
			{
				auto line_end = Utility::memstr(lpdata, RLIB_MIN(response->ContentLength, 16), RLIB_STR_LEN("\r\n"));
				if (line_end != nullptr) {
					// get the compressed data size
					char hexsize[16] = { 0 };
					memcpy(hexsize, lpdata, static_cast<size_t>(RLIB_MIN(RLIB_COUNTOF(hexsize) - 1, line_end - lpdata)));

					auto data_size = Int32::TryParse(hexsize, 16);
					if (data_size > 0) {
						// deflate
						unsigned long buffer_size = static_cast<size_t>(data_size) * 6;
						ManagedObject<BufferedStream> uncompressedStream = new BufferedStream(static_cast<intptr_t>(buffer_size));
						if (uncompressedStream.IsNotNull()) {
							auto decompressResult = GZip::Decompress(line_end + RLIB_COUNTOF_STR("\r\n"), // lpdata
																	 static_cast<unsigned long>(response->ContentLength),
																	 static_cast<unsigned char *>(uncompressedStream->ObjectData),
																	 &buffer_size,
																	 RLIB_MAX_WBITS + 32);
							if (decompressResult >= 0) {
								uncompressedStream->Length = static_cast<intptr_t>(buffer_size);
								response->m_response->ExChange(*uncompressedStream);
								break;
							} //if
						} //if
					} //if	
				}
			}
		default:
			{
				trace(!"not implemented or supported!");
			}
		}
    }
__return: 
	return response;
}

//-------------------------------------------------------------------------

HttpResponse::~HttpResponse()
{
    RLIB_Delete(this->m_response);
    RLIB_Delete(this->ResponseUri);
}

//-------------------------------------------------------------------------

String HttpResponse::GetAllCookies()
{
	String ck;
	HttpCookie::EnumAll(this->Headers.ToByteArray(), ck);
	return ck;
}

//-------------------------------------------------------------------------

String HttpResponse::GetAllResponseHeaders()
{
	return String(this->Headers.ToByteArray());
}

//-------------------------------------------------------------------------

String HttpResponse::GetResponseHeader(LPCSTR headerName, intptr_t length /* = -1 */)
{
	assert(headerName && length != 0);
    return length < 0 ? this->Headers.Get(headerName) : this->Headers.Get(headerName, length);
}

//-------------------------------------------------------------------------

ResponseStream *HttpResponse::GetResponseStream()
{
    return this->m_response;
}
