#include "jdk_world.h"

#include "jdk_socket.h"
#include "jdk_thread.h"

#if JDK_IS_BEOS
// beos likes to return B_ERROR instead of EINTR sometimes!

# define JDK_LOOP_CONDITION ((ret==SOCKET_ERROR && errnum() == EINTR)||(ret==SOCKET_ERROR && errnum() == B_ERROR))
# define JDK_LOOP_CONDITION1 ((ret==-1 && errnum() == EINTR)||(ret==-1 && errnum() == B_ERROR))

#elif JDK_IS_WIN32 && !JDK_IS_WINE
# define JDK_LOOP_CONDITION (0 )
# define JDK_LOOP_CONDITION1 (0 )
#else

# define JDK_LOOP_CONDITION ((ret==JDK_SOCKET_ERROR && errnum() == EINTR) )
# define JDK_LOOP_CONDITION1 ((ret==-1 && errnum() == EINTR) )
#endif

#if JDK_IS_MACOSX
bool jdk_socket_override_filtering;
#endif

int jdk_socket_lpsend(jdk_socket_handle s, const void *msg, int len, int flags ) 
{
  int ret;
  
  do
  {
    ret = send(s,(const char *)msg,len,flags);
  } while( JDK_LOOP_CONDITION1 );
  
#if !JDK_IS_WIN32
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket could not write data
    return 0;
  }
  else
#endif
  {
    return ret;
  }
  
  
}



int jdk_socket_lprecv(jdk_socket_handle s, void *msg, int len, int flags )
{
  int ret;
  
  do
  {
    ret = recv(s,(char *)msg,len,flags);
  } while( JDK_LOOP_CONDITION1 );
  
#if !JDK_IS_WIN32
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket had no data to read
    return 0;
  }
  else
#else

#endif
  {
    return ret;
  }
  
}




#if JDK_IS_WIN32 && !JDK_IS_WINE
static WinSockInit _winsockinit;
#elif JDK_IS_BEOS
# define JDK_SOCKADDRLENTYPE int
#elif JDK_IS_UNIX
static PosixSockInit _posixsockinit;
#endif





int jdk_socket_lpwrite(jdk_socket_handle s, const void *msg, int len ) 
{
#if JDK_IS_WIN32 && !JDK_IS_WINE
  return -1; // TODO: use proper WINAPI call
#else	
  int ret;
  
  do
  {
    ret = write(s,(const char *)msg,len);
  } while( ret==-1 && errno == EINTR );
  
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket could not write data
    return 0;
  }
  else
  {			
    return ret;
  }
#endif	
}



int jdk_socket_lpread(jdk_socket_handle s, void *msg, int len ) 
{
#if JDK_IS_WIN32 && !JDK_IS_WINE
  return -1;	// TODO: use proper WINAPI call
#else	
  int ret;
  
  do
  {
    ret = read(s,(char *)msg,len);
  } while( ret==-1 && errno == EINTR );
  
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket had no data to read
    return 0;
  }
  else
  {			
    return ret;
  }
#endif	
}


#if 0
int jdk_socket_lpsend(jdk_socket_handle s, const void *msg, int len, int flags ) 
{
  int ret;
  
  do
  {
    ret = send(s,(const char *)msg,len,flags);
  } while( JDK_LOOP_CONDITION1 );
  
#if !JDK_IS_WIN32
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket could not write data
    return 0;
  }
  else
#endif
  {
    return ret;
  }
  
  
}



int jdk_socket_lprecv(jdk_socket_handle s, void *msg, int len, int flags )
{
  int ret;
  
  do
  {
    ret = recv(s,(char *)msg,len,flags);
  } while( JDK_LOOP_CONDITION1 );
  
#if !JDK_IS_WIN32
  if( ret==-1 && errno == EAGAIN )
  {
    // return 0 instead of error if non-blocking socket had no data to read
    return 0;
  }
  else
#else

#endif
  {
    return ret;
  }
  
}
#endif


int jdk_socket_lpselect( 
  jdk_socket_handle n,  
  fd_set  *readfds,  
  fd_set *writefds,
  fd_set *exceptfds, 
  struct timeval *timeout
  )
{
  int ret;
  
  do
  {
    ret = select( n, readfds, writefds, exceptfds, timeout );
  } while( JDK_LOOP_CONDITION1);
  
  return ret;	
}



int jdk_socket_lpconnect( jdk_socket_handle sockfd, struct sockaddr *serv_addr, int  addrlen ) 
{
  int ret;
  
  do
  {
    ret = connect( sockfd, serv_addr, addrlen );
    
  } while( JDK_LOOP_CONDITION1 );
  
  return ret;
}



jdk_socket_handle jdk_socket_lpsocket(int domain, int type, int protocol)
{
  jdk_socket_handle ret;
  
  do
  {
    ret = (jdk_socket_handle)socket(domain,type,protocol);
  } while( JDK_LOOP_CONDITION );
  
  return ret;
}



jdk_socket_handle jdk_socket_lpaccept(jdk_socket_handle s, struct sockaddr *addr, JDK_SOCKADDRLENTYPE *addrlen)
{
  jdk_socket_handle ret;
  
  do
  {
    ret = (jdk_socket_handle)accept(s,addr,addrlen);
  } while( JDK_LOOP_CONDITION );
  
  return ret;
}

#if JDK_IS_UNIX && JDK_HAS_THREADS && !JDK_IS_MACOSX && !JDK_IS_NETBSD
#define JDK_HAS_GETHOSTBYNAME_R 0
#else
#define JDK_HAS_GETHOSTBYNAME_R 0
#endif

int jdk_socket_lpgethostbyname( const char *name, struct sockaddr_in *addr )
{
#if JDK_HAS_GETHOSTBYNAME_R
  
  struct hostent *host=(struct hostent *)calloc(sizeof(struct hostent),1);
  struct hostent *r;
  char *buf = (char *)calloc(32768,1);
  int e;
  
  if( gethostbyname_r(
        name,
        host,
        buf, 32768,
        &r, &e )<0
    )
  {
    /* host lookup failed */
    free(host);
    free(buf);
    return -1;
  }
  
  if( r )
  {
    addr->sin_family = r->h_addrtype;
    addr->sin_addr.s_addr = *((long*)r->h_addr);
    free(host);
    free(buf);
    return 0;
  }
  else
  {
    free(host);
    free(buf);
    return -1;
  }
  
#else
#if !JDK_IS_WIN32
  
#if JDK_HAS_FORK
#endif
  
  static jdk_mutex mymutex("gethostbyname_r");
  jdk_mutexsection section(mymutex);
  
#endif
  
  struct hostent *r;
  
//	jdk_log( JDK_LOG_INFO, "*GETHOSTBYNAME* %s", name );
  r=gethostbyname(name);
  
  if( r )
  {
    addr->sin_family = r->h_addrtype;
    addr->sin_addr.s_addr = *((long*)r->h_addr);
    
    return 0;
  }
  else
  {
    return -1;
  }
  
#endif
}




