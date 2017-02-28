/********************************************************************
Created:	2011/02/17  15:15
Filename: 	RLib_Winsock.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_Winsock.h"
#include "RLib_Threading.h"
#include "RLib_StringConvHelper.h"
#include "RLib_Interlocked.h"
#include "support/polarssl/library/polarssl/ssl.h"
#include "support/polarssl/library/polarssl/entropy.h"
#include "support/polarssl/library/polarssl/ctr_drbg.h"
#include "support/polarssl/library/polarssl/certs.h"
#include "RLib_File.h"
// POLARSSL_CERTS_C
// POLARSSL_PEM_PARSE_C

#pragma warning(push)
#pragma warning(disable:4365) // signed/unsigned mismatch
#pragma warning(disable:4574) // 'identifier' is defined to be '0': did you mean to use '#if identifier' ?
#include <Ws2tcpip.h>
#pragma warning(pop)
#pragma comment(lib, "Ws2_32.lib")

#if (WINVER > _WIN32_WINNT_WIN7)
# include <minwindef.h>
#endif

#if (WINVER >= _WIN32_WINNT_VISTA)
# include <mswsock.h> // for Disconnect
#endif

using namespace System::Net;

//-------------------------------------------------------------------------

static volatile long __is_winsock_initialized = 0;
static char __srv_key[] = {
	"-----BEGIN EC PRIVATE KEY-----\r\n"
	"MHcCAQEEIPEqEyB2AnCoPL/9U/YDHvdqXYbIogTywwyp6/UfDw6noAoGCCqGSM49\r\n"
	"AwEHoUQDQgAEN8xW2XYJHlpyPsdZLf8gbu58+QaRdNCtFLX3aCJZYpJO5QDYIxH/\r\n"
	"6i/SNF1dFr2KiMJrdw1VzYoqDvoByLTt/w==\r\n"
	"-----END EC PRIVATE KEY-----\r\n"};
#define  __ca_key __srv_key

//-------------------------------------------------------------------------

typedef struct rlib_ssl_info
{
public:
	entropy_context  entropy;
	ctr_drbg_context ctr_drbg;
	ssl_context      ssl;
public:
	rlib_ssl_info(){ memset(this, 0, sizeof(*this)); }
	RLIB_DECLARE_DYNCREATE;
} rlib_ssl_info;

#define this_ssl          static_cast<rlib_ssl_info *>(this->m_ssl)
#define DFL_SUBJECT_NAME  "CN=rlib.cn,O=XYApp,C=CN,OU=Organization and domain(s) authenticated by iTrus China"
#define DFL_ISSUER_NAME   "CN=Symantec Class 3 Secure Server CA - G4,OU=Symantec Trust Network,O=Symantec Corporation,C=US"
#define DFL_NOT_BEFORE    "20010101000000"
#define DFL_NOT_AFTER     "20301231235959"
#define DFL_SERIAL		  "36e4d8b5004a13a89d4035e68a28b" RLIB_STRINGIZE(RLIB_COMPILE_LINE)
#define DFL_MAX_PATHLEN   -1

//-------------------------------------------------------------------------

int Sockets::ssl_recv(void *ctx, unsigned char *buf, size_t len)
{
	return static_cast<Sockets *>(ctx)->recv(buf, static_cast<int>(len));
}

//-------------------------------------------------------------------------

int Sockets::ssl_send(void *ctx, const unsigned char *buf, size_t len)
{
	return static_cast<Sockets *>(ctx)->send(buf, static_cast<int>(len));
}

//-------------------------------------------------------------------------

static void rlib_format_ssl_error(int errcode, LPTSTR errout)
{
	#define COPY_TO_OUT(a) memcpy(errout, a, sizeof(a)) 
	switch (errcode)
	{
	case POLARSSL_ERR_SSL_FEATURE_UNAVAILABLE:
		COPY_TO_OUT(_T("The requested feature is not available."));
		break;

	case POLARSSL_ERR_SSL_BAD_INPUT_DATA:
		COPY_TO_OUT(_T("Bad input parameters to function."));
		break;

	case POLARSSL_ERR_SSL_INVALID_MAC:
		COPY_TO_OUT(_T("Verification of the message MAC failed."));
		break;

	case POLARSSL_ERR_SSL_INVALID_RECORD:
		COPY_TO_OUT(_T("An invalid SSL record was received."));
		break;

	case POLARSSL_ERR_SSL_CONN_EOF:
		COPY_TO_OUT(_T("The connection indicated an EOF."));
		break;

	case POLARSSL_ERR_SSL_UNKNOWN_CIPHER:
		COPY_TO_OUT(_T("An unknown cipher was received."));
		break;

	case POLARSSL_ERR_SSL_NO_CIPHER_CHOSEN:
		COPY_TO_OUT(_T("The server has no ciphersuites in common with the client."));
		break;

	case POLARSSL_ERR_SSL_NO_RNG:
		COPY_TO_OUT(_T("No RNG was provided to the SSL module."));
		break;

	case POLARSSL_ERR_SSL_NO_CLIENT_CERTIFICATE:
		COPY_TO_OUT(_T("No client certification received from the client, but required by the authentication mode."));
		break;

	case POLARSSL_ERR_SSL_CERTIFICATE_TOO_LARGE:
		COPY_TO_OUT(_T("Our own certificate(s) is/are too large to send in an SSL message."));
		break;

	case POLARSSL_ERR_SSL_CERTIFICATE_REQUIRED:
		COPY_TO_OUT(_T("The own certificate is not set, but needed by the server."));
		break;

	case POLARSSL_ERR_SSL_PRIVATE_KEY_REQUIRED:
		COPY_TO_OUT(_T("The own private key or pre-shared key is not set, but needed."));
		break;

	case POLARSSL_ERR_SSL_CA_CHAIN_REQUIRED:
		COPY_TO_OUT(_T("No CA Chain is set, but required to operate."));
		break;

	case POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE:
		COPY_TO_OUT(_T("An unexpected message was received from our peer."));
		break;

	case POLARSSL_ERR_SSL_FATAL_ALERT_MESSAGE:
		COPY_TO_OUT(_T("A fatal alert message was received from our peer."));
		break;

	case POLARSSL_ERR_SSL_PEER_VERIFY_FAILED:
		COPY_TO_OUT(_T("Verification of our peer failed."));
		break;

	case POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY:
		COPY_TO_OUT(_T("The peer notified us that the connection is going to be closed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CLIENT_HELLO:
		COPY_TO_OUT(_T("Processing of the ClientHello handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO:
		COPY_TO_OUT(_T("Processing of the ServerHello handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE:
		COPY_TO_OUT(_T("Processing of the Certificate handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST:
		COPY_TO_OUT(_T("Processing of the CertificateRequest handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE:
		COPY_TO_OUT(_T("Processing of the ServerKeyExchange handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO_DONE:
		COPY_TO_OUT(_T("Processing of the ServerHelloDone handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE:
		COPY_TO_OUT(_T("Processing of the ClientKeyExchange handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP:
		COPY_TO_OUT(_T("Processing of the ClientKeyExchange handshake message failed in DHM / ECDH Read Public."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS:
		COPY_TO_OUT(_T("Processing of the ClientKeyExchange handshake message failed in DHM / ECDH Calculate Secret."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY:
		COPY_TO_OUT(_T("Processing of the CertificateVerify handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC:
		COPY_TO_OUT(_T("Processing of the ChangeCipherSpec handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_FINISHED:
		COPY_TO_OUT(_T("Processing of the Finished handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_MALLOC_FAILED:
		COPY_TO_OUT(_T("Memory allocation failed."));
		break;

	case POLARSSL_ERR_SSL_HW_ACCEL_FAILED:
		COPY_TO_OUT(_T("Hardware acceleration function returned with error."));
		break;

	case POLARSSL_ERR_SSL_HW_ACCEL_FALLTHROUGH:
		COPY_TO_OUT(_T("Hardware acceleration function skipped / left alone data."));
		break;

	case POLARSSL_ERR_SSL_COMPRESSION_FAILED:
		COPY_TO_OUT(_T("Processing of the compression / decompression failed."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_PROTOCOL_VERSION:
		COPY_TO_OUT(_T("Handshake protocol not within min/max boundaries."));
		break;

	case POLARSSL_ERR_SSL_BAD_HS_NEW_SESSION_TICKET:
		COPY_TO_OUT(_T("Processing of the NewSessionTicket handshake message failed."));
		break;

	case POLARSSL_ERR_SSL_SESSION_TICKET_EXPIRED:
		COPY_TO_OUT(_T("Session ticket has expired."));
		break;

	case POLARSSL_ERR_SSL_PK_TYPE_MISMATCH:
		COPY_TO_OUT(_T("Public key type mismatch (eg, asked for RSA key exchange and presented EC key)."));
		break;

	case POLARSSL_ERR_SSL_UNKNOWN_IDENTITY:
		COPY_TO_OUT(_T("Unknown identity received (eg, PSK identity)."));
		break;

	case POLARSSL_ERR_SSL_INTERNAL_ERROR:
		COPY_TO_OUT(_T("Internal error (eg, unexpected failure in lower-level module)."));
		break;

	case POLARSSL_ERR_SSL_COUNTER_WRAPPING:
		COPY_TO_OUT(_T("A counter would wrap (eg, too many messages exchanged)."));
		break;

	default:
		COPY_TO_OUT(_T("Successful completed Or Unknown Error."));
		break;
	}
}

//-------------------------------------------------------------------------

static unsigned char *rlib_generate_server_cert(ctr_drbg_context *lpctr_drbg,
												unsigned char *lpbuf, size_t *lpsize)
{
	x509write_cert crt;
	x509write_crt_init(&crt);
	mpi serial;
	mpi_init(&serial);
	pk_context subject_key, issuer_key;
	pk_init(&subject_key);
	pk_init(&issuer_key);

	// parse serial number to MPI
	if (mpi_read_string(&serial, 16, DFL_SERIAL) != 0 || 
		x509write_crt_set_serial(&crt, &serial)  != 0) {
		goto __finally_clean;
	}
	int ret = x509write_crt_set_validity(&crt, DFL_NOT_BEFORE, DFL_NOT_AFTER);
	if (ret != 0) {
		goto __finally_clean;
	}
	ret = x509write_crt_set_basic_constraints(&crt, 0/*NOT_CA*/, DFL_MAX_PATHLEN);
	if (ret != 0) {
		goto __finally_clean;
	}
	ret = x509write_crt_set_key_usage(&crt, 
									  KU_DIGITAL_SIGNATURE | KU_KEY_ENCIPHERMENT | KU_DATA_ENCIPHERMENT);
	if (ret != 0) {
		goto __finally_clean;
	}
