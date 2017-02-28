/********************************************************************
	Created:	2011/12/08  19:28
	Filename: 	RLib_Winsock.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_WINSOCK
#define _USE_WINSOCK

#include "RLib_Net.h"
#include "RLib_Exception.h"
#include <winsock2.h>

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// The namespaces contain classes that provide a simple programming interface for a number of network protocols, 
	/// programmatically access and update configuration settings for the System.Net namespaces, define cache policies for web resources, 
	/// access network traffic data and network address information, and access peer-to-peer networking functionality. 
	/// </summary>
	namespace Net
	{
#ifdef UNICODE
		typedef ADDRINFOW AddrInfo;
#else
		typedef ADDRINFOA AddrInfo;
#endif
		/// <summary>
		/// The namespace provides a managed implementation of the Windows Sockets (Winsock) interface for developers
		/// who need to tightly control access to the network.
		/// </summary>
		class RLIB_API Sockets
		{
		public:
			enum SSLMode
			{
				SSL_NONE,
				SSL_CLIENT,
				SSL_SERVER
			};

		private:
			int recv(LPVOID buf, int size);
			int send(LPCVOID data, int size);
			static int ssl_recv(void *ctx, unsigned char *buf, size_t len);
			static int ssl_send(void *ctx, const unsigned char *buf, size_t len);

		protected:
			RLIB_SOCKET      m_socket;
			sockaddr_in      m_addr;		
			RLIB_SSL_CONTEXT m_ssl;
			Exception        m_error;

		public:
			Sockets(RLIB_SOCKET s = RLIB_INVALID_SOCKET);
			Sockets(SSLMode ssl_mode, RLIB_SOCKET s = RLIB_INVALID_SOCKET);
			Sockets(int protocol, int type, int af = AF_INET);
			~Sockets();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 获取 SOCKET 句柄
			/// </summary>
			operator RLIB_SOCKET();
			/// <summary>
			/// 获取 SOCKET 句柄
			/// </summary>
			RLIB_SOCKET GetNativeHandle();
			/// <summary>
			/// 获取 SOCKET 句柄
			/// </summary>
			RLIB_PROPERTY_GET(RLIB_SOCKET NativeHandle, GetNativeHandle);
			/// <summary>
			/// Sets the timeout, in milliseconds, for blocking send calls.
			/// </summary>
			bool SetSendTimeout(int timeout);
			/// <summary>
			/// Get value of the SO_SNDBUF option for this Socket, 
			/// that is the buffer size used by the platform for output on this Socket
			/// </summary>
			int GetSendBufferSize();
			/// <summary>
			/// Sets the timeout, in milliseconds, for blocking receive calls.
			/// </summary>
			bool SetReceiveTimeout(int timeout);
			/// <summary>
			/// Gets the value of the SO_RCVBUF option for this Socket, 
			/// that is the buffer size used by the platform for input on this Socket
			/// </summary>
			int GetReceiveBufferSize();
			/// <summary>
			/// Closes an existing socket
			/// </summary>
			bool Close();
			/// <summary>
			/// 当 Connect 失败时，返回一个值指示能否继续尝试建立连接
			/// </summary>
			bool IsConnectable();
			/// <summary>
			/// Establishes a connection to a specified socket
			/// </summary>
			/// <param name="port">A 16-bit number in TCP/IP network byte order</param>
			bool Connect(unsigned long addr, unsigned short port,
						 const timeval *lptimeout = nullptr);
			/// <summary>
			/// Establishes a connection to a specified socket
			/// </summary>
			/// <param name="host_port">A 16-bit number in host byte order</param>
			bool Connect(LPCTSTR host, int host_len, unsigned short host_port,
						 const timeval *lptimeout = nullptr);
			/// <summary>
			/// Sends data on a connected socket
			/// </summary>
			int Send(LPCVOID data, int size);
			/// <summary>
			/// Sends data to a specific destination
			/// </summary>
			int SendTo(LPCVOID data, int size, unsigned long addr, unsigned short port, int flags = 0);
			/// <summary>
			/// Sends data to a specific destination
			/// </summary>
			int SendTo(LPCVOID data, int size, LPCTSTR host, int host_len, unsigned short host_port, int flags = 0);
			/// <summary>
			/// Receives data from a connected or bound socket
			/// </summary>
			int Recv(LPVOID buf, int size);  
			/// <summary>
			/// Receives a datagram and stores the source address
			/// </summary>
			int RecvFrom(LPVOID buf, int size, sockaddr_in *addr = NULL, int *addrlen = nullptr, int flags = 0);
			/// <summary>
			/// associates a local address with a socket
			/// </summary>
			int Bind(ULONG addr, unsigned short port);  
			/// <summary>
			/// associates a local address with a socket
			/// 方法绑定到所有地址上
			/// </summary>
			int Bind(unsigned short host_port);  
			/// <summary>
			/// Places a socket a state where it is listening for an incoming connection
			/// </summary>
			int Listen(int backlog = RLIB_SOMAXCONN);  
			/// <summary>
			/// Permits an incoming connection attempt on a socket
			/// An optional pointer to a buffer that receives the address of the connecting entity,
			/// as known to the communications layer.
			/// </summary>
			Sockets *Accept(sockaddr_in *addr = NULL, int *addrlen = nullptr);  
			/// <summary>
			/// An extended version of Accept
			/// Permits an incoming connection attempt on a socket
			/// An optional pointer to a buffer that receives the address of the connecting entity,
			/// as known to the communications layer.
			/// </summary>
			Sockets *Accept(const timeval &_timeout, sockaddr_in *addr = NULL,
							int *addrlen = nullptr);
			/// <summary>
			/// Disables sends or receives on a socket
			/// </summary>
			int Shutdown(int how = RLIB_SD_BOTH);
			/// <summary>
			/// 获取 Wincock 发生的异常信息
			/// 该方法仅提供于实例对象
			/// </summary>
			Exception *GetLastException(); 
			/// <summary>
			/// 获取 Wincock 发生的异常信息, 随后可通过 GetLastException 方法访问
			/// </summary>
			void SaveLastException();
			/// <summary>
			/// Retrieves a socket option, or INT_MAX if failed
			/// </summary>
			int GetOption(_In_ int optname);
			/// <summary>
			/// Retrieves a socket option
			/// </summary>
			int GetOption(_In_ int optname, _Out_ void *optval, _Inout_ int *optlen);
			/// <summary>
			/// Sets a socket option
			/// </summary>
			int SetOption(_In_ int optname, _In_ int val);
			/// <summary>
			/// Sets a socket option
			/// </summary>
			int SetOption(_In_ int optname, _In_ const void *optval, _In_ int optlen);

		public:
			/// <summary>
			/// Parses host and port into Ipv4Host structure
			/// </summary>
			/// <param name="host_port">A 16-bit number in host byte order</param>
			static Ipv4Host ParseIpv4Host(LPCTSTR host, int host_len, unsigned short host_port);
			/// <summary>
			/// The function determines the status of one or more sockets, 
			/// waiting if necessary, to perform synchronous I/O
			/// </summary>
			static int Select(_In_ int nfds, 
							  _Inout_opt_ fd_set FAR *readfds, _Inout_opt_ fd_set FAR *writefds,
							  _Inout_opt_ fd_set FAR *exceptfds, _In_opt_ const struct timeval FAR *timeout);
			/// <summary>
			/// The function controls the I/O mode of a socket
			/// </summary>
			static int IoctlSocket(_In_ RLIB_SOCKET s, _In_ long cmd, _Inout_ u_long FAR *argp);
			/// <summary>
			/// The function closes a connection on a socket, and allows the socket handle to be reused.
			/// </summary>
			static bool Disconnect(_In_ SOCKET s, _In_ LPOVERLAPPED lpOverlapped);
			/// <summary>
			/// 获取本地计算机名称
			/// </summary>
			static bool GetLocalHost(_Out_ char *host, _In_ int hostlen);
			/// <summary>
			/// 获取本地计算机地址
			/// </summary>
			static bool GetLocalIpAddress(_In_ const TCHAR *host, _Out_ AddrInfo **addrinfo);
			/// <summary>
			/// 释放由GetLocalIpAddress获得的addrinfo
			/// </summary>
			static void FreeLocalIpAddress(AddrInfo **addrinfo);
			/// <summary>
			/// Converts a string containing an (Ipv4) Internet Protocol dotted address into a proper address for the in_addr structure
			/// </summary>
			static unsigned long Ipv4StringToAddress(LPCTSTR cp);
			/// <summary>
			/// Converts a string containing an (Ipv4) Internet Protocol dotted address into a proper address for the in_addr structure
			/// </summary>
			RLIB_DEPRECATED static unsigned long Ipv4StringToAddress(LPCTSTR cp, int length);
			/// <summary>
			/// Converts a string containing an host name into a proper address for the in_addr structure
			/// </summary>
			/// <returns>INADDR_NONE if failed</returns>
			static unsigned long Sockets::HostStringToAddress(LPCTSTR host);
			/// <summary>
			/// Converts a string containing an host name into a proper address for the in_addr structure
			/// </summary>
			/// <returns>INADDR_NONE if failed</returns>
			RLIB_DEPRECATED static unsigned long Sockets::HostStringToAddress(LPCTSTR host, int host_len);
			/// <summary>
			/// Converts an (IPv4) Internet network address into a string in Internet standard dotted format
			/// </summary>
			/// <param name="addr">A pointer to an IN_ADDR structure with the IPv4 address to convert</param>
			/// <param name="pIpv4StringBuf">A pointer to a buffer in which to store the NULL-terminated string representation of the IP address.
			/// this buffer should be large enough to hold at least 16 characters</param>
			/// <param name="ipv4StringBufSize">On input, the length, in characters, of the buffer pointed to by the pStringBuf parameter</param>
			static bool Ipv4AddressToString(unsigned long addr, LPTSTR pIpv4StringBuf, size_t ipv4StringBufSize);
			/// <summary>
			/// 当首次使用 Sockets 的时候调用此方法初始化网络模块内存
			/// </summary>
			/// <returns>
			/// If successful, the function returns zero. Otherwise, it returns one of the error codes
			/// </returns>
			static int InitializeNetworkEntry();
			/// <summary>
			/// 当不再使用 Sockets 的时候调用此方法释放网络模块内存
			/// </summary>
			static void Dispose();
			/// <summary>
			/// 获取最近一次错误Id
			/// </summary>
			static int GetLastErrorId();
		};
	}
}
#endif