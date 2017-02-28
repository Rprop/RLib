/********************************************************************
	Created:	2011/02/18  9:29
	Filename: 	RLib_Http.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HTTP
#define _USE_HTTP
#include "RLib_Net.h"
#include "RLib_Exception.h"
#include "RLib_BufferedStream.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		typedef IO::BufferedStream RequestStream, ResponseStream;
		/// <summary>
		/// Describes an exception that occurred during the processing of HTTP requests
		/// </summary>
		RLIB_INTERNAL_EXCEPTION(HttpException, Exception);		

		enum HTTP_CALLBACK_EVENT
		{
			CONN_EVENT_BEGIN,
			CONN_EVENT_END,

			SEND_EVENT_REQUEST_LINE,
			SEND_EVENT_REQUEST_HEADER,
			SEND_EVENT_REQUEST_BODY,
			SEND_EVENT_END,

			RECV_EVENT_BEGIN,
			RECV_EVENT_RECEIVED, // BODY only
			RECV_EVENT_REDIRECT,
			RECV_EVENT_REDIRECT_NOTIFY,
			RECV_EVENT_FLUSH,
			RECV_EVENT_RETEND, // ignores RECV_RESULT_END return
			RECV_EVENT_COMPLETED, // succeed, or RECV_EVENT_ERROR is ignored
			RECV_EVENT_INCOMPLETE_HEADER, // response headers is not yet received or incomplete, be cautious if you ignore this
			RECV_EVENT_ERROR // exception when receiving response body
		};

		typedef struct HTTP_CALLBACK_OBJECT
		{
		public:
#pragma warning(push)
#pragma warning(disable:4201) // warning C4201: 使用了非标准扩展 : 无名称的结构/联合
			union
			{
				struct // vaild only in Recv callback
				{
					intptr_t  sizeToRecv;
					intptr_t  received;
					intptr_t  lastReceived;
					intptr_t  sizeOfBuffer;
					char     *recvBuffer;
				};
			};
#pragma warning(pop)
			LPVOID pUserData;
		public:
			RLIB_DECLARE_DYNCREATE;
		} *LPHTTP_CALLBACK_OBJECT;

		enum HTTP_CALLBACK_RESULT
		{
			CONN_RESULT_IGNORE,
			CONN_RESULT_RETRY,

			SEND_RESULT_IGNORE,
			SEND_RESULT_CONTINUE,

			RECV_RESULT_IGNORE,
			RECV_RESULT_CONTINUE,
			RECV_RESULT_SKIP_HEADERS, // do not call GetResponse later
			RECV_RESULT_END
		};

		typedef HTTP_CALLBACK_RESULT (*HTTP_CALLBACK)(LPHTTP_CALLBACK_OBJECT, HTTP_CALLBACK_EVENT);
		typedef HTTP_CALLBACK HTTP_CONN_CALLBACK;
		typedef HTTP_CALLBACK HTTP_SEND_CALLBACK;
		typedef HTTP_CALLBACK HTTP_RECV_CALLBACK;
	}
}

#endif