jdk_socket::~jdk_socket()
{
}




jdk_client_socket::~jdk_client_socket()
{
}


int jdk_client_socket::get_connection_status()
{
  if( is_connected() )
  {
    return 0;
  }
  else
  {
    return -1;
  }
} 

int	 jdk_client_socket::read_string_with_timeout(
  void *str_,
  size_t max_len,
  int timeout_seconds,
  const char *terminators,
  const char *ignores
  ) 
{
  size_t len=0;
  char *str = (char *)str_;
  
  // only read max_len characters.
  
  while(is_open_for_reading() && len<max_len-1 )
  {
    char c;
    
    // read one character
    
    int cnt = read_data( &c, 1, timeout_seconds );
    
    // did we get one?
    
    if( cnt==1 )
    {
      // yes
      
      // was it an ignore character?
      
      if( ignores )
      {
        if( strchr( ignores, c ) )
        {
          // yes it was
          // ignore this character. try read the next character.
          continue;
        }
      }
      
      // was it a terminator?
      
      if( terminators )
      {
        if( strchr( terminators, c ) )
        {
          // yes it was a terminator. return the string as is.
          break;
        }
        
      }
      
      
      // anything else is appended to our string
      
      *str++ = c;
      ++len;
    }
    else if( cnt<=0 )
    {
      read_failed();
      return -1;
    }
  }		
  
  // put null on end of c string
  *str='\0';
  return int(len);
}

int jdk_client_socket::read_buf( jdk_buf &buf, int maxlen_ )
{
    char tmp[4096];
    int maxlen = maxlen_;
    if( maxlen>4096 )
      maxlen=4096;
    if( maxlen<=0 )
      return 0;
  
    int res=read_data( tmp, maxlen );
    if( res>0 )
    {
      buf.append_from_data( tmp, res );
    }
  return res;
}		  

int	 jdk_client_socket::read_string(
  void *str_,
  size_t max_len,
  const char *terminators,
  const char *ignores
  ) 
{
  size_t len=0;
  char *str = (char *)str_;
  
  // only read max_len characters.
  
  while(is_open_for_reading() && len<max_len-1 )
  {
    char c;
    
    // read one character
    
    int cnt = read_data( &c, 1 );
    
    // did we get one?
    
    if( cnt==1 )
    {
      // yes
      
      // was it an ignore character?
      
      if( ignores )
      {
        if( strchr( ignores, c ) )
        {
          // yes it was
          // ignore this character. try read the next character.
          continue;
        }
      }
      
      // was it a terminator?
      
      if( terminators )
      {
        if( strchr( terminators, c ) )
        {
          // yes it was a terminator. return the string as is.
          break;
        }
        
      }
      
      
      // anything else is appended to our string
      
      *str++ = c;
      ++len;
    }
    else if( cnt<=0 )
    {
      read_failed();
      return -1;
    }
  }		
  
  // put null on end of c string
  *str='\0';
  return int(len);
}


int  jdk_client_socket::read_partial_string(
  void *str_,
  size_t max_len,
  size_t *cur_pos,
  const char *terminators,
  const char *ignores
  ) 
{
  char *str = ((char *)str_)+(*cur_pos);
  
  // only read max_len characters.
  
  while(is_open_for_reading() && *cur_pos<max_len-1 )
  {
    jdk_select_manager mgr;
    
    // can we read from the socket?
    mgr.add_read( this );
    mgr.set_timeout( 0 );
    mgr.do_select();
    
    if( !mgr.can_read( this ) )
    {
      // no characters to read yet!
      return -1;	
    }		
    
    
    char c;
    
    // read one character
    
    int cnt = read_data( &c, 1 );
    
    // did we get one?
    
    if( cnt<=0 )
    {
      read_failed();
      
      if( *cur_pos==0 )
      {
        // we did not read any thing, so return -1
        return -1;
      }
      else
      {
        // return the current line.
        break;
      }
    }
    
    
    if( cnt==1 )
    {
      // yes
      
      // was it an ignore character?
      
      if( ignores )
      {
        if( strchr( ignores, c ) )
        {
          // yes it was
          // ignore this character. try read the next character.
          continue;
        }
      }
      
      // was it a terminator?
      
      if( terminators )
      {
        if( strchr( terminators, c ) )
        {
          // yes it was a terminator. return the string as is.
          break;
        }
        
      }
      
      
      // anything else is appended to our string
      
      *str++ = c;
      ++(*cur_pos);
    }
    else
    {
      // we did not read a character. return -1 to say we are not done yet.
      return -1;	
    }
    
  }		
  
  // put null on end of c string
  *str='\0';
  
  size_t len=0;
  
  // reset our current position to be ready for next string
  len = *cur_pos;
  *cur_pos = 0;
  
  // return the length of the full string
  return int(len);
} 

int  jdk_client_socket::write_data_block( const void *buf_, size_t len ) 
{
  size_t todo=len;
  int cnt=0;
  const unsigned char *buf = (const unsigned char *)buf_;
  
  
  while( is_connected() && todo>0 )
  {
    int c=write_data( buf, todo );
    
    if( c<=0 )
    {
      write_failed();
      break;
    }
    
    cnt+=c;
    buf+=c;
    todo-=c;
  }
  
  return cnt;
}


int	 jdk_client_socket::write_string_block( const char *str ) 
{
  size_t len = strlen(str);
  
  return write_data_block(str,len);
}


int	 jdk_client_socket::print( const char *fmt, ... ) 
{
  char s[4096];
  va_list ap;
  
  va_start(ap, fmt);
#if JDK_IS_WIN32
  jdk_vsprintf(s, fmt, ap);
#else
  vsnprintf(s, sizeof(s)-1, fmt, ap);
#endif
  s[4095]=0;
  
  va_end(ap);
  
  return write_string_block( s );
} 



