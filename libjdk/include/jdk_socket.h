#ifndef _JDK_SOCKET_H
#define _JDK_SOCKET_H

#include "jdk_world.h"
#include "jdk_map.h"
#include "jdk_thread.h"
#include "jdk_string.h"
#include "jdk_buf.h"
#include "jdk_error.h"

#if JDK_HAS_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


#if JDK_IS_BEOS
#include <kernel/OS.h>
#include <NetKit.h>
#include <socket.h>
#include <errno.h>
#include <stdio.h>
#endif

#if JDK_IS_UNIX || JDK_IS_WINE
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#endif

#if JDK_IS_WIN32 && !JDK_IS_WINE
#include <winsock.h>
typedef unsigned int jdk_socket_handle;
#define JDK_SOCKET_ERROR ((unsigned int)SOCKET_ERROR)

#else
typedef int jdk_socket_handle;
#define JDK_SOCKET_ERROR (-1)
#endif

#define JDK_GETSOCKOPTVALTYPE int *

#if JDK_IS_WIN32 && !JDK_IS_WINE
#define JDK_SOCKADDRLENTYPE int
#include "jdk_socket_win32.h"

#elif JDK_IS_BEOS
#define JDK_SOCKADDRLENTYPE int
#include "jdk_socket_beos.h"

#elif JDK_IS_LINUX || JDK_IS_NETBSD || JDK_IS_WINE || JDK_IS_MACOSX_TIGER
#if JDK_IS_CYGWIN 
#define JDK_SOCKADDRLENTYPE int
#else
#define JDK_SOCKADDRLENTYPE socklen_t
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "jdk_socket_unix.h"

#elif JDK_IS_UNIX && !JDK_IS_LINUX

#define JDK_SOCKADDRLENTYPE int
#include "jdk_socket_unix.h"

#endif




//
// Classes defined here
//

class jdk_socket;
	class jdk_client_socket;
		class jdk_inet_client_socket;
			class jdk_nb_inet_client_socket;
		class jdk_file_socket;


	class jdk_server_socket;
		class jdk_inet_server_socket;
			class jdk_nb_inet_server_socket;		// non blocking

class jdk_select_manager;
class jdk_socket_task;
class jdk_dns_cache;



#if !JDK_IS_WIN32 || JDK_IS_WINE
int jdk_socket_lpwrite(int s, const void *msg, int len );
int jdk_socket_lpread(int s, void *msg, int len );
#endif
int jdk_socket_lpsend(int s, const void *msg, int len, int flags );
int jdk_socket_lprecv(int s, void *msg, int len, int flags );
int jdk_socket_lpselect( int n,  fd_set  *readfds,  fd_set *writefds, 
					 fd_set *exceptfds, 
					 struct timeval *timeout
					 );
int jdk_socket_lpconnect( int sockfd, struct sockaddr *serv_addr, int  addrlen );
jdk_socket_handle jdk_socket_lpsocket(int domain, int type, int protocol);
jdk_socket_handle jdk_socket_lpaccept(int s, struct sockaddr *addr, unsigned int *addrlen);
int jdk_socket_lpgethostbyname( const char *name, struct sockaddr_in *addr );


class jdk_socket 
{
public:
    
    jdk_socket() 	{}
    
	virtual ~jdk_socket();
	    
	virtual jdk_socket_handle  get_fd() const= 0;
	
	virtual bool is_open_for_reading() const=0;

	virtual bool is_open_for_writing() const=0;
	
	virtual void close() = 0;
	
	virtual void shutdown_read() = 0;
	
	virtual void shutdown_write() = 0;
}; 


class jdk_client_socket : public jdk_socket 
{
	
	/*
	 
	 */
		
	
public:
    
	jdk_client_socket() : default_timeout(120) {}
	
    
	virtual ~jdk_client_socket();

	void set_default_timeout( int d )
	{
		default_timeout = d;
	}
	
	virtual void setup_socket_blocking(bool blocking)
		{
		}	
	
	// Abstract Methods:
	
    
	virtual	bool is_connected() const = 0;

