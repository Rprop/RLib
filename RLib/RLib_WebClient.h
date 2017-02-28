/********************************************************************
	Created:	2012/04/21  14:47
	Filename: 	RLib_WebClient.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_WEBCLIENT
#define _USE_WEBCLIENT
#include "RLib_HttpRequest.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		/// <summary>
		/// 提供用于将数据发送到由 URI 标识的资源及从这样的资源接收数据的常用方法
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE WebClient
		{
		public:
			/// <summary>
			/// 为指定资源返回一个 HttpRequest 对象, delete 以释放对象
			/// </summary>
			static HttpRequest *GetHttpRequest(const String &Url);
			/// <summary>
			/// 获取具有指定 URI 的原始响应流
			/// </summary>
			static ResponseStream *GetRawResponseStream(const String &Url, HttpRequest *pRequest IN = nullptr);
			/// <summary>
			/// 获取具有指定 URI 的资源流
			/// 如果 ppResponse != null, 则返回的 ResponseStream 无须二次释放
			/// </summary>
			static ResponseStream *GetResponseStream(const String &Url, HttpRequest *pRequest IN = nullptr,
													 HttpResponse **ppResponse OUT = nullptr);
			/// <summary>
			/// 获取具有指定 URI 的文本
			/// </summary>
			static String GetResponseText(const String &Url, HttpRequest *pRequest IN = nullptr,
										  HttpResponse **ppResponse OUT = nullptr);
			/// <summary>
			/// 获取具有指定 URI 的原始响应流
			/// </summary>
			static ResponseStream *PostRawResponseStream(const String &Url, const String &Data,
														 HttpRequest *pRequest IN = nullptr);
			/// <summary>
			/// 获取具有指定 URI 的资源流
			/// 如果 ppResponse != null, 则返回的 ResponseStream 无须二次释放
			/// </summary>
			static ResponseStream *PostResponseStream(const String &Url, const String &Data,
													  HttpRequest *pRequest IN = nullptr,
													  HttpResponse **ppResponse OUT = nullptr);
			/// <summary>
			/// 获取具有指定 URI 的文本
			/// </summary>
			static String PostResponseText(const String &Url, const String &Data,
										   HttpRequest *pRequest IN = nullptr,
										   HttpResponse **ppResponse OUT = nullptr);
			/// <summary>
			/// 将文本写入请求流(body), 并设置方法为 POST
			/// </summary>
			static void WriteRequestStream(HttpRequest *pRequest, const String &Data);
			/// <summary>
			/// 将具有指定 URI 的资源下载到本地文件
			/// </summary>
			static bool DownloadFile(const String &Url, const String &FilePath,
									 HttpRequest *pRequest IN = nullptr, HttpResponse **ppResponse OUT = nullptr);
		};
	}
}
//////////////////////////////////////////////////////////////////////////
#endif // _USE_WEBCLIENT