// 	ret = x509write_crt_set_ns_cert_type(&crt, NS_CERT_TYPE_SSL_SERVER);
// 	if (ret != 0) {
// 		goto __finally_clean;
// 	}
	// selfsign disabled
	if ((ret = x509write_crt_set_subject_name(&crt, DFL_SUBJECT_NAME)) != 0) {
		goto __finally_clean;
	}
	if ((ret = x509write_crt_set_issuer_name(&crt, DFL_ISSUER_NAME)) != 0) {
		goto __finally_clean;
	}
	// loading the subject key ... 
	ret = pk_parse_key(&subject_key, reinterpret_cast<const unsigned char *>(__srv_key),
					   RLIB_COUNTOF_STR(__srv_key), NULL, 0);
	if (ret != 0) {
		goto __finally_clean;
	}
	// loading the issuer key ... 
	ret = pk_parse_key(&issuer_key, reinterpret_cast<const unsigned char *>(__ca_key),
					   RLIB_COUNTOF_STR(__ca_key), NULL, 0);
	if (ret != 0) {
		goto __finally_clean;
	}
	x509write_crt_set_subject_key(&crt, &subject_key);
	x509write_crt_set_issuer_key(&crt, &issuer_key);
	x509write_crt_set_md_alg(&crt, POLARSSL_MD_SHA256);

	// generate certificate
	ret = x509write_crt_der(&crt, lpbuf, *lpsize,
							ctr_drbg_random, lpctr_drbg);
	if (ret > 0) {
		lpbuf += (*lpsize - ret);
		*lpsize = static_cast<size_t>(ret);
		// debug only
// 		auto p = IO::File::Create(_T("srv.cer"), IO::FileMode::CreateNew);
// 		p->Write(lpbuf, *lpsize);
// 		delete p;
	} else {
		*lpsize = 0;
	} //if

