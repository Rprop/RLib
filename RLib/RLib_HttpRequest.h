/********************************************************************
	Created:	2015/11/09  19:01
	Filename: 	RLib_HttpRequest.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HTTP_REQUEST
#define _USE_HTTP_REQUEST
#include "RLib_Utility.h"
#include "RLib_Winsock.h"
#include "RLib_HttpResponse.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Net
	{
		/// <summary>
		/// Makes a request to a Uniform Resource Identifier (URI)
		/// </summary>
		class RLIB_API HttpRequest
		{
		protected:
			RLIB_DELAY_DATA(Sockets, m_socket);

		private:
			HTTP_SEND_CALLBACK     m_conn_callback;
			LPHTTP_CALLBACK_OBJECT m_conn_callback_param;
			HTTP_SEND_CALLBACK     m_send_callback;
			LPHTTP_CALLBACK_OBJECT m_send_callback_param;
			HTTP_RECV_CALLBACK     m_recv_callback;
			LPHTTP_CALLBACK_OBJECT m_recv_callback_param;

		private:
			RequestStream         *m_request;   // 用户写入的请求数据
			Uri                   *m_realuri;   // 服务器响应地址(重定向后)
			IO::Stream            *m_extern_out;
			IO::BufferedStream    *m_default_out;

		private:
			bool init_socket();
			RequestStream *generate_request_packet();

		private:
			bool OnDataRecv(IO::Stream *);
			bool OnAutoRedirect(const char *);
			bool OnConnect();
			bool OnSend();
			bool OnRecv();

		public:
			/// <summary>
			/// 指定构成 HTTP 标头的名称/值对的集合
			/// </summary>
			WebHeaderCollection Headers;
			/// <summary>
			/// 获取或设置 Referer HTTP 标头的值
			/// </summary>
			String Referer;
			/// <summary>
			/// 获取或设置 Content-Type HTTP 标头的值
			/// </summary>
			String ContentType;
			/// <summary>
			/// 获取或设置 User-Agent HTTP 标头的值
			/// </summary>
			String UserAgent;
			/// <summary>
			/// 获取或设置 Cookie HTTP 标头的值
			/// 格式 key0=val0; ...; keyN=valN
			/// @warning 如果发生重定向, 该值可能被覆写
			/// </summary>
			String Cookie;
			/// <summary>
			/// 获取或设置 Content-Length HTTP 标头
			/// </summary>
			intptr_t ContentLength;

		public:
			/// <summary>
			/// 获取或设置所使用的解压缩类型
			/// </summary>
			DecompressionMethod AutomaticDecompression;
			/// <summary>
			/// 获取或设置请求的代理信息
			/// </summary>
			Ipv4Host WebProxy;
			/// <summary>
			/// 获取请求的统一资源标识符 (URI) 
			/// </summary>
			LPURI Address;
			/// <summary>
			/// 获取或设置请求的方法
			/// 如果该值指定为 HEAD, 则将重写属性 IgnoreResponseBody 为 true
			/// </summary>
			String Method;
			/// <summary>
			/// 获取或设置用于请求的 HTTP 版本
			/// </summary>
			String ProtocolVer;
			/// <summary>
			/// 获取或设置连接服务器的超时值（以毫秒为单位）, 默认为8000(8秒)
			/// </summary>
			long ConnectTimeout;
			/// <summary>
			/// 获取或设置发送和接收的超时值（以毫秒为单位）, 默认为8000(8秒)
			/// </summary>
			long Timeout;
			/// <summary>
			/// 获取或设置接收临时超时值（以毫秒为单位）, 默认为1000(1秒)
			/// 该属性仅在未知服务器响应长度时使用以防止长时间阻塞
			/// </summary>
			long RecvTimeout;
			/// <summary>
			/// 获取或设置请求将跟随的重定向的最大数目, 默认值为 3
			/// </summary>
			long MaxAutoRedirections;
			/// <summary>
			/// 获取或设置一个值，该值指示请求是否应跟随重定向响应, 默认值为 true
			/// @warning 如果属性 Method 设置为 HEAD, 此值将被忽略即不会自动重定向
			/// </summary>
			bool AutoRedirect;
			/// <summary>
			/// 获取或设置是否忽略 HTTP(s) 响应体, 默认值为 false
			/// enabling this option means asking for a download but without receiving body,
			/// instead of forcing HttpRequest to do a HEAD request
			/// </summary>
			bool IgnoreResponseBody;
			/// <summary>
			/// 获取或设置是否忽略对 HTTP(s) 响应标头的解析, 默认值为 false
			/// 属性仅对 HttpResponse 对象行为产生影响
			/// </summary>
			bool IgnoreResponseHeaders;
			/// <summary>
			/// 获取或设置是否忽略对 HTTP(s) 响应状态的解析, 默认值为 false
			/// 属性仅对 HttpResponse 对象行为产生影响
			/// </summary>
			bool IgnoreResponseStatus;
			/// <summary>
			/// 获取或设置一个值, 该值指示是否与 Internet 资源建立持久性连接, 默认值为 false 
			/// 该值直接影响 Connection HTTP 标头的值
			/// </summary>
			bool KeepAlive;
			/// <summary>
			/// Gets if connection has been established
			/// </summary>
			bool IsConnected;

		public:
			/// <summary>
			/// 为指定的 URI 方案初始化新的 HttpRequest 实例
			/// </summary>
			HttpRequest(const String &url);
			/// <summary>
			/// 为此 HttpRequest 实例执行必要的清理
			/// </summary>
			~HttpRequest();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 返回来自 Internet 资源的响应
			/// </summary>
			HttpResponse *GetResponse();
			/// <summary>
			/// returns a response to an Internet request
			/// @warning 无需释放且不应与 GetResponse() 方法同时使用
			/// @warning 如果 extern_out 被指定, 那么将直接返回传入的 extern_out(强制转换)
			/// </summary>
			/// <returns>possible nullptr or empty stream</returns>
			ResponseStream *GetResponseStream(OUT OPTIONAL IO::Stream *extern_out = nullptr);
			/// <summary>
			/// Returns a Stream for writing data to the Internet resource
			/// </summary>
			RequestStream *GetRequestStream(intptr_t size = -1);
			/// <summary>
			/// Gets the last exception during http request
			/// </summary>
			HttpException *GetLastException();
			/// <summary>
			/// Forces to disconnect from server
			/// </summary>
			bool DisConnect(bool reuse = false);
			/// <summary>
			/// Gets the associated socket used by HttpRequest
			/// </summary>
			Sockets *GetUsingSocket();

		public:
			/// <summary>
			/// Registers or cancels Connect callback
			/// </summary>
			void SetConnCallback(IN OPTIONAL HTTP_CONN_CALLBACK, IN OPTIONAL LPHTTP_CALLBACK_OBJECT);
			/// <summary>
			/// Registers or cancels Send callback
			/// </summary>
			void SetSendCallback(IN OPTIONAL HTTP_SEND_CALLBACK, IN OPTIONAL LPHTTP_CALLBACK_OBJECT);
			/// <summary>
			/// Registers or cancels Recv callback
			/// </summary>
			void SetRecvCallback(IN OPTIONAL HTTP_RECV_CALLBACK, IN OPTIONAL LPHTTP_CALLBACK_OBJECT);
		};
	}
}
#endif // !_USE_HTTP_REQUEST