jdk_select_manager::jdk_select_manager()
: read_set(), write_set(), except_set(), timeout(), forever(false),max_fd(0)
{
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  FD_ZERO( &except_set );
  timeout.tv_sec=0;
  timeout.tv_usec=0;
}



void jdk_select_manager::clear() 
{
  FD_ZERO( &read_set );
  FD_ZERO( &write_set );
  FD_ZERO( &except_set );
  timeout.tv_sec=0;
  timeout.tv_usec=0;
  max_fd=0;
  forever=false;
}



void jdk_select_manager::add_all( jdk_socket_handle fd ) 
{
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &read_set );
    FD_SET( fd, &write_set );
    FD_SET( fd, &except_set );
    if( fd>max_fd )
      max_fd=fd;
  }
  
}



void jdk_select_manager::add_all( jdk_socket *s ) 
{
  jdk_socket_handle fd=s->get_fd();
  if( fd!=JDK_SOCKET_ERROR )
  {		
    if( s->is_open_for_reading() )
      FD_SET( fd, &read_set );
    if( s->is_open_for_writing() )
      FD_SET( fd, &write_set );
    
    FD_SET( fd, &except_set );
    
    if( fd>max_fd )
      max_fd=fd;
  }	
}



void jdk_select_manager::add_read( jdk_socket_handle fd ) 
{
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &read_set );
    if( fd>max_fd )
      max_fd=fd;
  }	
}



void jdk_select_manager::add_read( jdk_socket *s ) 
{
  jdk_socket_handle fd=s->get_fd();
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &read_set );
    if( fd>max_fd ) 
      max_fd=fd;
  }
  
}



void jdk_select_manager::add_write( jdk_socket_handle fd ) 
{
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &write_set );
    if( fd>max_fd )
      max_fd=fd;
  }
  
}



void jdk_select_manager::add_write( jdk_socket *s ) 
{
  jdk_socket_handle fd=s->get_fd();
  
  if( fd!=JDK_SOCKET_ERROR )
  {		
    if( s->is_open_for_writing() )
    {			
      FD_SET( fd, &write_set );
      if( fd>max_fd )
        max_fd=fd;
    }
    
  }
  
}



void jdk_select_manager::add_except( jdk_socket_handle fd ) 
{
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &except_set );
    if( fd>max_fd )
      max_fd=fd;
  }
  
  
}



void jdk_select_manager::add_except( jdk_socket *s ) 
{
  jdk_socket_handle fd=s->get_fd();
  
  if( fd!=JDK_SOCKET_ERROR )
  {		
    FD_SET( fd, &except_set );
    if( fd>max_fd )
      max_fd=fd;
  }
  
}



int jdk_select_manager::do_select() 
{
  if( forever )
  {
    return ::jdk_socket_lpselect( max_fd+1, &read_set, &write_set, &except_set, 0 );
  }
  else
  {
    return ::jdk_socket_lpselect( max_fd+1, &read_set, &write_set, &except_set, &timeout );   		
  }
}





jdk_fd_client_socket::jdk_fd_client_socket() : fd(JDK_SOCKET_ERROR)
{
}



jdk_fd_client_socket::~jdk_fd_client_socket() 
{
#if !JDK_IS_WIN32 // TODO: use proper winapi call
  if( fd!=JDK_SOCKET_ERROR )
  {
    ::close( fd );
    fd=JDK_SOCKET_ERROR;
  }
#endif	
}



bool jdk_fd_client_socket::is_connected() const 
{
  return fd!=JDK_SOCKET_ERROR;
}



bool jdk_fd_client_socket::is_open_for_reading() const 
{
  return is_connected();
} 

bool jdk_fd_client_socket::is_open_for_writing() const 
{
  return is_connected();
} 

jdk_socket_handle jdk_fd_client_socket::get_fd() const 
{
  return fd;
}



void jdk_fd_client_socket::force_connection( jdk_socket_handle newfd ) 
{
#if !JDK_IS_WIN32 // TODO:	
  if( fd!=JDK_SOCKET_ERROR )
  {		
    ::close(fd);
    fd=JDK_SOCKET_ERROR;
  }
#endif
  fd=newfd;
}



void jdk_fd_client_socket::detach() 
{
  fd=JDK_SOCKET_ERROR;
} 

void jdk_fd_client_socket::close() 
{
#if !JDK_IS_WIN32 // TODO:
  if( fd!=JDK_SOCKET_ERROR )
  {
    ::close(fd);
    fd=JDK_SOCKET_ERROR;
  }
#endif
}



void jdk_fd_client_socket::shutdown_read() 
{
#if !JDK_IS_WIN32 // TODO:		
  if( fd!=JDK_SOCKET_ERROR )
  {
    ::close(fd);
    fd=JDK_SOCKET_ERROR;
  }
#endif	
}



void jdk_fd_client_socket::shutdown_write() 
{
#if !JDK_IS_WIN32 // TODO:	
  if( fd!=JDK_SOCKET_ERROR )
  {
    ::close(fd);
    fd=JDK_SOCKET_ERROR;
  }
#endif	
}


bool jdk_fd_client_socket::get_local_addr( char *addr, size_t max_len ) const 
{
  *addr=0;
  return true;
}



bool jdk_fd_client_socket::get_remote_addr( char *addr, size_t max_len ) const 
{
  *addr=0;
  return true;
}



int  jdk_fd_client_socket::read_data( void *buf, size_t len, int timeout )
{
  if( fd==JDK_SOCKET_ERROR )
    return 0;
  
  if( timeout==-1 )
    timeout=default_timeout;
  
  if( timeout!=-1 )
  {		
    jdk_select_manager mgr;
    
    // can we read from the socket?
    mgr.add_read( this );
    mgr.set_timeout( timeout );
    int sel_r = mgr.do_select();
    if( sel_r < 0 )
    {
      read_failed();
      return -1;	
    }	
    if( sel_r==0 )
    {
      read_failed();
      return 0;	
    }		
    if( !mgr.can_read( this ) )
    {
      read_failed();
      return 0;	
    }		
  }
  
  
  int cnt=::jdk_socket_lpread( fd, (char *)buf, int(len) );
#if !JDK_IS_WIN32
  if( cnt<0 && errno==EWOULDBLOCK)
    return 0;
  
  if( cnt<0 && errno==0 )
  {
    return 0;
  }
#endif
  
  if( cnt<=0 )
  {
    read_failed();
    return 0;
  }
  
  return cnt;
}