__finally_clean:
	pk_free(&issuer_key);
	pk_free(&subject_key);
	mpi_free(&serial);
	x509write_crt_free(&crt);

	return lpbuf;
}

//-------------------------------------------------------------------------

static unsigned char *rlib_generate_selfsign_ca_cert(ctr_drbg_context *lpctr_drbg,
													 unsigned char *lpbuf, size_t *lpsize)
{
	x509write_cert crt;
	x509write_crt_init(&crt);
	mpi serial;
	mpi_init(&serial);
	pk_context issuer_key;
	pk_init(&issuer_key);

	int ret = 0;

	// parse serial number to MPI
	if (mpi_read_string(&serial, 16, DFL_SERIAL) != 0 || 
		x509write_crt_set_serial(&crt, &serial)  != 0) {
		goto __finally_clean;
	}
	ret = x509write_crt_set_validity(&crt, DFL_NOT_BEFORE, DFL_NOT_AFTER);
	if (ret != 0) {
		goto __finally_clean;
	}
	ret = x509write_crt_set_basic_constraints(&crt, 1/*IS_CA*/, DFL_MAX_PATHLEN);
	if (ret != 0) {
		goto __finally_clean;
	}
	ret = x509write_crt_set_key_usage(&crt, 
									  KU_DIGITAL_SIGNATURE | KU_KEY_CERT_SIGN | KU_CRL_SIGN);
	if (ret != 0) {
		goto __finally_clean;
	}
// 	ret = x509write_crt_set_ns_cert_type(&crt, NS_CERT_TYPE_SSL_CA);
// 	if (ret != 0) {
// 		goto __finally_clean;
// 	}
	// selfsign mode, issuer_name and issuer_key are required,
	// while issuer_crt and subject_* are ignored
	if ((ret = x509write_crt_set_subject_name(&crt, DFL_ISSUER_NAME)) != 0) {
		goto __finally_clean;
	}
	if ((ret = x509write_crt_set_issuer_name(&crt, DFL_ISSUER_NAME)) != 0) {
		goto __finally_clean;
	}
	// loading the issuer key ... 
	ret = pk_parse_key(&issuer_key, reinterpret_cast<const unsigned char *>(__ca_key),
					   RLIB_COUNTOF_STR(__ca_key), NULL, 0);
	if (ret != 0) {
		goto __finally_clean;
	}
	x509write_crt_set_subject_key(&crt, &issuer_key);
	x509write_crt_set_issuer_key(&crt, &issuer_key);
	x509write_crt_set_md_alg(&crt, POLARSSL_MD_SHA256);

	// generate certificate
	ret = x509write_crt_der(&crt, lpbuf, *lpsize,
							ctr_drbg_random, lpctr_drbg);
	if (ret > 0) {
		lpbuf += (*lpsize - ret);
		*lpsize = static_cast<size_t>(ret);
// 		auto p = IO::File::Create(_T("ca.cer"), IO::FileMode::CreateNew);
// 		p->Write(lpbuf, *lpsize);
// 		delete p;
	} else {
		*lpsize = 0;
	} //if

__finally_clean:
	pk_free(&issuer_key);
	mpi_free(&serial);
	x509write_crt_free(&crt);

	return lpbuf;
}

//-------------------------------------------------------------------------

Sockets::Sockets(RLIB_SOCKET s /* = RLIB_INVALID_SOCKET */)
{
	if (s == RLIB_INVALID_SOCKET) {
		RLIB_InitClass(this, Sockets(IPPROTO_TCP, SOCK_STREAM));
	} else {
		this->m_socket = s;
		this->m_addr   = { static_cast<ADDRESS_FAMILY>(AF_INET) };
		this->m_ssl    = nullptr;
	} //if
}

//-------------------------------------------------------------------------