	virtual bool is_open_for_reading() const=0;

	virtual bool is_open_for_writing() const=0;
    
	
    
	virtual	bool make_connection( const char *hostname, int port, jdk_dns_cache *dnscache, bool use_ssl_ ) = 0;
	virtual bool make_connection( const char *hostandport, jdk_dns_cache *dnscache, bool use_ssl_ ) = 0;	
	virtual bool make_connection( const jdk_string &hostandport, jdk_dns_cache *dnscache, bool use_ssl_ ) = 0;		


	// get_connection_status() returns:
	// 		-1 upon failure of connection,
	//		0 upon success of connection
	//		1 if connection is still proceeding

	virtual int get_connection_status();

    
	virtual void force_connection( jdk_socket_handle fd ) = 0;
	
	
	virtual void detach() = 0;
	
    
	virtual jdk_socket_handle  get_fd() const= 0;
	
    
	virtual bool get_local_addr( char *addr, size_t max_len ) const = 0;
	
	bool get_local_addr( jdk_string &addr ) const
	{
	    return get_local_addr( addr.c_str(), addr.getmaxlen() );
	}
	
	virtual bool get_local_addr( struct sockaddr_in *addr ) const = 0;	
    
	virtual bool get_remote_addr( char *addr, size_t max_len ) const = 0;

	bool get_remote_addr( jdk_string &addr ) const
	{
	    return get_remote_addr( addr.c_str(), addr.getmaxlen() );
	}

	virtual bool get_remote_addr( struct sockaddr_in *addr ) const = 0;	
	
    
	virtual	void close() = 0;

	virtual void shutdown_read() = 0;
	
	virtual void shutdown_write() = 0;

	virtual int  read_data( void *buf, size_t len, int timeout_seconds=-1 ) = 0;

	virtual int  write_data( const void *buf, size_t len, int timeout_seconds=-1 ) = 0;

  virtual int  send_data( const void *buf, size_t len ) = 0;
  virtual int  recv_data( void *buf, size_t len ) = 0;

  virtual int  send_buf( const jdk_buf &buf ) { return send_data( buf.get_data(), buf.get_data_length() ); }
  virtual int  recv_buf( jdk_buf &buf ) { buf.clear(); return recv_data( buf.get_data(), buf.get_buf_length() ); }

	// final methods:

	// returns length of string read.
	
	virtual int	 read_string_with_timeout(
										  void *str, 
										  size_t max_len,
										  int timeout_seconds,
										  const char *terminators="\n",
										  const char *ignores="\r"
										  );
    int read_string_with_timeout( 
								 jdk_string &str,
								 int timeout_seconds,
								 const char *terminators="\n",
								 const char *ignores="\r"
					)
	{
	    return read_string_with_timeout( str.c_str(), str.getmaxlen(), timeout_seconds, terminators, ignores );
	}
	
	virtual int	 read_string( 
							 void *str, 
							 size_t max_len,
							 const char *terminators="\n",
							 const char *ignores="\r"
							 );
  int read_string( 
                    jdk_string &str,
					const char *terminators="\n",
					const char *ignores="\r"
					)
	{
	    return read_string( str.c_str(), str.getmaxlen(), terminators, ignores );
	}

    virtual int read_buf( jdk_buf &buf, int maxlen_=4096 );
	
	// returns length of string read, or -1 if not complete yet.
	// returns -2 if socket was closed during read
	virtual int  read_partial_string(
									 void *str,
									 size_t max_len,
									 size_t *cur_pos,
									 const char *terminators="\n",
									 const char *ignores="\r"
									 );
									 
    int read_partial_string( 
                    jdk_string &str,
		            size_t *cur_pos,
					const char *terminators="\n",
					const char *ignores="\r"
					)
	{
	    return read_partial_string( str.c_str(), str.getmaxlen(), cur_pos, terminators, ignores );
	}

        
	virtual int  write_data_block( const void *buf, size_t len );

	virtual int  write_data_block( const jdk_buf &buf )
	{
	  return write_data_block( buf.get_data(), buf.get_data_length() );
	}
		
