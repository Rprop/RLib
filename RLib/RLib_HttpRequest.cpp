/********************************************************************
Created:	2011/02/18  9:29
Filename: 	RLib_HttpRequest.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_HttpRequest.h"
#include "RLib_Fundamental.h"
#include "RLib_StringHelper.h"
#include "RLib_GlobalizeString.h"
#include "RLib_MemoryPool.h"
#include "RLib_Utility.h"
#include "RLib_HttpCookie.h"
using namespace System;
using namespace System::IO;
using namespace System::Net;

#define __FLUSH_DATA(p,n)            flush(data_out, p, n, this->m_recv_callback, this->m_recv_callback_param)
#define __UPDATE_SEND_PARAMES(dummy) if (this->m_recv_callback_param) {                           \
										this->m_recv_callback_param->sizeToRecv   = nSizeToRecv;  \
										this->m_recv_callback_param->received     = nReceived;    \
										this->m_recv_callback_param->lastReceived = nLastReceived;\
										this->m_recv_callback_param->sizeOfBuffer = nTotalSize;   \
										this->m_recv_callback_param->recvBuffer   = recvBuffer;   \
}
#define __INVOKE_CONN_CALLBACK(evt) this->m_conn_callback(this->m_conn_callback_param, evt)
#define __INVOKE_SEND_CALLBACK(evt) this->m_send_callback(this->m_send_callback_param, evt)
#define __INVOKE_RECV_CALLBACK(evt) this->m_recv_callback(this->m_recv_callback_param, evt)
#define __INVOKE_CALLBACK(cb,p,evt) cb(p, evt)

//-------------------------------------------------------------------------

static bool __get_http_response_header_value(const char *pheaders, 
											 char *pname, size_t lname,
											 char *pout, int nsize)
{
	const char *pstr = pheaders;
	while ((pstr = Utility::stristr(pstr, pname)) != nullptr)
	{
		if (pstr != pheaders && *(pstr - 1) != '\n') {
			pstr += lname;
			continue;
		} //if

		pstr += lname;
		if (*pstr != ':') continue;

		pstr += RLIB_COUNTOF_STR(": ");

		const char *pend = strchr(pstr, '\n');
		if (pend != nullptr) {
			if (*(pend - 1) == '\r') --pend;
			if ((pend - pstr + 1) <= nsize) {
				memcpy(pout, pstr, static_cast<size_t>(pend - pstr));
				pout[pend - pstr] = '\0';
				return true;
			} //if
			trace(!"overflow");
			break;
		} //if
	}
	return false;
}

//-------------------------------------------------------------------------

bool HttpRequest::init_socket()
{
	auto lpsocket     = reinterpret_cast<Sockets *>(this->m_socket);
	this->IsConnected = false;
	RLIB_InitClass(lpsocket, 
				   Sockets(this->Address->Scheme.Length != 5 || this->Address->Scheme != _T("HTTPS") ? Sockets::SSL_NONE : Sockets::SSL_CLIENT));
	
	// check if there was any exception
	if (lpsocket->GetLastException()->HResult == STATUS_SUCCESS){
		return true;
	} //if

	alert(lpsocket->GetLastException()->Message);
	return false;
}

//-------------------------------------------------------------------------

HttpRequest::HttpRequest(const String &url)
{
#pragma region 初始化类成员
	this->m_conn_callback        = nullptr;
	this->m_conn_callback_param  = nullptr;
	this->m_send_callback        = nullptr;
	this->m_send_callback_param  = nullptr;
	this->m_recv_callback        = nullptr;
	this->m_recv_callback_param  = nullptr;
	this->m_request              = nullptr;
	this->m_realuri              = nullptr;
	this->m_extern_out           = nullptr;
	this->m_default_out          = nullptr;
	this->ContentLength          = 0;
	this->AutomaticDecompression = DecompressionMethod::None;
	this->WebProxy               = { INADDR_ANY, NULL };
	this->Address                = new Uri(url);
	this->Method                 = _R("GET");
	this->ProtocolVer            = _R("1.1");
	this->ConnectTimeout         = 8000;
	this->Timeout                = 8000;
	this->RecvTimeout            = 1000;
	this->MaxAutoRedirections    = 3;
	this->AutoRedirect           = true;
	this->IgnoreResponseBody     = false;
	this->IgnoreResponseHeaders  = false;
	this->IgnoreResponseStatus   = false;
	this->KeepAlive              = false;

	this->init_socket();
#pragma endregion 初始化成员
}

//-------------------------------------------------------------------------

HttpRequest::~HttpRequest()
{
	RLIB_Delete(this->m_realuri);
    RLIB_Delete(this->m_request);
	RLIB_Delete(this->m_default_out);
	delete this->Address;

	RLIB_DELAY_DESTROY(Sockets, this->m_socket);
}

//-------------------------------------------------------------------------

bool HttpRequest::OnConnect()
{
	RLIB_DELAY_ALIAS(Sockets, this->m_socket, lpsocket);

	timeval tv;
	tv.tv_sec  = this->ConnectTimeout / 1000;
	tv.tv_usec = this->ConnectTimeout - (tv.tv_sec * 1000);
	if (tv.tv_usec < 0) tv.tv_usec = 0;
	// notify
	this->m_conn_callback && __INVOKE_CONN_CALLBACK(CONN_EVENT_BEGIN);
__retry_connect:
	// ignore INADDR_NONE
	if (this->WebProxy.addr != INADDR_ANY) {
		if (lpsocket->Connect(this->WebProxy.addr, this->WebProxy.port, &tv)) {
			this->IsConnected = true;
        } //if
    } else if (lpsocket->Connect(this->Address->Host, static_cast<int>(this->Address->Host.Length), 
									   this->Address->Port, &tv)) {
		this->IsConnected = true;
    } //if

	// notify
	if (this->m_conn_callback && __INVOKE_CONN_CALLBACK(CONN_EVENT_END) == CONN_RESULT_RETRY) {
		goto __retry_connect;
	} //if
	 
	return this->IsConnected;
}

//-------------------------------------------------------------------------

RequestStream *HttpRequest::generate_request_packet()
{
	auto local_request = new RequestStream(RLIB_DEFAULT_BUFFER_SIZE);
	if (local_request == nullptr) return nullptr;

	// construct request line
	if (this->m_send_callback == nullptr || 
		__INVOKE_SEND_CALLBACK(SEND_EVENT_REQUEST_LINE) == SEND_RESULT_CONTINUE)
	{
		RLIB_StreamWriteStringA(local_request, Method);
		if (this->WebProxy.addr != INADDR_ANY)
		{
			RLIB_StreamWriteA(local_request, (" http://"));
			RLIB_StreamWriteStringA(local_request, this->Address->Host);
			if (this->Address->Port != 80 && this->Address->Port != 443) {
				RLIB_StreamWriteA(local_request, (":"));
				RLIB_StreamWriteStringA(local_request, Int32(this->Address->Port).ToString());
			} //if
		} else {
			RLIB_StreamWriteA(local_request, " ");
		} //if
		RLIB_StreamWriteStringA(local_request, this->Address->PathAndQuery);
		RLIB_StreamWriteA(local_request, " HTTP/");
		RLIB_StreamWriteStringA(local_request, ProtocolVer);
		RLIB_StreamWriteA(local_request, "\r\n");
	} //if

	// construct request headers
	if (this->m_send_callback == nullptr || 
		__INVOKE_SEND_CALLBACK(SEND_EVENT_REQUEST_HEADER) == SEND_RESULT_CONTINUE)
	{
		if (!this->UserAgent.IsNullOrEmpty()) {
			RLIB_StreamWriteA(local_request, "User-Agent: ");
			RLIB_StreamWriteStringA(local_request, this->UserAgent);
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if

		RLIB_StreamWriteA(local_request, "Host: ");
		RLIB_StreamWriteStringA(local_request, this->Address->Host);
		if (this->Address->Port != 80 && this->Address->Port != 443) {
			RLIB_StreamWriteA(local_request, ":");
			RLIB_StreamWriteStringA(local_request, Int32(this->Address->Port).ToString());
		} //if
		RLIB_StreamWriteA(local_request, "\r\n");

		// Keep-Alive
		RLIB_StreamWriteA(local_request, "Connection: ");
		this->KeepAlive ? RLIB_StreamWriteA(local_request, "Keep-Alive") : RLIB_StreamWriteA(local_request, "close");
		RLIB_StreamWriteA(local_request, "\r\n");

		if (!this->Referer.IsNullOrEmpty()) {
			RLIB_StreamWriteA(local_request, "Referer: ");
			RLIB_StreamWriteStringA(local_request, this->Referer);
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if

		if (!this->ContentType.IsNullOrEmpty()) {
			RLIB_StreamWriteA(local_request, "Content-Type: ");
			RLIB_StreamWriteStringA(local_request, this->ContentType);
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if

		if (!this->Cookie.IsNullOrEmpty()) {
			RLIB_StreamWriteA(local_request, "Cookie: ");
			RLIB_StreamWriteStringA(local_request, this->Cookie);
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if

		if (this->ContentLength > 0) {
			RLIB_StreamWriteA(local_request, "Content-Length: ");
			RLIB_StreamWriteStringA(local_request, ToIntPtr(this->ContentLength).ToString());
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if

		// custom headers bytes
		if (!this->Headers.IsEmpty()) {
			local_request->Write(this->Headers.ToByteArray(), this->Headers.GetByteArraySize());
			RLIB_StreamWriteA(local_request, "\r\n\r\n");
		} else {
			RLIB_StreamWriteA(local_request, "\r\n");
		} //if
	}
	return local_request;
}

//-------------------------------------------------------------------------

bool HttpRequest::OnSend()
{
	auto lpsocket = reinterpret_cast<Sockets *>(this->m_socket);

	// set send timeout
	lpsocket->SetSendTimeout(this->Timeout);
	 
	bool send_state = false;

	// construct http request data
	auto local_request = generate_request_packet();
	while (local_request) {
		// send request packet
		int request_length = static_cast<int>(local_request->Length); // exclude '\0'
		int bytes_sent     = lpsocket->Send(static_cast<char *>(local_request->ObjectData),
											request_length);
		delete local_request;
		if (bytes_sent != request_length) {
//			send_state = false;
			alert(lpsocket->GetLastException()->Message);
			break;
		} //if

		// send body if exists
		if (this->m_request && this->m_request->Length > 0) {
			if (this->m_send_callback == nullptr ||
				__INVOKE_SEND_CALLBACK(SEND_EVENT_REQUEST_BODY) == SEND_RESULT_CONTINUE) 
			{
				int body_length = static_cast<int>(this->m_request->Length);
				bytes_sent      = lpsocket->Send(static_cast<char *>(this->m_request->ObjectData),
												 body_length);
				if (bytes_sent != body_length) {
//					send_state = false;
					alert(lpsocket->GetLastException()->Message);
					break;
				} //if
			} //if
		} //if

		send_state = true;
		break;
	}

	// notify
	if (this->m_send_callback && __INVOKE_SEND_CALLBACK(SEND_EVENT_END) != SEND_RESULT_CONTINUE){
		send_state = false; // user cancels/aborts the request
	} //if
    
	return send_state;
}

//-------------------------------------------------------------------------

bool HttpRequest::OnRecv()
{
	if (this->m_extern_out == nullptr) {
		if (this->m_default_out == nullptr) {
			this->m_default_out = new BufferedStream(RLIB_NETWORK_BUFFER_SIZE);
			if (this->m_default_out == nullptr) {
				RLIB_SetException((*this->GetLastException()), RLIB_EXCEPTION_ID,
								  _T("failed to instance default output"));
				return false;
			} //if
		} //if
		this->m_default_out->Position = 0;
	} //if

	// set recv timeout
	reinterpret_cast<Sockets *>(this->m_socket)->SetReceiveTimeout(this->Timeout);

	// notify
	if (this->m_recv_callback != nullptr) {
		switch (__INVOKE_RECV_CALLBACK(RECV_EVENT_BEGIN)) {
		case RECV_RESULT_IGNORE:
			{
				return true;
			}
		case RECV_RESULT_END:
			{
				return false;
			}
		}
	} //if

	Stream  *data_out  = this->m_extern_out ? this->m_extern_out : this->m_default_out;
	intptr_t position  = data_out->Position;
    bool receive_state = this->OnDataRecv(data_out);
	data_out->Position = position;
	return receive_state;
}

//-------------------------------------------------------------------------

bool HttpRequest::OnAutoRedirect(const char *flags)
{
	char location[512];
	if (!__get_http_response_header_value(flags, RLIB_STR_LEN("Location"), RLIB_BUF_SIZE(location)))
	{
		RLIB_SetException((*this->GetLastException()), RLIB_EXCEPTION_ID,
						  _T("missing Location header"));
		return false;
	} //if

	LPURI oldURI  = this->Address; // original address
	this->Address = new Uri(Uri::ProcessUri(location, this->Address)); // set new address

	if (this->m_recv_callback != nullptr) {
		// expose this->Address to user
		__INVOKE_RECV_CALLBACK(RECV_EVENT_REDIRECT_NOTIFY);
	} //if

	this->m_realuri = this->Address; // save redirected address

	bool diffhost = oldURI->Host != this->Address->Host;
	if (diffhost || !this->KeepAlive)
	{
		if (!this->DisConnect(true) || !this->OnConnect()) {
__socket_failed:
			this->Address = oldURI;
			return false;
		} //if
	} //if

	if (!diffhost || Uri::IsSubDomain(this->Address->Host, oldURI->GetTopDomain())) {
		HttpCookie::EnumAll(flags, this->Cookie);
	} else {
		this->Cookie = Nothing;
	} //if

	// force POST->GET, HTTP/1.1 303
	if (flags[2] == '3' && this->Method == _T("POST")){
		this->Method            = _R("GET");
		this->m_request->Length = 0;
	} //if

	if (!this->OnSend()) {
		goto __socket_failed;
	} //if

	// receives response recursively
	auto oldMaxRedirect       = this->MaxAutoRedirections--;
	bool bResult              = this->OnRecv();
	this->MaxAutoRedirections = oldMaxRedirect;

	// deletes temporary address object
	if (this->Address != this->m_realuri) {
		RLIB_Delete(this->Address);
	} //if

	this->Address = oldURI;
	return bResult;
}

//-------------------------------------------------------------------------

static void flush(Stream *data_out, char *buffer, intptr_t nsize, 
				  HTTP_RECV_CALLBACK callback, LPHTTP_CALLBACK_OBJECT params)
{
	if (!callback || __INVOKE_CALLBACK(callback, params, RECV_EVENT_FLUSH) != RECV_RESULT_IGNORE) {
		data_out->Write(buffer, nsize);
	} //if
}

//-------------------------------------------------------------------------

bool HttpRequest::OnDataRecv(IO::Stream *data_out)
{
	intptr_t nSizeToRecv = 0; // total or predictive size we should receive
	intptr_t nReceived   = 0; // size of bytes received 	
	intptr_t nLastReceived;   // the return value of the last time call to Recv
	constexpr intptr_t nTotalSize = RLIB_NETWORK_BUFFER_SIZE * 2; // 16kb default
	char  recvBuffer[nTotalSize];
	char *pHead   = nullptr;    // response body, begins with '\r\n\r\n'
	auto lpsocket = reinterpret_cast<Sockets *>(this->m_socket);

	// receives response headers
	intptr_t nRestSize = nTotalSize - RLIB_SIZEOF(char);
	while ((nLastReceived = lpsocket->Recv(recvBuffer + nReceived, static_cast<int>(nRestSize))) > 0)
	{
		nReceived            += nLastReceived;
		recvBuffer[nReceived] = 0; // make sure that it's ok to treat as null-terminated string
		if ((pHead = strstr(recvBuffer, "\r\n\r\n")) != nullptr) {
			break;
		} //if

		nRestSize = nTotalSize - RLIB_SIZEOF(char) - nReceived;
		if (nRestSize <= 0) {
			RLIB_SetException((*this->GetLastException()), RLIB_EXCEPTION_ID, _T("response headers too long"));
			break;
		} //if
	}

	// process response headers
	__UPDATE_SEND_PARAMES(this);
	if (pHead != nullptr) {
		// response headers received, try to parse it
		if (this->Method == _T("HEAD")) {
__ignore_body:
			nReceived = static_cast<intptr_t>(pHead - recvBuffer + RLIB_COUNTOF_STR("\r\n\r\n"));
			__FLUSH_DATA(recvBuffer, nReceived);
			goto __recv_completed; // always ignore response body if HEAD
		} //if

		// invoke callback if set
		bool skip_headers_output   = false;
		if (this->m_recv_callback != nullptr) {
			switch (__INVOKE_RECV_CALLBACK(RECV_EVENT_RETEND)) {
			case RECV_RESULT_IGNORE:
				{
					// if response body is ignored as well, treat as HEAD request
					if (this->IgnoreResponseBody) {
						goto __ignore_body;
					} //if
					// otherwise, just ignore headers(Location and Content-Length)
					goto __ignore_headers_check;
				}
			case RECV_RESULT_END:
				{
					__FLUSH_DATA(recvBuffer, nReceived);
					goto __force_return;
				}
			case RECV_RESULT_SKIP_HEADERS:
				{
					skip_headers_output = true;
				}
			}
		} //if		
		
		assert(StringStartWith_4_A(pHead, "\r\n\r\n"));

		// process http redirection 301 302 303 307
		if (AutoRedirect && MaxAutoRedirections > 0) {
			auto flags = recvBuffer + RLIB_COUNTOF_STR("HTTP/1.1 ");
			if (flags[0] == '3' && flags[1] == '0' && (flags[2] == '2' || flags[2] == '1' || flags[2] == '3' || flags[2] == '7')) {
				if (!this->m_recv_callback || __INVOKE_RECV_CALLBACK(RECV_EVENT_REDIRECT) != RECV_RESULT_IGNORE) {
					// follow redirection				
					pHead[RLIB_COUNTOF_STR("\r\n\r\n")] = '\0';
					return this->OnAutoRedirect(flags);
				} //if		
			} //if	
		} //if	

		// check if ignore response body
		if (this->IgnoreResponseBody) {
			goto __ignore_body;
		} //if

		// check field Content-Length
		char len_str[16];
		pHead[RLIB_COUNTOF_STR("\r\n")] = '\0';
		if (__get_http_response_header_value(recvBuffer, RLIB_STR_LEN("Content-Length"), RLIB_BUF_SIZE(len_str))) {
			nSizeToRecv = atoi(len_str);
		} //if
		pHead[RLIB_COUNTOF_STR("\r\n")] = '\r';

		if (nSizeToRecv > 0) {
			// add header size
			nSizeToRecv += static_cast<intptr_t>(pHead - recvBuffer + RLIB_COUNTOF_STR("\r\n\r\n"));
		} else {
__ignore_headers_check:
			lpsocket->SetReceiveTimeout(RecvTimeout); // used only the response length is unknown
		} //if

		if (!skip_headers_output) {
			__FLUSH_DATA(recvBuffer, nReceived);
		} else {
			auto headers_size = static_cast<intptr_t>(pHead - recvBuffer + RLIB_COUNTOF_STR("\r\n\r\n"));
			assert(headers_size <= nReceived);
			auto body_size    = nReceived - headers_size;
			if (body_size > 0) {
				__FLUSH_DATA(recvBuffer + headers_size, body_size);
			} //if
		} //if

		// we have obtained the response length, and all data received
		if (nSizeToRecv > 0 && nReceived >= nSizeToRecv) {
			goto __recv_completed;
		} //if
	} else {
		// failed to receive response headers, as a result, any attempt to read it is unpredictable
		return !this->m_recv_callback ? false : __INVOKE_RECV_CALLBACK(RECV_EVENT_INCOMPLETE_HEADER) == RECV_RESULT_IGNORE;
	} //if
	
	// receives response body
	while ((nLastReceived = lpsocket->Recv(recvBuffer, nTotalSize)) > 0)
	{
		nReceived += nLastReceived;

		if (this->m_recv_callback != nullptr) {
			__UPDATE_SEND_PARAMES(this);
			switch (__INVOKE_RECV_CALLBACK(RECV_EVENT_RECEIVED)) {
			case RECV_RESULT_IGNORE:
				{
					nReceived -= nLastReceived;
					continue;
				}
			case RECV_RESULT_END:
				{
					__FLUSH_DATA(recvBuffer, nLastReceived);
					goto __force_return;
				}
			}
		} //if

		__FLUSH_DATA(recvBuffer, nLastReceived);
		if (nSizeToRecv > 0 && nReceived >= nSizeToRecv) {
			break; // all body received
		} //if
	}

	__UPDATE_SEND_PARAMES(this);

	// a socket error occurred when receiving response body
	if (nLastReceived == SOCKET_ERROR) {
		// connection timed out, but we have got data from server
		if (nReceived <= 0 || lpsocket->GetLastException()->HResult != WSAETIMEDOUT) {
			if (this->m_recv_callback != nullptr) {	
				switch (__INVOKE_RECV_CALLBACK(RECV_EVENT_ERROR)) {
				case RECV_RESULT_IGNORE:
					{
						goto __force_return;
					}
				case RECV_RESULT_END:
					{
						return false;
					}
				}
			} //if	
			return false;
		} //if
	} //if

__recv_completed: 
	if (this->m_recv_callback != nullptr) {
		switch (__INVOKE_RECV_CALLBACK(RECV_EVENT_COMPLETED)) {
		case RECV_RESULT_END:
			{
				return false;
			}
		}
	} //if

__force_return:
	if (data_out == this->m_default_out) {
		data_out->Length = nReceived;	
	} //if
	return true;
}

//-------------------------------------------------------------------------

HttpResponse *HttpRequest::GetResponse()
{
    ResponseStream *pResponseStream = this->GetResponseStream();
	if (pResponseStream != nullptr) {
		if (pResponseStream->MaxReadSize <= 0) {
			RLIB_SetException((*this->GetLastException()), RLIB_EXCEPTION_ID, _T("empty response"));
			return nullptr; 
		} //if

		auto pResponse = this->AutomaticDecompression == DecompressionMethod::None ?
			HttpResponse::Create(pResponseStream, this->IgnoreResponseStatus, this->IgnoreResponseHeaders) :
			HttpResponse::Create(pResponseStream, this->AutomaticDecompression, this->IgnoreResponseStatus, this->IgnoreResponseHeaders);
		if (pResponse != nullptr) {
			pResponse->ResponseUri = new Uri(this->m_realuri != nullptr ? *this->m_realuri : *this->Address);
			return pResponse;
		} //if

		// set exception
		RLIB_SetException((*this->GetLastException()), RLIB_EXCEPTION_ID, _T("failed to create HttpResponse"));
	} //if
    return nullptr;
}

//-------------------------------------------------------------------------

ResponseStream *HttpRequest::GetResponseStream(OUT OPTIONAL Stream *extern_out /* = nullptr */)
{
	// check if first request
	if (!this->IsConnected || !this->KeepAlive) {
		// first request
		if (!this->OnConnect()) {
			goto __close_and_return_failed;
		} //if
	} //if

	// send request data
	if (!this->OnSend()) {
        goto __close_and_return_failed;
	} //if

	// init
	if (this->m_realuri != nullptr) {
		delete this->m_realuri;
		this->m_realuri = nullptr;
	} //if
	this->m_extern_out = extern_out;

	// receive response data
	if (this->OnRecv())
    {
		if (!this->KeepAlive) this->DisConnect();

		// default is BufferedStream
		if (this->m_extern_out == nullptr) {
			return this->m_default_out;
		} //if

		// otherwise, we should check the type of extern out
		if (!this->m_extern_out->InheritFromBufferedStream()) {	
			#pragma todo("ignored case of extern out")
			return nullptr;
		} //if

		return static_cast<ResponseStream *>(this->m_extern_out);
	} //if

__close_and_return_failed:
	auto lpsocket = reinterpret_cast<Sockets *>(this->m_socket);
	if (!lpsocket->IsConnectable()) this->DisConnect();
    return nullptr;
}