Sockets::Sockets(SSLMode ssl_mode, RLIB_SOCKET s /* = RLIB_INVALID_SOCKET */)
{
	RLIB_InitClass(this, Sockets(s));
	if (ssl_mode == SSL_NONE) return;

	// init ssl context
	this->m_ssl = new rlib_ssl_info;
	if (this_ssl == nullptr) return;

	// seeding the random number generator...
	entropy_init(&this_ssl->entropy);

	int ret = ctr_drbg_init(&this_ssl->ctr_drbg, entropy_func,
							&this_ssl->entropy, reinterpret_cast<const unsigned char *>("ssl_rlib"),
							RLIB_COUNTOF_STR("ssl_rlib"));
	if (ret != 0) {
		RLIB_SetException(this->m_error, ret, _T("ctr_drbg_init failed."));
		return;
	} //if

	  // setting up the SSL / TLS structure...
	if ((ret = ssl_init(&this_ssl->ssl)) != 0) {
		RLIB_SetException(this->m_error, ret, _T("ssl_init failed."));
		return;
	} //if

	ssl_set_authmode(&this_ssl->ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&this_ssl->ssl, ctr_drbg_random, &this_ssl->ctr_drbg);
	ssl_set_bio(&this_ssl->ssl, ssl_recv, this, ssl_send, this);

	if (ssl_mode == SSL_CLIENT) {
		ssl_set_endpoint(&this_ssl->ssl, SSL_IS_CLIENT);
	} else {
		ssl_set_endpoint(&this_ssl->ssl, SSL_IS_SERVER);
		/*
		x509_crt srvcert_chain;
		x509_crt_init(&srvcert_chain);

		unsigned char crt_data[1024];
		size_t crt_size = sizeof(crt_data);
		ret = x509_crt_parse_der(&srvcert_chain,
								 rlib_generate_server_cert(&this_ssl->ctr_drbg, crt_data, &crt_size),
								 crt_size);
		if (ret != 0) {
			RLIB_SetException(this->m_error, ret, _T("x509_crt_parse_der failed."));
			goto __finally_clean;
		} //if
		  //			x509_crt_info(reinterpret_cast<char *>(crt_data), sizeof(crt_data), "", &srvcert_chain);
		crt_size = sizeof(crt_data);
		ret = x509_crt_parse_der(&srvcert_chain,
								 rlib_generate_selfsign_ca_cert(&this_ssl->ctr_drbg, crt_data, &crt_size),
								 crt_size);
		if (ret != 0) {
			RLIB_SetException(this->m_error, ret, _T("x509_crt_parse failed."));
			goto __finally_clean;
		} //if

		pk_context srv_key;
		pk_init(&srv_key);
		ret = pk_parse_key(&srv_key, reinterpret_cast<const unsigned char *>(__srv_key),
						   RLIB_COUNTOF_STR(__srv_key), NULL, 0);
		if (ret != 0) {
			RLIB_SetException(this->m_error, ret, _T("pk_parse_key failed."));
			goto __finally_clean;
		}

		ssl_set_ca_chain(&this_ssl->ssl, srvcert_chain.next, NULL, NULL);
		if ((ret = ssl_set_own_cert(&this_ssl->ssl, &srvcert_chain, &srv_key)) != 0) {
			RLIB_SetException(this->m_error, ret, _T("pk_parse_key failed."));
			goto __finally_clean;
		}

		while ((ret = ssl_handshake(&this_ssl->ssl)) != 0) {
			if (ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE) {
				if (ret == -1) {
					RLIB_SetException(this->m_error, ret, _T("ssl_handshake failed."));
				} else {
					this->m_error.HResult = ret;
					rlib_format_ssl_error(this->m_error.HResult, this->m_error.Message);
					RLIB_SetExceptionInfo(this->m_error);
				} //if
				break;
			}
		}

__finally_clean:
		x509_crt_free(&srvcert_chain);
		*/
	} //if
}

//-------------------------------------------------------------------------

Sockets::Sockets(int protocol, int type, int af /* = AF_INET */)
{
	if ((this->m_error.HResult = this->InitializeNetworkEntry()) != STATUS_SUCCESS) {
		Exception::FormatErrorMessage(this->m_error.Message,
									  RLIB_COUNTOF(this->m_error.Message),
									  this->m_error.ErrorNo);
		RLIB_SetExceptionInfo(this->m_error);
		return;
	} //if
	if ((this->m_socket = socket(af, type, protocol)) == INVALID_SOCKET) {
		this->SaveLastException();
		RLIB_SetExceptionInfo(this->m_error);
	} //if

	this->m_addr = { static_cast<ADDRESS_FAMILY>(af) };
	this->m_ssl  = nullptr;
}
//-------------------------------------------------------------------------

Sockets::~Sockets()
{
    this->Close();
}

//-------------------------------------------------------------------------

int Sockets::InitializeNetworkEntry()
{
	int ret = STATUS_SUCCESS;
	if (__is_winsock_initialized == 0) {
		WSADATA wsaData;
		// If successful, the WSAStartup function returns zero
		if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) == STATUS_SUCCESS) {
			Threading::Interlocked::Increment(&__is_winsock_initialized);
		} //if
	} //if
	return ret;
}

//-------------------------------------------------------------------------

void Sockets::Dispose()
{
    if (__is_winsock_initialized != 0 && ::WSACleanup() == 0) {
        Threading::Interlocked::Exchange(&__is_winsock_initialized, 0L);
    } //if
}

//-------------------------------------------------------------------------

Sockets::operator SOCKET()
{
	return this->NativeHandle;
}

//-------------------------------------------------------------------------

RLIB_SOCKET Sockets::GetNativeHandle()
{
	return this->m_socket;
}

//-------------------------------------------------------------------------

