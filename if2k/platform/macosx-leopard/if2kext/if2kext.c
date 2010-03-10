#include <mach/mach_types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <net/route.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <sys/fcntl.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <net/kext_net.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <libkern/OSByteOrder.h>

#include <sys/kpi_socketfilter.h>

#define IPADDR_TO_REDIRECT_TO (0x7f000001)
#define PORT_TO_REDIRECT_TO (8000)


#define DO_DEBUG 0

#ifndef DO_DEBUG
#warning no DO_DEBUG
#define DO_DEBUG 1
#endif

#if DO_DEBUG
#define if2k_debug( fmt, ... ) printf("if2k: DEBUG: %s:%d: " fmt "\n",__FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define if2k_debug_print_ntoa( a, b ) print_ntoa( __FUNCTION__,__LINE__,a,b)
#define if2k_debug_enter(fmt,...) printf("if2k: ENTER: %s:%d: " fmt "\n", __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#define if2k_debug_exit(fmt,...) printf("if2k: EXIT: %s:%d: " fmt "\n", __FUNCTION__,__LINE__, ## __VA_ARGS__ )
#else
#define if2k_debug( fmt, ... )
#define if2k_debug_print_ntoa(a,b)
#define if2k_debug_enter(fmt,...)
#define if2k_debug_exit(fmt,...)
#endif


#define if2k_error( fmt, ... ) printf("if2k ERROR:" fmt "\n", ## __VA_ARGS__ )
#define if2k_warning( fmt, ... ) printf("if2k WARNING:" fmt "\n", ## __VA_ARGS__ )

struct if2kext_cookie_t
{
  int allow_unfiltered;
};

kern_return_t if2kext_start (kmod_info_t * ki, void * d);
kern_return_t if2kext_stop (kmod_info_t * ki, void * d);
static kern_return_t if2kext_init (kmod_info_t * ki, void * d);
static kern_return_t if2kext_kill (kmod_info_t * ki, void * d);

static void if2kextsf_unregistered ( sflt_handle handle );
static errno_t if2kextsf_attach ( void **cookie, socket_t so );
static void if2kextsf_detach ( void *cookie, socket_t so );
static errno_t if2kextsf_ioctl(
    void *cookie,
    socket_t so, 
    u_int32_t request,
    const char*argp
    ); 
static errno_t if2kextsf_bind(
    void *cookie,
    socket_t so, 
    const struct sockaddr *to
    ); 
static errno_t if2kextsf_connect_out ( void *cookie, socket_t so, const struct sockaddr *to );
static errno_t if2kext_calculate_redirect( socket_t so, struct sockaddr_in *sockname, struct sockaddr_in *to );

static int active_socket_count=0;
static int unloading = 0;
static int can_be_unloaded = 1;

static struct sflt_filter if2kext_sflt =
{
  sf_handle : 0xB00B1E5,
  sf_flags : SFLT_GLOBAL,
  sf_name : "if2kext",
  sf_unregistered : if2kextsf_unregistered,
  sf_attach : if2kextsf_attach,
  sf_detach : if2kextsf_detach,
  sf_ioctl : 0,
  sf_bind : if2kextsf_bind,
  sf_connect_out : if2kextsf_connect_out
};

static inline void print_ntoa( const char *fn, int line, in_addr_t ina, in_port_t port)
{
  unsigned char *ucp = (unsigned char *)&ina;

  printf( 
    "%s:%d: %d.%d.%d.%d:%u\n",
    fn,
    line,
    ucp[0] & 0xff,
    ucp[1] & 0xff,
    ucp[2] & 0xff,
    ucp[3] & 0xff,
    ntohs(port)
    );


}


kern_return_t if2kext_start (kmod_info_t * ki, void * d) 
{
  kern_return_t	e;
  if2k_debug_enter("");
  
  e = if2kext_init(ki,d);

  if2k_debug_exit( "%d", e );
  return e;
}


kern_return_t if2kext_stop (kmod_info_t * ki, void * d) 
{
  int	e=0;
  if2k_debug_enter("");
  
  e = if2kext_kill(ki,d);

  if2k_debug_exit( "%d", e );
  return e;
}


kern_return_t if2kext_init (kmod_info_t * ki, void * d) 
{
  errno_t e;
  if2k_debug_enter("");
  
  e = sflt_register(
                    &if2kext_sflt,
                    AF_INET, 
                    SOCK_STREAM,
                    IPPROTO_TCP
                    );
  
  if( e==0 )
  {
    can_be_unloaded=0;
  }
  else
  {
    can_be_unloaded=1;
  }
  
  if2k_debug_exit( "%d", e );
  
  return e;
}

kern_return_t if2kext_kill (kmod_info_t * ki, void * d) 
{
  errno_t e=0;
  if2k_debug_enter("");
  
  if( can_be_unloaded==0 && unloading==0 )
  {
    unloading=1;
    e = sflt_unregister( if2kext_sflt.sf_handle );
  }
  
  if( e==0 )
  {
    if( can_be_unloaded==0 )
    {
      e=EINPROGRESS;
    }
  }
  
  if( e==0 && active_socket_count>0 )
  {
    e=EINPROGRESS;
  }
  if2k_debug_exit( "%d", e );
  return e;
}

static void if2kextsf_unregistered ( sflt_handle handle )
{
  if2k_debug_enter("");
  can_be_unloaded=1;
  if2k_debug_exit("");
}