int  jdk_fd_client_socket::write_data( const void *buf, size_t len, int timeout_seconds )
{
  if( fd==JDK_SOCKET_ERROR )
    return 0;
  
  int cnt= ::jdk_socket_lpwrite( fd, (const char *)buf, int(len) );
  
#if !JDK_IS_WIN32
  if( cnt<0 && errno==EWOULDBLOCK )
    return 0;
  
  if( cnt<0 && errno==0)
    return 0;
#endif
  if( cnt<=0 )
  {
    write_failed();
    return 0;
  }
  
  return cnt;
}



void jdk_fd_client_socket::read_failed()
{
  
}

void jdk_fd_client_socket::write_failed()
{
}



jdk_inet_client_socket::jdk_inet_client_socket()
:
  sock_id( JDK_SOCKET_ERROR ),
  connected(false),
  readable(false),
  writable(false)
{  
#if 0
  struct linger ling;
  ling.l_onoff=1;
  ling.l_linger=120;
  setsockopt(sock_id,SOL_SOCKET,SO_LINGER,&ling,sizeof(ling) );
#endif

}



jdk_inet_client_socket::~jdk_inet_client_socket()
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
#if JDK_HAS_SSL
    if( use_ssl )
    {
      SSL_shutdown(ssl);		   
      SSL_free(ssl);
      SSL_CTX_free(ssl_ctx);
    }
#endif
    ::closesocket( sock_id );
  }
  
}




void jdk_inet_client_socket::setup_socket_blocking(bool blocking)
{
  if( !blocking )
  {
    // select non-blocking mode
#if JDK_IS_WIN32 && !JDK_IS_WINE
    unsigned long v =1;
    ioctlsocket( sock_id, FIONBIO, &v );
#elif defined(O_NONBLOCK) && !JDK_IS_SUNOS && !JDK_IS_SOLARIS
    int flags;
    int dummy = 0;
    
    if ((flags = fcntl(sock_id, F_GETFL, dummy)) < 0)
    {
      close();
      return;
    }
    
    fcntl( sock_id, F_SETFL, flags | O_NONBLOCK );
#else
    int flags;
    int dummy = 0;
    
    if ((flags = fcntl(sock_id, F_GETFL, dummy)) < 0)
    {
      close();
      return;
    }
    
    fcntl( sock_id, F_SETFL, flags | O_NDELAY );
#endif
  }
  else
  {
    
    // select blocking mode
#if JDK_IS_WIN32 && !JDK_IS_WINE
    unsigned long v =0;
    ioctlsocket( sock_id, FIONBIO, &v );
#elif defined(O_NONBLOCK) && !JDK_IS_SUNOS && !JDK_IS_SOLARIS
    int flags;
    int dummy = 0;
    
    if ((flags = fcntl(sock_id, F_GETFL, dummy)) < 0)
    {
      close();
      return;
    }
    
    fcntl( sock_id, F_SETFL, flags & ~O_NONBLOCK );
#else
    int flags;
    int dummy = 0;
    
    if ((flags = fcntl(sock_id, F_GETFL, dummy)) < 0)
    {
      close();
      return;
    }
    
    fcntl( sock_id, F_SETFL, flags & ~O_NDELAY );
    
#endif
  }
  
}



bool jdk_inet_client_socket::is_connected() const 
{
  return sock_id != JDK_SOCKET_ERROR;
}



bool jdk_inet_client_socket::is_open_for_reading() const 
{
  return is_connected() && readable;
} 

bool jdk_inet_client_socket::is_open_for_writing() const 
{
  return is_connected() && writable;
} 

jdk_socket_handle jdk_inet_client_socket::get_fd() const 
{
  return sock_id;
}


int  jdk_inet_client_socket::send_data( const void *buf, size_t len ) 

{ 

   return ::jdk_socket_lpsend(sock_id,buf,len,0); 

}



int  jdk_inet_client_socket::recv_data( void *buf, size_t len ) 

{ 

      return ::jdk_socket_lprecv(sock_id,buf,len,0); 

}


bool jdk_inet_client_socket::make_connection_from( 
  const char *hostname, int port, 
  const char *bindname, int bindport,
  jdk_dns_cache *cache,
  bool use_ssl_
  ) 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    // if we already had a socket, close it first.
    ::closesocket(sock_id);	 
    sock_id=JDK_SOCKET_ERROR;		
  }
  
  // open a socket
  sock_id = ::jdk_socket_lpsocket(AF_INET, SOCK_STREAM, 0);
  
  if( sock_id==JDK_SOCKET_ERROR )
  {
    return false;	// didnt work!
  }
  
  int flag = 1;	  
  setsockopt ( sock_id, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag) );
  
  // bind it to the specified address
  struct sockaddr_in server_address;
  
  int len = sizeof(server_address);
  
  memset((char*) &server_address, 0, len);
  
  // null, blank, or "0" means listen to all local ips
  // ip address or hostname must be converted
  
  if( bindname && *bindname && strcmp(bindname,"0")!=0 )
  {		
    jdk_gethostbyname( bindname, &server_address );
  }
  else
  {
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(bindport);
  
  if (::bind(sock_id, (sockaddr *)&server_address, len) != 0)
  {
    // bind failed. close socket and return false.
    
    ::closesocket(sock_id);
    sock_id=JDK_SOCKET_ERROR;
    return false;
  }
  
  
  readable=true;
  writable=true;
  
  // set socket options for the socket
  setup_socket();
  
  // get the remote address via the dns cache
  
  struct sockaddr_in my_address;
  
  if( cache )
  {
    if( cache->gethostbyname(hostname,&my_address)<0 )
    {
      // cant find host
      return false;
    }
  }
  else
  {
    if( jdk_gethostbyname(hostname, &my_address )<0 )
    {
      // cant find host
      return false;
    }
  }
  
  my_address.sin_port = htons( port );
  
  // try connect
  
  if( ::jdk_socket_lpconnect(
        sock_id,
        (struct sockaddr *)&my_address,
        sizeof(my_address))==SOCKET_ERROR
      && (errnum()!=EINPROGRESS) // allow nonblocking connect to succeed at this point
    )
  {
    // connect failed. close the socket and return.
    
    close();
    
    return false;
  }
  
  // connection succeeded, initialize ssl if needed
  
#if JDK_HAS_SSL
  if( use_ssl_ )
  {
    use_ssl = true;
    
    ssl_client_method = SSLv2_client_method();
    ssl_ctx = SSL_CTX_new(ssl_client_method);
    use_ssl=false;
    
    SSL_set_fd(ssl, sock_id );
    SSL_connect( ssl );
//	    X509 *ssl_server_cert;
//		ssl_server_cert = SSL_get_peer_certificate(ssl);
//      X509_free( ssl_server_cert );
  }
#endif
  
  return true;
  
}