	virtual int	 write_string_block( const char *str );
	
	virtual int  write_string_block( const jdk_string &str )
	{
	  return write_string_block( str.c_str() );
	}
	

  virtual bool read_data_block( const void *buf_, size_t required_len, int timeout=2 )
  {
    unsigned char *buf = (unsigned char *)buf_;
    size_t todo = required_len;
    size_t pos = 0;

    while( todo>0 )
    {
      int count = read_data( buf+pos, todo, timeout );
      if( count>0 )
      {
        pos+=count;
        todo-=count;
      }
      else
      {
        break;
      }
    }

    return todo==0;
  }

  virtual bool read_data_block( jdk_buf &buf, size_t required_len, int timeout=2 )
  {
    bool r=false;
    buf.clear();
    buf.resize( required_len );
    r=read_data_block( buf.get_data(), required_len, timeout );

    if( r )
    {
      buf.set_data_length( required_len );
    }
    else
    {
      buf.clear();
    }

    return r;
  }
	
	virtual int	 print( const char *fmt, ... );
		
protected:

	virtual void write_failed() = 0;
	
	virtual void read_failed() = 0;

	int default_timeout;
private:
	
};


class jdk_fd_client_socket : public jdk_client_socket 
{
public:
    
	jdk_fd_client_socket();
    
	virtual ~jdk_fd_client_socket();

    
	bool is_connected() const;

	bool is_open_for_reading() const;
 	 
	bool is_open_for_writing() const;
	    
    bool make_connection( const char *hostname, int port, jdk_dns_cache *dnscache, bool use_ssl_ ) {return false;}
    bool make_connection( const char *hostandport, jdk_dns_cache *dnscache, bool use_ssl_ )  { return false; }
    bool make_connection( const jdk_string &hostandport, jdk_dns_cache *dnscache, bool use_ssl_ ) { return false; }
	    
	void force_connection( jdk_socket_handle fd );
   
	void detach();
	    	
	jdk_socket_handle  get_fd() const;
	    
	bool get_local_addr( char *addr, size_t max_len ) const;
    
	bool get_remote_addr( char *addr, size_t max_len ) const;

	bool get_local_addr( struct sockaddr_in *addr ) const
	{
	    return false;
	}

	bool get_remote_addr( struct sockaddr_in *addr ) const 
	{
	    return false;
	}	
    
	void close();

	void shutdown_read();
	
	void shutdown_write();
    
	int  read_data( void *buf, size_t len, int timeout_seconds=-1 );
	    
	int  write_data( const void *buf, size_t len, int timeout_seconds=-1 );
	
protected:

	void write_failed();
	
	void read_failed();
	
	
protected:
    
	jdk_socket_handle fd;

};


class jdk_inet_client_socket : public jdk_client_socket 
{
public:
    
	jdk_inet_client_socket();
    
	virtual ~jdk_inet_client_socket();

	void setup_socket_blocking(bool blocking);

	bool is_connected() const;
    
	bool is_open_for_reading() const;
 	 
	bool is_open_for_writing() const;

	bool bind_socket( const char *ip, int port );
	
	bool bind_socket( const char *binding );

	bool bind_socket( const jdk_string &binding );
	
	bool make_connection( const char *hostname, int port, jdk_dns_cache *dnscache, bool use_ssl_ );

	bool make_connection_from( const char *hostname, int port, const char *bindname, int bindport, jdk_dns_cache *dnscache, bool use_ssl_ );

	bool make_connection( const char *hostandport, jdk_dns_cache *dnscache, bool use_ssl_)
	{		
	    char addr[4096];
	    int port;
	    if( jdk_sscanf( hostandport, "%[^:]:%d", addr, &port )==2 )
	    {
			return make_connection( addr, port, dnscache, use_ssl_ );
	    }
		else
		{
			return false;
		}
	}
	
	bool make_connection( const jdk_string &hostandport, jdk_dns_cache *dnscache, bool use_ssl_)
	{
		return make_connection( hostandport.c_str(), dnscache, use_ssl_ );
	}
	