static errno_t if2kextsf_attach ( void **cookie, socket_t so )
{
  struct if2kext_cookie_t *c;
  errno_t e=0;
  if2k_debug_enter("");

  (void)so;
  
  c = ( struct if2kext_cookie_t *)IOMalloc( sizeof( struct if2kext_cookie_t ) );
  c->allow_unfiltered=0;
  
  *cookie = (void *)c;
  if2k_debug( "cookie is %p", cookie );
  active_socket_count++;

  if2k_debug_exit("");
  return e;
}

static void if2kextsf_detach ( void *cookie, socket_t so )
{
  if2k_debug_enter("cookie=%p",cookie);
 
  if( cookie )
  {
    IOFree( cookie, sizeof( struct if2kext_cookie_t ) );
  }
  
  active_socket_count--;

  if2k_debug_exit("");
}




static errno_t if2kextsf_ioctl(
    void *cookie,
    socket_t so, 
    u_int32_t request,
    const char *argp
    )
{
  struct if2kext_cookie_t *c = (struct if2kext_cookie_t *)cookie;
  errno_t e=0;
  if2k_debug_enter("ioctl ****** request=%u, argp=%p", request, argp );

  // TODO: check ioctl for magic allowance
  if( c )
  {
    c->allow_unfiltered=1;
  }

  if2k_debug_exit( "%d", e );
  return e;
}


static errno_t if2kextsf_bind(
    void *cookie,
    socket_t so, 
    const struct sockaddr *to
    )
{
  struct if2kext_cookie_t *c = (struct if2kext_cookie_t *)cookie;
  errno_t e=0;
  if2k_debug_enter("******" );

  if( c && to && to->sa_family==PF_INET )
  {
    struct sockaddr_in *a = (struct sockaddr_in *)to;  
    
    if( a->sin_addr.s_addr == htonl(0x00000001)
        && a->sin_port == htons(0x0001)
        )
    {
      if( c->allow_unfiltered == 0 )
      {
        if2k_debug( "Got first special bind" );
        c->allow_unfiltered = 2;
      }
    }
    else
    {
      if( c->allow_unfiltered == 2 )
      {
        if2k_debug( "Got second special bind, handshake complete." );
        c->allow_unfiltered = 1;
      }
    }
  }

  if2k_debug_exit( "%d", e );
  return e;

}

    

static errno_t if2kextsf_connect_out ( void *cookie, socket_t so, const struct sockaddr *to )
{
  struct if2kext_cookie_t *c = (struct if2kext_cookie_t *)cookie;
  errno_t e=0;
  
  if2k_debug_enter("");
  
  if( c && c->allow_unfiltered != 1 )
  {
    if( to )
    {
      if( to->sa_family == PF_INET )
      {
        struct sockaddr_in *to_in = (struct sockaddr_in *)to;  // secretly lose const
        struct sockaddr_in sockname;
        
        if2k_debug( "to ip addr: " );
        if2k_debug_print_ntoa( to_in->sin_addr.s_addr, to_in->sin_port );
        
        e = sock_getsockname(
                               so,
                               (struct sockaddr *)&sockname,
                               sizeof(sockname)
                               );
          
        if( e==0 )
        {
          if2k_debug( "from ip addr: " );
          if2k_debug_print_ntoa( sockname.sin_addr.s_addr, sockname.sin_port );
            
          e = if2kext_calculate_redirect( so, &sockname, to_in );
        }
        else
        {
          if2k_warning( "Unable to get socket local address" );
        }
      }
    }
  }
  else
  {
    if2k_debug( "allowing unfiltered socket" );
  }
    
  
  if2k_debug_exit( "%d", e );
  return e;
}


static errno_t if2kext_calculate_redirect( socket_t so, struct sockaddr_in *in, struct sockaddr_in *to )
{
  errno_t e=0;
  int redirect_style=0;
  
  if2k_debug_enter("");
 
  // any outgoing connection that has no local port assigned or non-root port, and is going to a non-localhost address is suspect
  
  uint16_t from_port = ntohs(in->sin_port);
  uint16_t to_port = ntohs(to->sin_port);
  uint32_t to_ip = ntohl(to->sin_addr.s_addr);
   
  if( ( from_port != 666 ) && ((to_ip & 0xffffff00) != 0x7f000000) )
  {
    if2k_debug( "recognized interesting ip address: %08x", to_ip );
  
    // after that, we are interested in the following ports:
		if(
      to_port == 6346 // gnutella
      || to_port==80 // http
      || to_port==3128 // http squid proxy
      || to_port==8000 // http
      || to_port==8080 // http
      || to_port==8090 // http
      )
    {
      if2k_debug( "recognized interesting port: %u", to_port );
      redirect_style = 1;
    }
  }
   
  switch( redirect_style )
  {
  default:
  case 0: // no redirect
    if2k_debug("No redirect\n" );
    break;
    
  case 1: // redirect to local proxy
    to->sin_addr.s_addr = htonl(IPADDR_TO_REDIRECT_TO);
    to->sin_port = htons(PORT_TO_REDIRECT_TO);
    if2k_debug("Redirect to %08x:%d", to->sin_addr.s_addr, to->sin_port );
    break;
    
  case 2: // cause connection error
    e=ECONNREFUSED;
    if2k_debug("Redirect to refusal" );
    break;
  }

  if2k_debug_exit( "%d", e );
  
  return e;
}