bool jdk_inet_client_socket::make_connection( 
  const char *hostname, 
  int port, 
  jdk_dns_cache *cache, 
  bool use_ssl_
  ) 
{
  
  struct sockaddr_in my_address;
  
  
  close();
  
  // make a socket
  
  sock_id = ::jdk_socket_lpsocket( AF_INET, SOCK_STREAM, 0 );
  
  if( sock_id==JDK_SOCKET_ERROR )
  {
    return false;
  }
  
  readable=true;
  writable=true;
  
  // set socket options for the socket
  setup_socket();
  
  // get the remote address via the dns cache
  
  if( cache )
  {
    if( cache->gethostbyname(hostname,&my_address)<0 )
    {
      // cant find host
      return false;
    }
  }
  else
  {
    if( jdk_gethostbyname(hostname, &my_address )<0 )
    {
      // cant find host
      return false;
    }
  }
#if JDK_IS_MACOSX
  extern bool jdk_socket_override_filtering;
  if( jdk_socket_override_filtering )
  {
    // do magic handshake with driver
    struct sockaddr_in from;
    int len = sizeof( struct sockaddr_in );
    from.sin_family = AF_INET;
    from.sin_addr.s_addr = htonl(0x00000001);
    from.sin_port = htons(1);
    bind( sock_id, (struct sockaddr*)&from, len );
    from.sin_port = htons(0);
    bind( sock_id, (struct sockaddr*)&from, len );
  }
#endif
  my_address.sin_port = htons( port );
  
  // try connect
  
  if( ::jdk_socket_lpconnect(
        sock_id,
        (struct sockaddr *)&my_address,
        sizeof(my_address))==SOCKET_ERROR
      && (errnum()!=EINPROGRESS) // allow nonblocking connect to succeed at this point
    )
  {
    // connect failed. close the socket and return.
    
    close();
    
    return false;
  }
  
  // connection succeeded, initialize ssl if needed
  
#if JDK_HAS_SSL
  if( use_ssl_ )
  {
    use_ssl = true;
    
    ssl_client_method = SSLv2_client_method();
    ssl_ctx = SSL_CTX_new(ssl_client_method);
    use_ssl=false;
    
    SSL_set_fd(ssl, sock_id );
    SSL_connect( ssl );
//	    X509 *ssl_server_cert;
//		ssl_server_cert = SSL_get_peer_certificate(ssl);
//      X509_free( ssl_server_cert );
  }
#endif
  
  return true;
}





void jdk_inet_client_socket::force_connection( jdk_socket_handle fd ) 
{
  close();
  
  sock_id=fd;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {		
    readable=true;
    writable=true;
    
    // set socket options for the socket
    setup_socket();
  }	
}


bool jdk_inet_client_socket::get_local_addr( struct sockaddr_in *addr ) const 
{
  JDK_SOCKADDRLENTYPE len;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    len=sizeof(*addr);
    int r=::getsockname( sock_id, (struct sockaddr *)addr, &len );
    if( r==SOCKET_ERROR )
      return false;						
    return true;
  }
  
  return false;	
}

bool jdk_inet_client_socket::get_remote_addr( struct sockaddr_in *addr ) const 
{
  JDK_SOCKADDRLENTYPE len;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    len=sizeof(*addr);
    int r=::getpeername( sock_id, (struct sockaddr *)addr, &len );
    if( r==SOCKET_ERROR )
      return false;						
    return true;
  }
  
  return false;	
}



bool jdk_inet_client_socket::get_local_addr( char *addr, size_t max_len ) const 
{
  JDK_SOCKADDRLENTYPE len;
  struct sockaddr_in local_addr;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    len=sizeof(local_addr);
    int r=::getsockname( sock_id, (struct sockaddr *)&local_addr, &len );
    if( r==SOCKET_ERROR )
      return false;
    
    
    unsigned long ip = ntohl( local_addr.sin_addr.s_addr );
    unsigned short port = ntohs( local_addr.sin_port );
    
    //::strncpy( addr, inet_ntoa(local_addr.sin_addr), max_len );
    jdk_sprintf( addr, "%ld.%ld.%ld.%ld:%u", 
             (ip>>24)&0xff,
             (ip>>16)&0xff,
             (ip>>8)&0xff,
             (ip>>0)&0xff,
             port
      );
    
    return true;
  }
  
  return false;	
}



bool jdk_inet_client_socket::get_remote_addr( char *addr, size_t max_len ) const
{
  JDK_SOCKADDRLENTYPE len;
  struct sockaddr_in remote_addr;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    len=sizeof(remote_addr);
    int r=::getpeername( sock_id, (struct sockaddr *)&remote_addr, &len );
    if( r==SOCKET_ERROR )
      return false;
    
    
    unsigned long ip = ntohl( remote_addr.sin_addr.s_addr );
    unsigned short port = ntohs( remote_addr.sin_port );
    
    //::strncpy( addr, inet_ntoa(remote_addr.sin_addr), max_len );
    
    jdk_sprintf( addr, "%ld.%ld.%ld.%ld:%u",
             (ip>>24)&0xff,
             (ip>>16)&0xff,
             (ip>>8)&0xff,
             (ip>>0)&0xff,
             port
      );
    
    return true;
  }
  
  return false;
}



void jdk_inet_client_socket::detach()
{
  sock_id=JDK_SOCKET_ERROR;
  readable=false;
  writable=false;
}

void jdk_inet_client_socket::close()
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
#if JDK_HAS_SSL
    if( use_ssl )
    {
      SSL_shutdown(ssl);		   
      SSL_free(ssl);
      SSL_CTX_free(ssl_ctx);
    }
#endif
    ::closesocket(sock_id);
    sock_id=JDK_SOCKET_ERROR;
    readable=false;
    writable=false;
    
  }
}



void jdk_inet_client_socket::shutdown_read() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    jdk_log( JDK_LOG_DEBUG3, "shutdown read" );
    ::shutdown(sock_id,0);
    readable=false;
  }
}



