/********************************************************************
Created:	2012/04/21  14:48
Filename: 	RLib_WebClient.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_StringHelper.h"
#include "RLib_GlobalizeString.h"
#include "RLib_WebClient.h"
#include "RLib_File.h"

using namespace System::IO;
using namespace System::Net;

//-------------------------------------------------------------------------

HttpRequest *WebClient::GetHttpRequest(const String &Url)
{
    auto pRequest = new HttpRequest(Url);
    if (pRequest != nullptr)
    {
		pRequest->UserAgent = TSTR(_T("RLib Visitor/") _T(RLIB_STRINGIZE(RLIB_VER)));
// 		pRequest->Headers.Add(RLIB_STR_LEN("Accept-Charset"),  RLIB_STR_LEN("iso-8859-1,utf-8;q=0.7,*;q=0.7"));
// 		pRequest->Headers.Add(RLIB_STR_LEN("Accept-Language"), RLIB_STR_LEN("zh-cn, zh;q=1.0,en"));
		pRequest->Headers.Add(RLIB_STR_LEN("Accept-Encoding"), RLIB_STR_LEN("gzip, x-gzip; q=0.9"));
		pRequest->AutomaticDecompression = System::Net::DecompressionMethod::GZip;

#ifdef _DEBUG
		if (pRequest->GetLastException()->HResult != STATUS_SUCCESS) {
			alert(pRequest->GetLastException()->Message);
		} //if
#endif // _DEBUG
    }
    
	return pRequest;
}

//-------------------------------------------------------------------------

ResponseStream *WebClient::GetRawResponseStream(const String &Url,
												HttpRequest *pRequest /* = nullptr */)
{
	bool bDelRequest = false;
	if (pRequest == nullptr) {
		pRequest = WebClient::GetHttpRequest(Url);
		if (pRequest == nullptr) return nullptr;

		bDelRequest = true;
	}//if

	auto responseStream = pRequest->GetResponseStream();
	if (responseStream == nullptr) {
		if(bDelRequest) delete pRequest;
		return nullptr;
    } //if

    auto resStream = new ResponseStream(1);
	responseStream->ExChange(*resStream);

	if (bDelRequest) delete pRequest;
	return resStream;
}

//-------------------------------------------------------------------------

ResponseStream *WebClient::GetResponseStream(const String &Url,
											 HttpRequest *pRequest /* = nullptr */,
											 HttpResponse **ppResponse OUT /* = nullptr */)
{
	HttpResponse *pResponse;
	if (pRequest == nullptr) {
		pRequest = WebClient::GetHttpRequest(Url);
		if (pRequest != nullptr) {
			pResponse = pRequest->GetResponse();
			delete pRequest;
		} else {
			return nullptr;
		} //if
	} else {
		pResponse = pRequest->GetResponse();
	} //if

	if (pResponse == nullptr){
		return nullptr;
	} //if

	// handle exception
	if (pResponse->GetLastException()->HResult != STATUS_SUCCESS) {
		delete pResponse;
		return nullptr;
	} //if

	// 输出响应对象, 其它不予处理
	if (ppResponse != nullptr) {
		*ppResponse = pResponse;
		return pResponse->GetResponseStream();
	} //if

	// 不要求输出响应对象, 那么拷贝响应流并关闭响应对象
	auto resStream = new ResponseStream(1);
	if (resStream != nullptr) {
		auto resp  = pResponse->GetResponseStream();
		if (resp  != nullptr) {
			resp->ExChange(*resStream);
		} //if
	} //if
		
	delete pResponse;

	return resStream;
}

//-------------------------------------------------------------------------