	void force_connection( jdk_socket_handle fd );
	
	void detach();
    	
	jdk_socket_handle  get_fd() const;
    
	bool get_local_addr( char *addr, size_t max_len ) const;
	bool get_local_addr( struct sockaddr_in *addr ) const;
    
	bool get_remote_addr( char *addr, size_t max_len ) const;
	bool get_remote_addr( struct sockaddr_in *addr) const;
    
    
	void close();

	void shutdown_read();
	
	void shutdown_write();
    	
	int  read_data( void *buf, size_t len, int timeout_seconds=-1 );
    
	int  write_data( const void *buf, size_t len, int timeout_seconds );

  int  send_data( const void *buf, size_t len );
  int  recv_data( void *buf, size_t len );

protected:	
	void write_failed();
	
	void read_failed();
	
   	virtual void setup_socket();
	
	jdk_socket_handle sock_id;
	bool connected;
	bool readable;
	bool writable;

#if JDK_HAS_SSL
	bool use_ssl;
	SSL_CTX *ssl_ctx;
	SSL *ssl;
	SSL_METHOD *ssl_client_method;
#endif
};


class jdk_nb_inet_client_socket : public jdk_inet_client_socket 
{
public:
    
	jdk_nb_inet_client_socket();
    
	virtual ~jdk_nb_inet_client_socket();
    	    
	// get_connection_status() returns:
	// 		-1 upon failure of connection,
	//		0 upon success of connection
	//		1 if connection is still proceeding
	
	int get_connection_status();

protected:

	// overriding jdk_inet_client_socket:
	
    void setup_socket();

};



class jdk_server_socket : public jdk_socket 
{
public:
    
	jdk_server_socket() {}
    
	virtual ~jdk_server_socket();
	
	virtual bool is_open_for_reading() const=0;

	virtual bool is_open_for_writing() const=0;
    
    
	virtual bool bind_socket(										 
							 int port, 										 
							 int max_connect,
							 const char *addr
							 ) = 0;

	virtual bool bind_socket( const char *binding, int max_connect ) = 0;

	virtual bool bind_socket( const jdk_string &binding, int max_connect ) = 0;

	virtual jdk_socket_handle  get_fd() const= 0;
	
    
	virtual bool accept( jdk_client_socket *s ) = 0;
	
	
	virtual jdk_client_socket * accept_create() = 0;
	
    
	virtual void close() = 0;

	virtual void shutdown_read() = 0;
	
	virtual void shutdown_write() = 0;
};


class jdk_inet_server_socket : public jdk_server_socket 
{
public:
    
	jdk_inet_server_socket();
	
	explicit jdk_inet_server_socket( const jdk_string &binding, int max_connect=10 )
	    : server_address(), sock_id(JDK_SOCKET_ERROR)
	{
		if( !bind_socket( binding, max_connect ) )
    {
			jdk_log( JDK_LOG_ERROR, "Unable to bind socket to: %s", binding.c_str() );
//			JDK_THROW_ERROR( "Unable to bind socket to: ", binding.c_str() );
    }
	}
	
	explicit jdk_inet_server_socket(										 
						   int port, 										 
						   int max_connect=10,
						   const char *addr = 0
						   ) :
		server_address(), sock_id(JDK_SOCKET_ERROR)
	{
	    if( !bind_socket( port, max_connect, addr ) )
	    {			
        jdk_log( JDK_LOG_ERROR, "Unable to bind socket to: %s:%d", addr ? addr : "0", port );
//	        JDK_THROW_ERROR( "Unable to bind socket", ""  );
        }	    
	}


    
	virtual ~jdk_inet_server_socket();
	
	bool is_open_for_reading() const;
 	 
	bool is_open_for_writing() const;
    
	bool bind_socket(										 
					 int port, 										 
					 int max_connect,
					 const char *addr
					 );

	bool bind_socket( const jdk_string &binding, int max_connect )
	{
		return bind_socket( binding.c_str(), max_connect );
	}