void jdk_inet_client_socket::shutdown_write() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    ::shutdown(sock_id,1);
    jdk_log( JDK_LOG_DEBUG3, "shutdown write" );
    writable=false;
  }
}


void jdk_inet_client_socket::read_failed() 
{
  jdk_log( JDK_LOG_DEBUG3, "read failed" );
  readable=false;	
} 

void jdk_inet_client_socket::write_failed() 
{
  jdk_log( JDK_LOG_DEBUG3, "write failed" );
  writable=false;
} 

int  jdk_inet_client_socket::read_data( void *buf, size_t len, int timeout )
{
  if( sock_id==JDK_SOCKET_ERROR )
    return 0;
  
  if( timeout==-1 )
    timeout=default_timeout;
  if( timeout!=-1 )	  
  {		
    jdk_select_manager mgr;
    
    // can we read from the socket?
    mgr.add_read( this );
    mgr.set_timeout( timeout );
    int sel_r = mgr.do_select();
    if( sel_r < 0 )
    {
      read_failed();
      return -1;	
    }	
    if( sel_r==0 )
    {
      read_failed();
      return 0;	
    }		
    if( !mgr.can_read( this ) )
    {
      read_failed();
      return 0;	
    }		
  }
  
#if JDK_HAS_SSL
  int cnt=-1;
  if( use_ssl )
  {
    cnt=SSL_read(ssl, (char *)buf, len );
  }
  else
  {
    cnt=::jdk_socket_lprecv( sock_id, (char *)buf, len, 0 );
  }
#else	
  int cnt=::jdk_socket_lprecv( sock_id, (char *)buf, int(len), 0 );
#endif
  
//	if( cnt<0 )
//		perror( "\nrecv: " );
  
  if( cnt==0 )
    jdk_log( JDK_LOG_DEBUG3, "read_data fd %d returning 0", sock_id );
  else
    jdk_log( JDK_LOG_DEBUG4, "read_data fd %d returning %d", sock_id, cnt );
  
  
  
  if( cnt<0 && errno==EWOULDBLOCK)
    return 0;
  
  
  if( cnt<=0 )
  {
    read_failed();
    return 0;
  }
  
  return cnt;
}



int  jdk_inet_client_socket::write_data( const void *buf, size_t len,int timeout_seconds )
{
  if( sock_id==JDK_SOCKET_ERROR )
    return 0;
  
  int timeout=default_timeout;
  
  jdk_select_manager mgr;
  
  
  if( timeout==-1 )
    mgr.set_timeout_forever();
  else
    mgr.set_timeout( timeout );
  
  // can we write to the socket?
  mgr.add_write( this );
  
  int sel_r = mgr.do_select();
  if( sel_r < 0 )
  {
    jdk_log( JDK_LOG_DEBUG2, "select error in write_data()" );		
    write_failed();
    return -1;	
  }	
  if( sel_r==0 )
  {
    jdk_log( JDK_LOG_DEBUG2, "timeout in write_data()" );
    write_failed();
    return 0;	
  }
  if( !mgr.can_write( this ) )
  {
    jdk_log( JDK_LOG_DEBUG2, "write_data() but select said I can't write" );
    write_failed();
    return 0;	
  }
  
  
  int cnt=-1;
  
#if JDK_HAS_SSL
  if( use_ssl )
  {
    cnt=SSL_write(ssl, (const char *)buf, len );
  }
  else
  {
    cnt=::jdk_socket_lpsend( sock_id, (const char *)buf, len, 0 );
  }
#else	
  cnt = ::jdk_socket_lpsend( sock_id, (const char *)buf, int(len), 0 );
#endif
  
  if( cnt==0 )
    jdk_log( JDK_LOG_DEBUG3, "write_data fd %d returning 0", sock_id );
  else
    jdk_log( JDK_LOG_DEBUG4, "write_data %d returning %d", sock_id, cnt );
  
  if( cnt<0 && errno==EWOULDBLOCK )
    return 0;
  
  if( cnt<0 )
  {
    write_failed();
    return 0;
  }
  
  return cnt;
}



void jdk_inet_client_socket::setup_socket()
{
  setup_socket_blocking(true);
}




jdk_nb_inet_client_socket::jdk_nb_inet_client_socket()
{
}


jdk_nb_inet_client_socket::~jdk_nb_inet_client_socket()
{
  
}


int jdk_nb_inet_client_socket::get_connection_status()
{
  if( sock_id==JDK_SOCKET_ERROR )
  {
    // we dont have a socket so definitely the connection failed
    
    
    return -1;
  }
  else
  {
    // ok, we have a socket. call getsockopt to find out if the connection succeeded yet.
    
    int optval;
    size_t optlen=sizeof(optval);
    
    if( getsockopt(
          sock_id,
          SOL_SOCKET,
          SO_ERROR,
          (JDK_GETSOCKOPTVALTYPE)&optval,
          (JDK_SOCKADDRLENTYPE *)&optlen
          )==0 )
    {
      // ok we got a response in optval.
      
      if( optval==EINPROGRESS )
      {
        return 1; // still connecting!
      }
      
      if( optval==EISCONN || optval==0 )
      {
        readable=true;
        writable=true;
        return 0; // we are connected!
      }
      
      // any other value means failure.
      
      close();
      return -1;
    }
    else
    {
      // getting sock opt failed - close socket and return -1 for err
      
      close();
      return -1;
    }
  }
}


void jdk_nb_inet_client_socket::setup_socket()
{
  setup_socket_blocking(false);
#if 0	
  struct linger ling;
  ling.l_onoff=1;
  ling.l_linger=120;
  setsockopt(sock_id,SOL_SOCKET,SO_LINGER,&ling,sizeof(ling) );
#endif
}




jdk_server_socket::~jdk_server_socket()
{
}


jdk_inet_server_socket::jdk_inet_server_socket() :
    server_address(),
    sock_id(JDK_SOCKET_ERROR)
{
}



jdk_inet_server_socket::~jdk_inet_server_socket() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    ::closesocket(sock_id);
    sock_id=JDK_SOCKET_ERROR;
  }	
}



bool jdk_inet_server_socket::is_open_for_reading() const 
{
  return sock_id!=JDK_SOCKET_ERROR;
} 

bool jdk_inet_server_socket::is_open_for_writing() const 
{
  return false;
} 