String WebClient::GetResponseText(const String &Url,
								  HttpRequest *pRequest /* = nullptr */,
								  HttpResponse **ppResponse OUT /* = nullptr */)
{
	HttpResponse *pResponse = nullptr;
	auto responseStream = WebClient::GetResponseStream(Url, pRequest, &pResponse);
	if (pResponse == nullptr) {
		assert(responseStream == nullptr);
		return Nothing;
	} else if (responseStream == nullptr ||
			   responseStream->Length <= 0)
	{
		if (ppResponse == nullptr) {
			delete pResponse;
		} else {
			*ppResponse = pResponse;
		} //if

		// returns empty value, not null
		return String::nullpcstr;
	} //if

	String responseText;
	do 
	{
		if (pResponse->ContentType.ContainsNoCase(_T("utf-8")))
		{
			auto dStream = Text::Encoder::ToWideChar(System::Text::UTF8Encoding,
													 *responseStream);
			if (dStream == nullptr) break;

#ifndef _UNICODE
			auto tStream = dStream;
			dStream = Text::Encoder::WideCharTo(System::Text::ASCIIEncoding, *tStream);
			delete tStream;
#endif // _UNICODE

			responseText.copy(StringReference(dStream->ObjectData), 
							  dStream->Length / static_cast<intptr_t>(sizeof(TCHAR)));

			delete dStream;
		} else if (pResponse->ContentType.ContainsNoCase(_T("gb2312"))) {
#ifdef _UNICODE
			auto tStream = Text::Encoder::ToWideChar(System::Text::ASCIIEncoding,
													 *responseStream);
			if (tStream == nullptr) break;

			responseText.copy(reinterpret_cast<LPCWSTR>(tStream->ObjectData), 
							  tStream->Length / static_cast<intptr_t>(sizeof(wchar_t)));

			delete tStream;
#else 
			responseText.copy(reinterpret_cast<LPCSTR>(responseStream->ObjectData),
							  responseStream->Length / static_cast<intptr_t>(sizeof(char)));
#endif // _UNICDOE
		} else {
			assert(responseStream->MaxReadSize > 0);
			auto tStream = Text::Encoder::ToCurrentEncoding(responseStream->CurrentPtr,
															responseStream->MaxReadSize,
															Text::UnknownEncoding);
			if (tStream == nullptr) break;

			responseText.copy(StringReference(tStream->ObjectData), 
							  tStream->Length / static_cast<intptr_t>(sizeof(TCHAR)));
			delete tStream;
		}
#pragma warning(disable:4127) //条件表达式是常量
	} while (false);
#pragma warning(default:4127) //条件表达式是常量

	//输出响应对象
	if (ppResponse == nullptr || responseText.IsNull()) {
		delete pResponse;
	} else {
		*ppResponse = pResponse;
	} //if

	return responseText;
}

//-------------------------------------------------------------------------

void WebClient::WriteRequestStream(HttpRequest *pRequest, const String &Data)
{
	GlobalizeString globalizeData(Data);
	if (pRequest->ContentType.IsNullOrEmpty()) {
		pRequest->ContentType = _R("application/x-www-form-urlencoded; charset=gb2312");
	} //if
	pRequest->Method          = _R("POST");
	pRequest->ContentLength   = globalizeData.sizeofGBK();
	pRequest->GetRequestStream(pRequest->ContentLength)->Write(globalizeData.toGBK(),
										                       pRequest->ContentLength);
}

//-------------------------------------------------------------------------

ResponseStream *WebClient::PostRawResponseStream(const String &Url,
												 const String &Data, 
												 HttpRequest *pRequest IN /* = nullptr */)
{
	bool bDelRequest = false;
	if (pRequest == nullptr) {
		pRequest = WebClient::GetHttpRequest(Url);
		if (pRequest == nullptr) return nullptr;

		bDelRequest = true;
	} //if

	WebClient::WriteRequestStream(pRequest, Data);

	auto resStream = WebClient::GetRawResponseStream(Url, pRequest);
	if (bDelRequest) delete pRequest;
	return resStream;
}

//-------------------------------------------------------------------------

ResponseStream *WebClient::PostResponseStream(const String &Url, const String &Data,
											  HttpRequest *pRequest IN /* = nullptr */,
											  HttpResponse **ppResponse OUT /* = nullptr */)
{
	bool bDelRequest = false;
	if (pRequest == nullptr) {
		pRequest = WebClient::GetHttpRequest(Url);
		if (pRequest == nullptr) return nullptr;

		bDelRequest = true;
	} //if

	WebClient::WriteRequestStream(pRequest, Data);

	auto resStream = WebClient::GetResponseStream(Url, pRequest, ppResponse);
	if (bDelRequest) delete pRequest;
	return resStream;
}

//-------------------------------------------------------------------------

String WebClient::PostResponseText(const String &Url, const String &Data,
								   HttpRequest *pRequest IN /* = nullptr */,
								   HttpResponse **ppResponse OUT /* = nullptr */)
{
	bool bDelRequest = false;
	if (pRequest == nullptr) {
		pRequest = WebClient::GetHttpRequest(Url);
		if (pRequest == nullptr) return Nothing;

		bDelRequest = true;
	} //if

	WebClient::WriteRequestStream(pRequest, Data);

	auto resString = WebClient::GetResponseText(Url, pRequest, ppResponse);
	if (bDelRequest) delete pRequest;
	return resString;
}

//-------------------------------------------------------------------------

bool WebClient::DownloadFile(const String &Url, 
							 const String &FilePath,
							 HttpRequest *pRequest, 
							 HttpResponse **ppResponse OUT /* = nullptr */)
{
    auto resStream = WebClient::GetResponseStream(Url,
												  pRequest, ppResponse);
    if (resStream != nullptr)
    {
		bool result  = false;
        auto outFile = IO::File::Create(FilePath, FileMode::CreateNew);
        if (outFile != nullptr) {
            outFile->Write(resStream->ObjectData, resStream->Length);
            delete outFile;
			result = true;
        } //if

		// 当输出响应对象时, 不能删除该流
		if (ppResponse == nullptr) delete resStream;
		return result;
    } //if
    return false;
}
