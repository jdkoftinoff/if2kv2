#ifndef JDK_SOCKET_WIN32_H
#define JDK_SOCKET_WIN32_H

// Win32 specific TCPIP Network stuff

#if JDK_HAS_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


#undef JDK_GETSOCKOPTVALTYPE
#define JDK_GETSOCKOPTVALTYPE char *
#define errnum() WSAGetLastError()
#define herrnum() WSAGetLastError()


//#define EINTR	WSAELOOP

#undef EWOULDBLOCK
#define EWOULDBLOCK             WSAEWOULDBLOCK
#undef EINPROGRESS
#define EINPROGRESS             WSAEINPROGRESS
#undef EALREADY
#define EALREADY                WSAEALREADY
#undef ENOTSOCK
#define ENOTSOCK                WSAENOTSOCK
#undef EDESTADDRREQ
#define EDESTADDRREQ            WSAEDESTADDRREQ
#undef EMSGSIZE
#define EMSGSIZE                WSAEMSGSIZE
#undef EPROTOTYPE
#define EPROTOTYPE              WSAEPROTOTYPE
#undef ENOPROTOOPT
#define ENOPROTOOPT             WSAENOPROTOOPT
#undef EPROTONOSUPPORT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#undef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#undef EOPNOTSUPP
#define EOPNOTSUPP              WSAEOPNOTSUPP
#undef EPFNOSUPPORT
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#undef EAFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#undef EADDRINUSE
#define EADDRINUSE              WSAEADDRINUSE
#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#undef ENETDOWN
#define ENETDOWN                WSAENETDOWN
#undef ENETUNREACH
#define ENETUNREACH             WSAENETUNREACH
#undef ENETRESET
#define ENETRESET               WSAENETRESET
#undef ECONNABORTED
#define ECONNABORTED            WSAECONNABORTED
#undef ECONNRESET
#define ECONNRESET              WSAECONNRESET
#undef ENOBUFS
#define ENOBUFS                 WSAENOBUFS
#undef EISCONN
#define EISCONN                 WSAEISCONN
#undef ENOTCONN
#define ENOTCONN                WSAENOTCONN
#undef ESHUTDOWN
#define ESHUTDOWN               WSAESHUTDOWN
#undef ETOOMANYREFS
#define ETOOMANYREFS            WSAETOOMANYREFS
#undef ETIMEDOUT
#define ETIMEDOUT               WSAETIMEDOUT
#undef ECONNREFUSED
#define ECONNREFUSED            WSAECONNREFUSED
#undef ELOOP
#define ELOOP                   WSAELOOP
#undef ENAMETOOLONG
#define ENAMETOOLONG            WSAENAMETOOLONG
#undef EHOSTDOWN
#define EHOSTDOWN               WSAEHOSTDOWN
#undef EHOSTUNREACH
#define EHOSTUNREACH            WSAEHOSTUNREACH
#undef ENOTEMPTY
#define ENOTEMPTY               WSAENOTEMPTY
#undef EPROCLIM
#define EPROCLIM                WSAEPROCLIM
#undef EUSERS
#define EUSERS                  WSAEUSERS
#undef EDQUOT
#define EDQUOT                  WSAEDQUOT
#undef ESTALE
#define ESTALE                  WSAESTALE
#undef EREMOTE
#define EREMOTE                 WSAEREMOTE


// Here is some nice hack stuff to initialize Winsock

#ifndef MAKEWORD
#define MAKEWORD(a, b) \
	((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

class WinSockInit
{
#ifndef JDK_SOCKET_NOWINSOCKINIT
public:
	WinSockInit()
	{

#if JDK_HAS_SSL
		SSLeay_add_ssl_algorithms();
		SSL_load_error_strings();
#endif

		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		
		wVersionRequested = MAKEWORD( 1, 1 );
		
		err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 ) 
		{
			return;
		}
		if ( LOBYTE( wsaData.wVersion ) != 1 ||
			HIBYTE( wsaData.wVersion ) != 1 ) 
		{
			WSACleanup( );
		}
		
		return;
	}
	  
	~WinSockInit()
	{
		WSACleanup();
	}
#endif	  
};


#endif



