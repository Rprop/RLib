/********************************************************************
	Created:	2016/07/18  19:15
	Filename: 	HttpRequest.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>

//-------------------------------------------------------------------------

int __stdcall WinMain(__in HINSTANCE /*hInstance*/, __in_opt HINSTANCE /*hPrevInstance*/,
					  __in LPSTR /*lpCmdLine*/, __in int /*nShowCmd*/)
{
	// WebClient is a wrapper class for HttpRequest/HttpResponse,
	// and platform-independent in a sense.

	// performs a HTTP GET request and gets response as string
	String page = WebClient::GetResponseText(_R("http://rlib.cf/"));

	// also, HTTPS/SSL is supported.
	String ssl_page = WebClient::GetResponseText(_R("https://www.alipay.com/"));

	// performs a HTTP POST request with string data
	String post_page = WebClient::PostResponseText(_R("http://rlib.cf/"), _R("post data"));

	return STATUS_SUCCESS;
}