	bool bind_socket( const char *binding, int max_connect )
	{
		sock_id=JDK_SOCKET_ERROR;
	    char addr[256];
	    int port;		
	    if( jdk_sscanf( binding, "%[^:]:%d", addr, &port )==2 )
	    {
			close();
	        if( !bind_socket( port, max_connect, addr ) )
			{
				return false;
		    }
	    }
	    else
	    {
			return false;
	    }
		return true;
	}
		
    
	jdk_socket_handle  get_fd() const;
	
	
	bool accept( jdk_client_socket *s );
	
	
	jdk_client_socket * accept_create();
	
	
	void close();

	void shutdown_read();
	
	void shutdown_write();
    

protected:

	struct sockaddr_in server_address;

	jdk_socket_handle    sock_id;

};


class jdk_nb_inet_server_socket : public jdk_inet_server_socket 
{
public:
    
	jdk_nb_inet_server_socket()
	{
	}

	explicit jdk_nb_inet_server_socket( const jdk_string &binding, int max_connect=30 )
	{
		if( !bind_socket( binding, max_connect ) )
			JDK_THROW_ERROR( "Unable to bind socket to: ", binding.c_str() );
	}
	
	explicit jdk_nb_inet_server_socket(										 
						   int port, 										 
						   int max_connect=10,
						   const char *addr = 0
						   ) 
	{
	    if( !bind_socket( port, max_connect, addr ) )
	    {			
			jdk_log( JDK_LOG_ERROR, "Unable to bind socket to: %s:%d", addr ? addr : "0", port );
	        JDK_THROW_ERROR( "Unable to bind socket", "" );
        }	    
	}


		    
	virtual ~jdk_nb_inet_server_socket();
		    		

	bool bind_socket(										 
					 int port, 										 
					 int max_connect,
					 const char *addr
					 );

	bool bind_socket( const jdk_string &binding, int max_connect )
	{
		return jdk_inet_server_socket::bind_socket( binding.c_str(), max_connect );
	}

	bool bind_socket( const char *binding, int max_connect )
	{
		return jdk_inet_server_socket::bind_socket( binding, max_connect );
	}
	
	jdk_client_socket * accept_create();

};

#if 0
class jdk_file_socket : public jdk_fd_client_socket 
{
public:
	jdk_file_socket() :
	 open_flags(0)
	{
	}
		
	jdk_file_socket( const char *fname, int open_flags, int mode=0666 );



	virtual ~jdk_file_socket();

	bool is_open_for_reading() const;

	bool is_open_for_writing() const;


	bool open( const char *fname, int open_flags, int mode=0666 );

	jdk_int64 seek( jdk_int64 pos, int whence );

	void shutdown_read();

	void shutdown_write();

	bool make_connection( const char *hostname,  jdk_dns_cache *dnscache, bool use_ssl_ );

private:
	int open_flags;

};
#endif


class jdk_select_manager 
{
public:
	
	jdk_select_manager();
	
	
	void clear();

	
	void add_all( jdk_socket_handle fd );	
	void add_all( jdk_socket *s );	
	void add_all( jdk_socket &s )
	{
	    add_all( &s );
	}


	void add_read( jdk_socket_handle fd );
	void add_read( jdk_socket *s );
	void add_read( jdk_socket &s )
	{
	    add_read( &s );
	}


	void add_write( jdk_socket_handle fd );
	void add_write( jdk_socket *s );
	void add_write( jdk_socket &s )
	{
	    add_write( &s );
	}

	
	void add_except( jdk_socket_handle fd );	
	void add_except( jdk_socket *s );
	void add_except( jdk_socket &s )
	{
	    add_except( &s );
	}
		
	
	void set_timeout( unsigned long secs, unsigned long usecs=0 ) 
	{
		timeout.tv_sec=secs;
		timeout.tv_usec=usecs;
	}
	
   void set_timeout_forever()
   {
	   forever=true;
   }
		