bool jdk_inet_server_socket::bind_socket( 
  int port, 
  int max_connect, 
  const char *ip_addr_string
  ) 
{
  int len;
  
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    // if we already had a socket, close it first.
    ::closesocket(sock_id);	 
    sock_id=JDK_SOCKET_ERROR;		
  }
  
  // open a socket
  sock_id = ::jdk_socket_lpsocket(AF_INET, SOCK_STREAM, 0);
  
  if( sock_id==JDK_SOCKET_ERROR )
  {
    return false;	// didnt work!
  }
  
  int flag = 1;	  
  setsockopt ( sock_id, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag) );
  
  // bind it to the specified address
  len = sizeof(server_address);
  
  memset((char*) &server_address, 0, len);
  
  // null, blank, or "0" means listen to all local ips
  // ip address or hostname must be converted
  
  if( ip_addr_string && *ip_addr_string && strcmp(ip_addr_string,"0")!=0 )
  {		
    jdk_gethostbyname( ip_addr_string, &server_address );
  }
  else
  {
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  
  if (::bind(sock_id, (sockaddr *)&server_address, len) != 0)
  {
    // bind failed. close socket and return false.
    
    ::closesocket(sock_id);
    sock_id=JDK_SOCKET_ERROR;
    return false;
  }
  
  // tell tcpip how many connection backlog we want
  ::listen(sock_id, max_connect);
  
  // we have a bound socket!
  return true;
}



jdk_socket_handle jdk_inet_server_socket::get_fd() const
{
  return sock_id;
}



bool jdk_inet_server_socket::accept( jdk_client_socket *s )
{
  if( sock_id==JDK_SOCKET_ERROR )
    return false;
  
  JDK_SOCKADDRLENTYPE len;
  jdk_socket_handle connect_id;
  struct sockaddr_in user_address;
  
  
  // try accept the connection
  
  len = sizeof( user_address );
  connect_id = ::jdk_socket_lpaccept( sock_id, (sockaddr *)&user_address, &len );
  
  // did it work?
  
  if( connect_id!=JDK_SOCKET_ERROR )
  {
    // yes, force the passed socket object to be connected to this fd.
    
    s->force_connection( connect_id );
    
    return true;
  }
  else
  {
    // no, return false
    
    return false;
  }	
  
}



jdk_client_socket * jdk_inet_server_socket::accept_create() 
{
  if( sock_id==JDK_SOCKET_ERROR )
    return 0;
  
  JDK_SOCKADDRLENTYPE len;
  jdk_socket_handle connect_id;
  struct sockaddr_in user_address;
  
  
  // try accept the connection
  
  len = sizeof( user_address );
  connect_id = ::jdk_socket_lpaccept( sock_id, (sockaddr *)&user_address, &len );
  
  if( connect_id==JDK_SOCKET_ERROR )
  {
    return 0;
  }
  else
  {
    jdk_client_socket *s = new jdk_inet_client_socket;
    s->force_connection( connect_id );
    return s;
  }
}



void jdk_inet_server_socket::close() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {		
    ::closesocket( sock_id );
    sock_id=JDK_SOCKET_ERROR;
  }
}



void jdk_inet_server_socket::shutdown_read() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    ::shutdown(sock_id,0);
  }
}



void jdk_inet_server_socket::shutdown_write() 
{
  if( sock_id!=JDK_SOCKET_ERROR )
  {
    ::shutdown(sock_id,1);
  }
}






jdk_nb_inet_server_socket::~jdk_nb_inet_server_socket()
{
} 


bool jdk_nb_inet_server_socket::bind_socket(
  int port, 
  int max_connect, 
  const char *ip_addr_string
  ) 
{
  if( jdk_inet_server_socket::bind_socket( port, max_connect, ip_addr_string ) )
  {
    // select non-blocking mode
#if JDK_IS_WIN32 && !JDK_IS_WINE
    unsigned long v =1;
    ioctlsocket( sock_id, FIONBIO, &v );
#else
    fcntl( sock_id, F_SETFL, O_NONBLOCK );
#endif
    return true;
  }
  return false;
}



jdk_client_socket * jdk_nb_inet_server_socket::accept_create() 
{
  if( sock_id==JDK_SOCKET_ERROR )
    return 0;
  
  JDK_SOCKADDRLENTYPE len;   
  jdk_socket_handle connect_id;
  struct sockaddr_in user_address;
  
  
  // try accept the connection
  
  len = sizeof( user_address );
  connect_id = ::jdk_socket_lpaccept( sock_id, (sockaddr *)&user_address, &len );
  
  if( connect_id==JDK_SOCKET_ERROR )
  {
    return 0;
  }
  else
  {
    jdk_client_socket *s = new jdk_nb_inet_client_socket;
    s->force_connection( connect_id );
    return s;
  }
}




#if 0
jdk_file_socket::jdk_file_socket( 
  const char *fname, 
  int open_flags, 
  int mode
  )
{
  open( fname, open_flags, mode );
}


jdk_file_socket::~jdk_file_socket()
{
}

bool jdk_file_socket::open(								 
  const char *fname, 
  int open_flags, 
  int mode
  ) 
{
  close();
  
  fd = ::open( fname, open_flags, mode);
  
  return fd!=JDK_SOCKET_ERROR;
} 

bool jdk_file_socket::is_open_for_reading() const 
{
  return (fd!=JDK_SOCKET_ERROR) && ((open_flags&O_RDONLY) || (open_flags&O_RDWR));
}

bool jdk_file_socket::is_open_for_writing() const
{
  return (fd!=JDK_SOCKET_ERROR) && ((open_flags&O_WRONLY) || (open_flags&O_RDWR));
} 

jdk_int64 jdk_file_socket::seek(
  jdk_int64 pos,
  int whence
  ) 
{
#if !JDK_IS_WIN32 || JDK_IS_WINE	
  if( fd!=JDK_SOCKET_ERROR )
  {	
    return lseek( fd, pos, whence );
  }
  else
#endif	  
  {	
    return (jdk_int64)-1;
  }
  
} 

bool jdk_file_socket::make_connection( const char *hostname, int port, jdk_dns_cache *cache, bool use_ssl ) 
{
  return false;
} 

void jdk_file_socket::shutdown_write() 
{
} 

void jdk_file_socket::shutdown_read()
{
}

#endif



jdk_socket_task::jdk_socket_task()
{
}

jdk_socket_task::~jdk_socket_task()
{
}



jdk_socket_task_manager::jdk_socket_task_manager( int max_tasks_  )
  :
  max_tasks(max_tasks_),
  task_list( new jdk_socket_task *[max_tasks] )
{
  for( int i=0; i<max_tasks; ++i )
  {
    task_list[i] = 0;	
  }
  
}