//-------------------------------------------------------------------------

RequestStream *HttpRequest::GetRequestStream(intptr_t size /* = -1 */)
{
	if (this->m_request == nullptr){
		this->m_request = new RequestStream(size > 0 ? size : RLIB_DEFAULT_BUFFER_SIZE);
	} //if
    return this->m_request;
}

//-------------------------------------------------------------------------

HttpException *HttpRequest::GetLastException()
{
	auto lpsocket = reinterpret_cast<Sockets *>(this->m_socket);
	return static_cast<HttpException *>(lpsocket->GetLastException());
}

//-------------------------------------------------------------------------

bool HttpRequest::DisConnect(bool reuse /* = false */)
{
	auto lpsocket = reinterpret_cast<Sockets *>(this->m_socket);
	if (reuse) {
//		Sockets::Disconnect(*this->m_socket, nullptr);
		lpsocket->~Sockets();
		return this->init_socket();
	} else {
		this->IsConnected = false;
		return lpsocket->Close();
	} //if
}

//-------------------------------------------------------------------------

Sockets *HttpRequest::GetUsingSocket()
{
	return reinterpret_cast<Sockets *>(this->m_socket);
}

//-------------------------------------------------------------------------

void HttpRequest::SetConnCallback(IN OPTIONAL HTTP_CONN_CALLBACK callback,
								  IN OPTIONAL LPHTTP_CALLBACK_OBJECT obj)
{
	this->m_conn_callback       = callback;
	this->m_conn_callback_param = obj;
}

//-------------------------------------------------------------------------

void HttpRequest::SetSendCallback(IN OPTIONAL HTTP_SEND_CALLBACK callback,
								  IN OPTIONAL LPHTTP_CALLBACK_OBJECT obj)
{
	this->m_send_callback       = callback;
	this->m_send_callback_param = obj;
}

//-------------------------------------------------------------------------

void HttpRequest::SetRecvCallback(IN OPTIONAL HTTP_RECV_CALLBACK callback,
								  IN OPTIONAL LPHTTP_CALLBACK_OBJECT obj)
{
	this->m_recv_callback       = callback;
	this->m_recv_callback_param = obj;
}