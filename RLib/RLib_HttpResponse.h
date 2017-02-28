/********************************************************************
	Created:	2015/11/09  19:05
	Filename: 	RLib_HttpResponse.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HTTP_RESPONSE
#define _USE_HTTP_RESPONSE
#include "RLib_Http.h"
#include "RLib_String.h"
#include "RLib_Uri.h"
#include "RLib_WebHeader.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		/// <summary>
		/// Provides a response from a Uniform Resource Identifier (URI)
		/// </summary>
		class WebResponse
		{
		};
		/// <summary>
		/// Provides an HTTP-specific implementation of the WebResponse class
		/// </summary>
		class RLIB_API HttpResponse/* : public WebResponse*/
		{
		private:
			ResponseStream *m_response;
			HttpException   m_error;

		private:		
			HttpResponse() : m_response(nullptr) {}
			void SaveResponseBody(IO::MemoryStream *);
			char *ParseResponseStatus(char *pData, intptr_t nSize, IO::MemoryStream *responseData);
		
		public:
			~HttpResponse();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 获取响应内容的长度
			/// @warning 字段 Content-Length 并不会影响该值
			/// </summary>
			intptr_t ContentLength;
			/// <summary>
			/// 获取响应的状态
			/// 参见http://msdn.microsoft.com/zh-cn/library/system.net.httpstatuscode.aspx
			/// </summary>
			intptr_t StatusCode;
			/// <summary>
			/// Gets the cookies that are associated with this response
			/// </summary>
			RLIB_PROPERTY_GET(String Cookies, GetAllCookies);
			/// <summary>
			/// 获取响应的内容类型
			/// </summary>
			String ContentType;
			/// <summary>
			/// 获取来自服务器的与此响应关联的标头
			/// </summary>
			WebHeaderCollection Headers;
			/// <summary>
			/// 获取响应中使用的 HTTP 协议的版本
			/// </summary>
			String ProtocolVer;
			/// <summary>
			/// 获取与响应一起返回的状态说明
			/// </summary>
			String StatusDescription;
			/// <summary>
			/// 获取响应请求的 Internet 资源的 URI
			/// 如果请求被重定向, 将返回最终地址
			/// </summary>
			LPURI ResponseUri;

		public:
			/// <summary>
			/// Gets the cookies that are associated with this response
			/// </summary>
			String GetAllCookies();
			/// <summary>
			/// 获取与响应一起返回的所有标头的内容
			/// </summary>
			String GetAllResponseHeaders();
			/// <summary>
			/// 获取与响应一起返回的标头的内容
			/// </summary>
			String GetResponseHeader(LPCSTR headerName, intptr_t length = -1);
			/// <summary>
			/// 获取流，该流用于读取来自服务器的响应体
			/// </summary>
			ResponseStream *GetResponseStream();
			/// <summary>
			/// 关闭响应流
			/// </summary>
			RLIB_DEPRECATED void Close() {
				delete this;
			}
			/// <summary>
			/// 获取HttpResponse发生的异常信息
			/// </summary>
			HttpException *GetLastException();

		public:
			/// <summary>
			/// 根据指定的 MemoryStream 实例初始化 HttpResponse 类的新实例
			/// </summary>
			static HttpResponse *Create(IO::MemoryStream *responseData, bool ignoreStatus,
										bool ignoreHeaders);
			/// <summary>
			/// 根据指定的 MemoryStream 实例和 解压缩类型 初始化 HttpResponse 类的新实例
			/// </summary>
			static HttpResponse *Create(IO::MemoryStream *responseData, DecompressionMethod automaticDecompression,
										bool ignoreStatus, bool ignoreHeaders);
		};
	}
}


#endif // !_USE_HTTP_RESPONSE