jdk_socket_task_manager::~jdk_socket_task_manager()
{
  for( int i=0; i<max_tasks; ++i )
  {
    delete task_list[i];
    task_list[i]=0;
  }
  delete [] task_list;
}


bool jdk_socket_task_manager::add_task( jdk_socket_task *task )
{
  for( int i=0; i<max_tasks; ++i )
  {
    if( task_list[i]==0 )
    {
      task_list[i] = task;
      task->start();
      return true;
    }		
  }
  
  return false;
}

bool jdk_socket_task_manager::remove_task( jdk_socket_task *task )
{
  for( int i=0; i<max_tasks; ++i )
  {
    if( task_list[i]==task )
    {
      task->stop();
      delete task;
      task_list[i] = 0;
      return true;
    }		
  }
  
  return false;
  
}


void jdk_socket_task_manager::poll( int timeout_secs )
{
  jdk_select_manager sel;
  
  sel.set_timeout( timeout_secs );
  
  
  for( int i=0; i<max_tasks; ++i )
  {
    if( task_list[i] && !task_list[i]->is_done() )
    {
      task_list[i]->prepare( &sel );	
    }		
  }
  
  if( sel.do_select()>0 )
  {
    for( int i=0; i<max_tasks; ++i )
    {
      if( task_list[i] && !task_list[i]->is_done() )
      {
        task_list[i]->dispatch( &sel );
      }		
    }
  }		
}



void jdk_single_socket_task_run( jdk_socket_task *t ) 
{
  jdk_select_manager sel;
  
  while( !t->is_done() )
  {
    sel.clear();
    sel.set_timeout( 1 );
    t->prepare( &sel );
    
    int sel_return = sel.do_select();
    
    if( sel_return >0 )	
    {
      t->dispatch( &sel );
    }
    else if( sel_return<0 )
    {
      //int i=WSAGetLastError();
      //char buf[40];
      //sprintf( buf, "%d", i);
      
      break;
    }
  }	
} 


int jdk_gethostname( char *name, int maxlen )
{
  // sigh, this has to be here for win32 so the winsock is inited before this is called
  return gethostname( name, maxlen );	
}


int jdk_gethostbyname( const char *name, char *ip, int ip_sz )
{
  struct sockaddr_in addr;
  if( jdk_gethostbyname( name, &addr )==0 )
  {
    jdk_strncpy( ip, inet_ntoa( addr.sin_addr ), ip_sz );
    return 0;
  }
  return -1;
}

int jdk_gethostbyname( const char *name, struct sockaddr_in *addr ) 
{
  unsigned long ip = inet_addr( name );
  
  if( ip!= (unsigned long)-1 )
  {
    // yup, it is an ascii IP.
    
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip;
    return 0;
  }	
  else
  {		
    return jdk_socket_lpgethostbyname( name, addr );
  }
  
}

int jdk_gethostbyaddr( const struct sockaddr_in *addr, char *name, int name_sz )
{
  struct hostent *h = gethostbyaddr((char *)addr, sizeof(*addr), AF_INET );
  if( h )
  {
    jdk_strncpy( name, h->h_name, name_sz );
    return 0;
  }
  return -1;
}

int jdk_gethostbyaddr( const char *addr, char *name, int name_sz )
{
  unsigned long ip = inet_addr( addr );
  
  if( ip!= (unsigned long)-1 )
  {
    struct hostent *h = gethostbyaddr((char *)&ip, sizeof(addr), AF_INET );
    if( h )
    {
      jdk_strncpy( name, h->h_name, name_sz );
      return 0;
    }
  }
  jdk_strncpy( name, addr, name_sz );
  return 0;
}


jdk_dns_cache::jdk_dns_cache( unsigned long max_age_ ) :
  dns_map(),
  max_age( max_age_ ),
  access_count(0),
  last_cleaning_time(0)
#if JDK_HAS_THREADS
  ,map_mutex()
#endif
{
}



int jdk_dns_cache::gethostbyname( const char *name, struct sockaddr_in *addr )
{
  int r=-1;
  // first check if it is just an ascii IP address 
  unsigned long ip = inet_addr( name );
  
  if( ip!= (unsigned long)-1 )
  {
    // yup, it is an ascii IP.
    
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip;
    r=0;	 // success
  }	
  else
  {
    // ok, it is a real domain name. Try look it up in the cache
    int pos = -1;
    {
      // do this whole block in the mutex
      jdk_synchronized( map_mutex );
      
      // first increase our access count
      access_count++;
      
      pos = dns_map.find( dns_name(name) );
      
      // did we find it?
      if( pos!=-1 )
      {
        // yup! access the map entry
        dns_info info( dns_map.getvalue( pos ) );
        
        // extract the network address
        *addr = info.addr;
        
        // increase the usage of this entry
        info.usage++;
        
        r=0; // success
      }
      
      // do garbage collection of old entries
      kill_old_entries();
    }
    // Do we still have to look it up via real dns?
    if( pos==-1 )
    {
      // yup.
      
      //fprintf( stderr, "dnscache looking up %s\n", name );
      if( jdk_gethostbyname(name,addr)==0 )
      {
        // got it from the real dns! now stick it into our cache
        
        dns_info info;				
#if JDK_HAS_THREADS
        // do this whole block in the mutex
        jdk_synchronized( map_mutex );
#endif
        // remember the time we put him into the cache
        info.birth = access_count;
        
        info.addr = *addr;
        
        // set his usage count to 0
        info.usage = 0;
        
        // add it to the map. ignore the return value - another
        // thread may have added it just before this mutex block.
        dns_map.add( dns_name(name), info );
        
        r=0; // success
      }
      else
      {
        // damn it! dns doesn't know about it. we failed to find the domain name.
        r=-1;
      }						
    }				
    
  }
  
  return r;		
}

void jdk_dns_cache::kill_old_entries()
{
  // we can only be called while the map_mutex is taken by our thread
  
  // only do anything if access_count is a multiple of max_age/10
  
  if( (access_count - last_cleaning_time)>max_age )
  {
    // WHUPS jdk_array<> has no delete!! darn it. all we can do is erase all of them. TODO:
    
    dns_map.clear();
    last_cleaning_time = access_count;
    //fprintf( stderr, "cleared dns cache, access count=%ld\n", access_count );
  }
  
}