	int do_select();

	
	bool can_read( jdk_socket_handle fd ) const 
	{
    return fd!=JDK_SOCKET_ERROR && FD_ISSET( fd, const_cast<fd_set *>(&read_set) );
	}
	
    
	bool can_read( jdk_socket *s ) const 
	{
    return s && s->is_open_for_reading() && FD_ISSET( s->get_fd(), const_cast<fd_set *>(&read_set) );
	}
	
	bool can_read( jdk_socket &s ) const 
	{
    return s.is_open_for_reading() && FD_ISSET( s.get_fd(), const_cast<fd_set *>(&read_set) );	
	}
		    
	bool can_write( jdk_socket_handle fd ) const 
	{
    return fd!=JDK_SOCKET_ERROR && FD_ISSET( fd, const_cast<fd_set *>(&write_set) );
    }
				    
	bool can_write( jdk_socket *s ) const 
	{
    return s && s->is_open_for_writing() && FD_ISSET( s->get_fd(), const_cast<fd_set *>(&write_set) );
	}
	
	bool can_write( jdk_socket &s ) const 
	{
    return s.is_open_for_writing() && FD_ISSET( s.get_fd(), const_cast<fd_set *>(&write_set) );
	}		
    
	bool is_except( jdk_socket_handle fd ) const 
	{
    return fd!=JDK_SOCKET_ERROR && FD_ISSET( fd, const_cast<fd_set *>(&except_set) );
	}		
    
	bool is_except( jdk_socket *s ) const 
	{
    return s && s->get_fd()!=JDK_SOCKET_ERROR && FD_ISSET( s->get_fd(), const_cast<fd_set *>(&except_set) );
	}

	bool is_except( jdk_socket &s ) const 
	{
    return s.get_fd()!=JDK_SOCKET_ERROR && FD_ISSET( s.get_fd(), const_cast<fd_set *>(&except_set) );
	}
		
		
private:	
	
	fd_set read_set, write_set, except_set;
	
	struct timeval timeout;
    bool forever;
	
	jdk_socket_handle max_fd;
};


class jdk_socket_task 
{ 
public:
	jdk_socket_task();
	virtual ~jdk_socket_task();

	virtual bool is_done()
	{
		return false;
	}
	
	virtual void start() = 0;
	
	virtual void prepare( jdk_select_manager *mgr ) =0;

	virtual	bool dispatch( jdk_select_manager *mgr ) =0;

	virtual bool timeout() = 0;
	
	virtual void stop() = 0;
	
}; 

class jdk_socket_task_manager 
{
public:
   	jdk_socket_task_manager( int max_tasks_ = 256 );
	~jdk_socket_task_manager();
	
	bool add_task( jdk_socket_task *task );
	bool remove_task( jdk_socket_task *task );
	
	void poll( int timeout_secs =0);
	
private:

	int max_tasks;
	
	jdk_socket_task **task_list;

	jdk_socket_task_manager( const jdk_socket_task_manager & );
	const jdk_socket_task_manager & operator = (const jdk_socket_task_manager& );
}; 

void jdk_single_socket_task_run( jdk_socket_task *t );

int jdk_gethostname( char *name, int maxlen );
int jdk_gethostbyname( const char *name, struct sockaddr_in *addr );
int jdk_gethostbyname( const char *name, char *ip, int ip_sz );

int jdk_gethostbyaddr( const struct sockaddr_in *addr, char *name, int name_sz );
int jdk_gethostbyaddr( const char *addr, char *name, int name_sz );

class jdk_dns_cache
{
public:
	jdk_dns_cache( unsigned long max_age=500 );
	
	int gethostbyname( const char *name, struct sockaddr_in *addr );

protected:	

	void kill_old_entries();
	
private:

	typedef jdk_str<256> dns_name;
	struct dns_info
	{
		dns_info() : addr(), usage(0), birth(0)
		{
		}
		
		struct sockaddr_in addr;
		unsigned long usage;
		unsigned long birth;
	};
	
	
	jdk_map< dns_name, dns_info > dns_map;
	unsigned long max_age;
	unsigned long access_count;
	unsigned long last_cleaning_time;
#if JDK_HAS_THREADS
	jdk_mutex map_mutex;
#endif	

	
}; 

#endif