bool Sockets::Close()
{
	if (this_ssl != nullptr)
	{
		ssl_close_notify(&this_ssl->ssl);
		ssl_free(&this_ssl->ssl);
		ctr_drbg_free(&this_ssl->ctr_drbg);
		entropy_free(&this_ssl->entropy);
		delete this_ssl;
		this->m_ssl = nullptr;
	} //if

    if (this->m_socket != INVALID_SOCKET)
    {
        ::shutdown(m_socket, SD_BOTH);
        // If no error occurs, closesocket returns zero
        if (::closesocket(this->m_socket) != 0)
        {
			this->SaveLastException();
            RLIB_SetExceptionInfo(this->m_error);
            return false;
        } //if
        this->m_socket = INVALID_SOCKET;
    } //if
    return true;
}

//-------------------------------------------------------------------------

bool Sockets::IsConnectable()
{
	// If the error code returned indicates the connection attempt failed, 
	// the application can call connect again for the same socket.
	switch (this->m_error.HResult) 
	{
	case WSAETIMEDOUT:
	case WSAECONNREFUSED: 
	case WSAENETUNREACH:
	case STATUS_SUCCESS:
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------

bool Sockets::Connect(unsigned long addr, unsigned short port,
					  const timeval *lptimeout /* = nullptr */)
{
	this->m_addr.sin_addr.S_un.S_addr = addr;
	this->m_addr.sin_port             = port;

	if (lptimeout != nullptr) {
		// 切换为异步IO, 忽略错误检查
		unsigned long argp = 1;
		this->IoctlSocket(this->m_socket, FIONBIO, &argp);
	} //if

__continue_conn_async:
	if (connect(this->m_socket, reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) != 0) {
		if (lptimeout != nullptr) {
			int ecode = this->GetLastErrorId();
			switch (ecode) 
			{
			case WSAEWOULDBLOCK:
				{
					fd_set writefds;
					FD_ZERO(&writefds);
#pragma warning(disable:4548) // expression before comma has no effect; expected expression with side-effect
					FD_SET(this->m_socket, &writefds);
#pragma warning(default:4548)

					int ret = this->Select(NULL, NULL, &writefds, NULL, lptimeout);
					if (ret > 0 /*|| FD_ISSET(this->m_socket, &writefds)*/) {
						goto __connected;
					} //if
					if (ret == 0) {
						this->m_error.HResult = WSAETIMEDOUT;
						goto __set_exception_and_return;
					} //if
				}
				break;
			case WSAEALREADY: 
				// An operation was attempted on a non-blocking socket that already had an operation in progress
				goto __continue_conn_async;
			default:
				this->m_error.HResult = ecode;
				goto __set_exception_and_return;
			}
		}
		this->m_error.HResult = this->GetLastErrorId();
__set_exception_and_return:
		Exception::FormatErrorMessage(this->m_error.Message, 
									  RLIB_COUNTOF(this->m_error.Message),
									  this->m_error.ErrorNo);
		RLIB_SetExceptionInfo(this->m_error);
		return false;
	}

__connected:
	if (lptimeout != nullptr) {
		// 切换回同步IO, 忽略错误检查
		unsigned long argp = 0;
		this->IoctlSocket(this->m_socket, FIONBIO, &argp);
	} //if

	if (this_ssl != nullptr) {
		int ret;
		while ((ret = ssl_handshake(&this_ssl->ssl)) != 0) {
			if (ret != POLARSSL_ERR_NET_WANT_READ &&
				ret != POLARSSL_ERR_NET_WANT_WRITE)
			{
				RLIB_SetException(this->m_error, ret, _T("ssl_handshake failed."));
				trace(!"ssl_handshake failed.");
				return false;
			} //if
		}
	} //if

	return true;
}

//-------------------------------------------------------------------------

bool Sockets::Connect(LPCTSTR host, int host_len, unsigned short host_port, 
					  const timeval *lptimeout /* = nullptr */)
{
	Ipv4Host &&iphost = this->ParseIpv4Host(host, host_len, host_port);

	if (iphost.addr != INADDR_NONE) {
		return this->Connect(iphost.addr, iphost.port, lptimeout);
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return false;
}

//-------------------------------------------------------------------------

bool Sockets::SetSendTimeout(int timeout)
{
	int optres = ::setsockopt(this->m_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&timeout),
							  sizeof(timeout));
    // If no error occurs, setsockopt returns zero. 
    if (optres == 0) {
		return true;
    } //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return false;
}

//-------------------------------------------------------------------------

int Sockets::GetSendBufferSize()
{
	int optval;
	int	optlen = sizeof(optval);
	int optres = ::getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,
							  reinterpret_cast<char *>(&optval), &optlen);
	// If no error occurs, getsockopt returns zero.
	return optres == 0 ? optval : RLIB_DEFAULT_MAX_BUFFER_SIZE;
}

//-------------------------------------------------------------------------

int Sockets::send(LPCVOID data_, int size)
{
	const char *data = reinterpret_cast<RLIB_TYPE(data)>(data_);

	int thisSend = 1;
	int allSend  = 0; 
	while (size > 0 && thisSend > 0)
	{
		thisSend = ::send(this->m_socket, data + allSend, size, NULL);
		if (thisSend < 0) // SOCKET_ERROR
		{
			this->SaveLastException();
			RLIB_SetExceptionInfo(this->m_error);

			if (allSend == 0) allSend = -1;
			break;
		} //if
		allSend += thisSend;
		size    -= thisSend;
	}
	return allSend;
}

//-------------------------------------------------------------------------

int Sockets::Send(LPCVOID data, int size)
{
	if (this_ssl != nullptr) {
		int ret;
		while ((ret = ssl_write(&this_ssl->ssl, static_cast<const unsigned char *>(data), static_cast<size_t>(size))) < 0) {
			if (ret != POLARSSL_ERR_NET_WANT_WRITE) {
				this->m_error.HResult = ret;
				rlib_format_ssl_error(this->m_error.HResult, this->m_error.Message);
				RLIB_SetExceptionInfo(this->m_error);
				break;
			} //if
		}
		return ret;
	} //if
	return this->send(data, size);
}

//-------------------------------------------------------------------------

int Sockets::SendTo(LPCVOID data, int size, unsigned long addr, unsigned short port,
					int flags /* = 0 */)
{
	sockaddr_in saddr;
	saddr.sin_family = this->m_addr.sin_family;
	saddr.sin_port   = port;
	saddr.sin_addr.S_un.S_addr = addr;
	int sent = ::sendto(this->m_socket, static_cast<const char *>(data), size, flags,
						reinterpret_cast<const sockaddr *>(&saddr), sizeof(saddr));
	if (sent != SOCKET_ERROR) {
		return sent;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::SendTo(LPCVOID data, int size, LPCTSTR host, int host_len, 
					unsigned short host_port, int flags /* = 0 */)
{
	Ipv4Host &&iphost = this->ParseIpv4Host(host, host_len, host_port);

	if (iphost.addr != INADDR_NONE) {
		return this->SendTo(data, size, iphost.addr, iphost.port, flags);
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return this->m_error.HResult;
}

//-------------------------------------------------------------------------

bool Sockets::SetReceiveTimeout(int timeout)
{
	int optres = ::setsockopt(this->m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&timeout),
							  sizeof(timeout));
	// If no error occurs, setsockopt returns zero. 
	if (optres == 0) {
		return true;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return false;
}

//-------------------------------------------------------------------------

int Sockets::GetReceiveBufferSize()
{
	int optval;
	int	optlen = sizeof(optval);
	int optres = ::getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, 
							  reinterpret_cast<char *>(&optval), &optlen);
    // If no error occurs, getsockopt returns zero.
	return optres == 0 ? optval : RLIB_DEFAULT_MAX_BUFFER_SIZE;
}

//-------------------------------------------------------------------------

int Sockets::recv(LPVOID buf, int size)
{
    int received = ::recv(m_socket, static_cast<char *>(buf), size, NULL);
    if (received != SOCKET_ERROR) {
       return received;
    } //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::Recv(LPVOID buf_, int size)
{
	if (this_ssl != nullptr)
	{
		unsigned char *buf = reinterpret_cast<RLIB_TYPE(buf)>(buf_);
		int ret;
		while ((ret = ssl_read(&this_ssl->ssl, buf, static_cast<size_t>(size))) < 0)
		{
			if (ret == POLARSSL_ERR_NET_WANT_READ ||
				ret == POLARSSL_ERR_NET_WANT_WRITE) {
				continue;
			} //if

			if (ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY) {
				break;
			} //if

			this->m_error.HResult = ret;
			rlib_format_ssl_error(this->m_error.HResult, this->m_error.Message);
			RLIB_SetExceptionInfo(this->m_error);
			break;
		}
		return ret;
	} //if
	return this->recv(buf_, size);
}

//-------------------------------------------------------------------------

int Sockets::RecvFrom(LPVOID buf, int size, 
					  sockaddr_in *addr /* = NULL */, int *addrlen /* = nullptr */, 
					  int flags /* = 0 */)
{
	int received = ::recvfrom(this->m_socket, reinterpret_cast<char *>(buf), size, flags,
							  reinterpret_cast<sockaddr *>(addr),
							  addrlen);
	if (received != SOCKET_ERROR) {
		return received;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::Bind(ULONG addr, u_short port)
{
    m_addr.sin_addr.S_un.S_addr = addr;
	m_addr.sin_port             = port;
    if (::bind(this->m_socket, reinterpret_cast<LPSOCKADDR>(&m_addr), sizeof(m_addr)) != SOCKET_ERROR) {
       return STATUS_SUCCESS;
    } //if
    
	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::Bind(u_short host_port)
{
	RLIB_HTONS(host_port, host_port);
    return this->Bind(INADDR_ANY, host_port);
}

//-------------------------------------------------------------------------

int Sockets::Listen(int backlog /* = SOMAXCONN */)
{
	if (listen(this->m_socket, backlog) != SOCKET_ERROR) {
		return STATUS_SUCCESS;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

Sockets *Sockets::Accept(sockaddr_in *addr /* = nullptr */, int *addrlen /* = nullptr */)
{
    SOCKET accepted = accept(this->m_socket, reinterpret_cast<LPSOCKADDR>(addr), addrlen);
    if (accepted != INVALID_SOCKET) {
        return new Sockets(accepted);
    } //if
    
	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return nullptr;
}

//-------------------------------------------------------------------------

Sockets *Sockets::Accept(const timeval &_timeout, sockaddr_in *addr /* = NULL */,
						 int *addrlen /* = nullptr */)
{
	fd_set readfds;
	FD_ZERO(&readfds);
#pragma warning(disable:4548) // expression before comma has no effect; expected expression with side-effect
	FD_SET(this->m_socket, &readfds);
#pragma warning(default:4548)

	int ret = this->Select(NULL, &readfds, NULL, NULL, &_timeout);
	if (ret > 0 ||
		FD_ISSET(this->m_socket, &readfds)) {
		return this->Accept(addr, addrlen);
	} //if

	if (ret < 0) {
		this->SaveLastException();
		RLIB_SetExceptionInfo(this->m_error);
	} else {
		RLIB_SetException(this->m_error, WSAETIMEDOUT, _T("WSAETIMEDOUT"));
	} //if
	return nullptr;
}

//-------------------------------------------------------------------------

int Sockets::Shutdown(int how /* = SD_BOTH */)
{
    if (shutdown(this->m_socket, how) == 0) {
       return STATUS_SUCCESS;
    } //if
    
	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

Exception *Sockets::GetLastException()
{
    return &this->m_error;
}

//-------------------------------------------------------------------------

void Sockets::SaveLastException()
{
	this->m_error.HResult = this->GetLastErrorId();
	Exception::FormatErrorMessage(this->m_error.Message,
								  RLIB_COUNTOF(this->m_error.Message),
								  this->m_error.ErrorNo);
}

//-------------------------------------------------------------------------

bool Sockets::GetLocalHost(char *host, int hostlen)
{
    return ::gethostname(host, hostlen) == STATUS_SUCCESS;
}

//-------------------------------------------------------------------------

bool Sockets::GetLocalIpAddress(_In_ const TCHAR *host, _Out_ AddrInfo **addrinfo)
{
    return ::GetAddrInfo(host, NULL, NULL, addrinfo) == STATUS_SUCCESS;
}

//-------------------------------------------------------------------------

void Sockets::FreeLocalIpAddress(PADDRINFOT *addrinfo)
{
    ::FreeAddrInfo(*addrinfo);
    *addrinfo = NULL;
}

//-------------------------------------------------------------------------

unsigned long Sockets::Ipv4StringToAddress(LPCTSTR cp)
{
#if (WINVER <= _WIN32_WINNT_WIN7)
	return inet_addr(RT2A(cp));
#else
	IN_ADDR addr;
	if (InetPton(AF_INET, cp, &addr) != 1) {
		return INADDR_NONE;
	} //if
	return addr.S_un.S_addr;
#endif
}

//-------------------------------------------------------------------------

unsigned long Sockets::Ipv4StringToAddress(LPCTSTR cp, int length)
{
#if (WINVER <= _WIN32_WINNT_WIN7)
# ifdef _UNICODE
	return inet_addr(RW2AL(cp, static_cast<intptr_t>(length)));
# else
	char host[RLIB_COUNTOF("255.255.255.255")];
	length       = min(RLIB_COUNTOF_STR("255.255.255.255"), length);
	host[length] = '\0';
	memcpy(host, cp, length * sizeof(char));

	return inet_addr(host);
# endif // _UNICODE
#else
	TCHAR host[RLIB_COUNTOF("255.255.255.255")];
	length       = min(RLIB_COUNTOF_STR("255.255.255.255"), length);
	host[length] = _T('\0');
	memcpy(host, cp, length * sizeof(TCHAR));

	return Ipv4StringToAddress(host);
#endif
}

//-------------------------------------------------------------------------

unsigned long Sockets::HostStringToAddress(LPCTSTR host)
{
	unsigned long host_addr = INADDR_NONE;

#if (WINVER <= _WIN32_WINNT_WIN7)
	LPHOSTENT pHost = ::gethostbyname(RT2A(host));	
	if (pHost != nullptr) {
//		memcpy(&host_addr, pHost->h_addr_list[0], pHost->h_length);
		assert(pHost->h_length == sizeof(unsigned long));
		host_addr = *reinterpret_cast<unsigned long *>(pHost->h_addr_list[0]);
	} //if
#else
	#pragma todo("only support IPv4")
	AddrInfo hints    = { 0 };
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	AddrInfo *addr;
	auto dwRetval = ::GetAddrInfo(host, nullptr, &hints, &addr);
	if (dwRetval == 0) {
		auto result = addr;
		trace(addr->ai_addrlen >= sizeof(sockaddr_in));
// 		while (addr->ai_family != AF_INET) {
// 			addr = addr->ai_next;
// 			if (addr == nullptr) {
// 				this->m_error.HResult = SOCKET_ERROR;
// 				FreeAddrInfo(result);
// 				goto exception_return_already_set;
// 			}
// 		}
		host_addr = reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr.S_un.S_addr;

		::FreeAddrInfo(result);
	} else {
		::WSASetLastError(dwRetval);
	} //if
#endif

	return host_addr;
}

//-------------------------------------------------------------------------

unsigned long Sockets::HostStringToAddress(LPCTSTR cp, int host_len)
{
	// To simplify implementations, the total number of octets that represent a
	// domain name(i.e., the sum of all label octets and label lengths) is
	// limited to 255.
	#define RLIB_MAX_DOMAIN_LENGTH 255

	unsigned long host_addr = INADDR_NONE;

#if (WINVER <= _WIN32_WINNT_WIN7)
# ifdef _UNICODE
	LPHOSTENT pHost = ::gethostbyname(RW2AL(cp, static_cast<intptr_t>(host_len)));
# else
	char host[RLIB_MAX_DOMAIN_LENGTH + 1];
	length = min(RLIB_MAX_DOMAIN_LENGTH, length);
	host[length] = '\0';
	memcpy(host, cp, length * sizeof(char));

	LPHOSTENT pHost = ::gethostbyname(host);
# endif // _UNICODE
	if (pHost != nullptr) {
//		memcpy(&host_addr, pHost->h_addr_list[0], pHost->h_length);
		assert(pHost->h_length == sizeof(unsigned long));
		host_addr = *reinterpret_cast<unsigned long *>(pHost->h_addr_list[0]);
	} //if
#else
	TCHAR host[RLIB_MAX_DOMAIN_LENGTH + 1];
	host_len       = min(RLIB_MAX_DOMAIN_LENGTH, host_len);
	host[host_len] = _T('\0');
	memcpy(host, cp, host_len * sizeof(TCHAR));

	host_addr = HostStringToAddress(host);
#endif

	return host_addr;
}

//-------------------------------------------------------------------------

bool Sockets::Ipv4AddressToString(unsigned long addr, LPTSTR pIpv4StringBuf, size_t ipv4StringBufSize)
{
	in_addr inaddr;
	inaddr.S_un.S_addr = addr;

#if (WINVER <= _WIN32_WINNT_WIN7)
	auto pstr = inet_ntoa(inaddr);
	// If no error occurs, 
	// inet_ntoa returns a character pointer to a static buffer 
	// containing the text address in standard ".'' notation
	if (pstr == nullptr) return false;

	// automatically convert encoding
	String result(pstr);
	auto length = RLIB_MIN(static_cast<intptr_t>(ipv4StringBufSize) - 1, result.Length);
	String::FastStringCopy(pIpv4StringBuf, result, length);
	pIpv4StringBuf[length] = _T('\0');
	return true;
#else
	return InetNtop(AF_INET, &inaddr, pIpv4StringBuf, ipv4StringBufSize) != nullptr;
#endif
}

//-------------------------------------------------------------------------

int Sockets::GetLastErrorId()
{
	return ::WSAGetLastError();
}

//-------------------------------------------------------------------------

int Sockets::GetOption(_In_ int optname)
{
	int val;
	int len = sizeof(val);
	if (this->GetOption(optname, &val, &len) == 0) return val;
	return INT_MAX;
}

//-------------------------------------------------------------------------

int Sockets::GetOption(_In_ int optname, _Out_ void *optval, _Inout_ int *optlen)
{
	if (getsockopt(this->m_socket, SOL_SOCKET, optname, static_cast<char *>(optval), optlen) == 0) {
		return STATUS_SUCCESS;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::SetOption(_In_ int optname, _In_ int val)
{
	return this->SetOption(optname, &val, sizeof(val));
}

//-------------------------------------------------------------------------

int Sockets::SetOption(_In_ int optname, _In_ const void *optval, _In_ int optlen)
{
	if (setsockopt(this->m_socket, SOL_SOCKET, optname, static_cast<const char *>(optval), optlen) == 0) {
		return STATUS_SUCCESS;
	} //if

	this->SaveLastException();
	RLIB_SetExceptionInfo(this->m_error);
	return SOCKET_ERROR;
}

//-------------------------------------------------------------------------

int Sockets::Select(_In_ int nfds,
					_Inout_opt_ fd_set FAR *readfds,
					_Inout_opt_ fd_set FAR *writefds,
					_Inout_opt_ fd_set FAR *exceptfds,
					_In_opt_ const struct timeval FAR *timeout)
{
	return select(nfds, readfds, writefds, exceptfds, timeout);
}

//-------------------------------------------------------------------------

int Sockets::IoctlSocket(_In_ RLIB_SOCKET s,
						 _In_ long cmd,
						 _Inout_ u_long FAR *argp)
{
	return ioctlsocket(s, cmd, argp);
}

//-------------------------------------------------------------------------

Ipv4Host Sockets::ParseIpv4Host(LPCTSTR host, int host_len, unsigned short host_port)
{
	unsigned long host_addr = INADDR_NONE;
	#pragma todo("potential bug when host ends with numeric characters")
	#pragma warning(push)
	#pragma warning(disable:4996) // This function or variable has been deprecated
	if (host[host_len - 1] >= _T('0') && host[host_len - 1] <= _T('9')) {
        // ip address
		host_addr = host[host_len] != _T('\0') ? Ipv4StringToAddress(host, host_len) : Ipv4StringToAddress(host);
    } else {
		// domain
		host_addr = host[host_len] != _T('\0') ? HostStringToAddress(host, host_len) : HostStringToAddress(host);
	} //if
	#pragma warning(pop)

	RLIB_HTONS(host_port, host_port);
	return { host_addr, host_port };
}

//-------------------------------------------------------------------------

template<typename LPFN> void __get_extension(_In_ SOCKET s, _In_ GUID __guid, _Out_ volatile LPFN *__lppf)
{
	LPFN __disp = nullptr;
	::WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
			   &__guid, sizeof(__guid), reinterpret_cast<void *>(&__disp), sizeof(__disp),
			   &__guid.Data1, nullptr, nullptr);

	Threading::Interlocked::ExchangePointer<PVOID>(reinterpret_cast<volatile PVOID *>(__lppf),
												   __disp);
}

//-------------------------------------------------------------------------

bool Sockets::Disconnect(_In_ SOCKET s, _In_ LPOVERLAPPED lpOverlapped)
{
#if WINVER >= _WIN32_WINNT_VISTA
	static volatile LPFN_DISCONNECTEX DisconnectEx = nullptr;
	if (DisconnectEx == nullptr) {
		__get_extension<LPFN_DISCONNECTEX>(s, WSAID_DISCONNECTEX, &DisconnectEx);
	} //if

	// ConnectEx <-> TransmitFile(s, NULL, 0, 0, nullptr, nullptr, TF_DISCONNECT | TF_REUSE_SOCKET)
	// AcceptEx  <-> TransmitPackets(s, NULL, 0, 0, nullptr, TF_DISCONNECT | TF_REUSE_SOCKET)
	return DisconnectEx && DisconnectEx(s, lpOverlapped, TF_REUSE_SOCKET, 0) == TRUE;
#else
	UNREFERENCED_PARAMETER(s);
	UNREFERENCED_PARAMETER(lpOverlapped);
	return false;
#endif
}