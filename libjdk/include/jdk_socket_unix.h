#ifndef _JDK_SOCKET_UNIX_H
#define _JDK_SOCKET_UNIX_H

// unix specific tcpip socket stuff

#include <signal.h>
#include <errno.h>

#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#if JDK_HAS_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif



#define SOCKET_ERROR (-1)
#define errnum() errno
#define herrno() h_errno
#define closesocket(a) close(a)

// assume POSIX compliant system - ignore SIGPIPE signals

class PosixSockInit
{
public:
	
	PosixSockInit()
	{
		// tell the system to ignore PIPE signals
		signal( SIGPIPE, SIG_IGN );
#if JDK_HAS_SSL
		SSLeay_add_ssl_algorithms();
		SSL_load_error_strings();
#endif
	}	    
};

#endif

