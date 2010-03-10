
#include "jdk_world.h"

#if JDK_HAS_RS232
#include "jdk_rs232.h"

#if !JDK_IS_WINE && (defined(WIN32) || defined( __CYGWIN__ ))

jdk_rs232::jdk_rs232()
  : rs232_handle(INVALID_HANDLE_VALUE)
{
  
}

jdk_rs232::~jdk_rs232()
{
  if( rs232_handle!=INVALID_HANDLE_VALUE )
    rs232_close();
}




int
jdk_rs232::rs232_open( const char *port, int bd )
{
  if( rs232_handle == INVALID_HANDLE_VALUE )
  {
    CloseHandle( rs232_handle );
    rs232_handle = INVALID_HANDLE_VALUE;			 
  }
  
  DCB dcb;
  char modebuf[128];
  
  rs232_handle=CreateFileA(
    port,
    GENERIC_READ | GENERIC_WRITE,
    0, // share mode
    0, // security attributes
    OPEN_EXISTING,
    0,
    0
    );
  
  if( rs232_handle==INVALID_HANDLE_VALUE )
    return false;
  
  
  SetupComm( rs232_handle, 32768, 32768 );
  
  
  jdk_sprintf( modebuf, "%s baud=%d parity=N data=8 stop=1",
           port, bd );
  
  dcb.DCBlength = sizeof( DCB );
  GetCommState( (void *)rs232_handle, &dcb );
  
  if( BuildCommDCBA( modebuf, &dcb )==false )
    return false;
  
  dcb.fBinary = 1;
  dcb.fOutX = 0;
  dcb.fInX = 0;
  dcb.fErrorChar = 0xfe;
  dcb.fTXContinueOnXoff = 0;
  dcb.fOutxCtsFlow = 0;
  dcb.fOutxDsrFlow = 0;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fNull = 0;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.fAbortOnError = 0;
  
  
  if( SetCommState( rs232_handle, &dcb )==FALSE )
  {
    int e=GetLastError();
    
    {
      LPVOID lpMsgBuf;
      
      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );
      
      // Display the string.
      MessageBoxA( NULL, (char *)lpMsgBuf, "Error setting COMM state", MB_OK|MB_ICONINFORMATION );
      
      // Free the buffer.
      LocalFree( lpMsgBuf );
      
    }
    
    CloseHandle( rs232_handle );
    rs232_handle=INVALID_HANDLE_VALUE;
    return false;
  }
  
  
  
  {
    COMMTIMEOUTS tmout;
    
    tmout.ReadIntervalTimeout=1;
    tmout.ReadTotalTimeoutMultiplier=1;
    tmout.ReadTotalTimeoutConstant=1;
    tmout.WriteTotalTimeoutMultiplier=0;
    tmout.WriteTotalTimeoutConstant=0;
    
    if( SetCommTimeouts( rs232_handle, &tmout )==false )
    {
      CloseHandle( rs232_handle );
      rs232_handle=INVALID_HANDLE_VALUE;
      return false;
    } 
  }
  return true;
}



void 
jdk_rs232::rs232_close( void )
{
  if( rs232_handle!=INVALID_HANDLE_VALUE )
  {
    Sleep(100);
    CloseHandle( rs232_handle );
    rs232_handle = INVALID_HANDLE_VALUE;
  }
}



int
jdk_rs232::rs232_in( void )
{
  char buf;
  
  if( rs232_handle!=INVALID_HANDLE_VALUE )
  {
    DWORD actual_read;
    
    if( ReadFile(
          rs232_handle,
          &buf,
          1,
          &actual_read,
          0) )
    {
      if (actual_read==1)
      {
        return ((int)buf)&0xff;
      }			 
    }
  }
  
  return -1;
}

int  jdk_rs232::rs232_read( void *buf, size_t len )
{
  if( rs232_handle!=INVALID_HANDLE_VALUE )
  {
    DWORD actual_read;
    
    if( ReadFile(
          rs232_handle,
          buf,
          len,
          &actual_read,
          0 ) )
    {
      return actual_read;
    }
    else
    {
      return 0;
    }
  }
  
  return 0;
}

int  jdk_rs232::rs232_write( const void *buf, size_t len )
{
  if( rs232_handle!=INVALID_HANDLE_VALUE )
  {
    DWORD actual_write;
    
    if( WriteFile(
          rs232_handle,
          buf,
          len,
          &actual_write,
          0 ) )
    {
      return actual_write;
    }
  }
  return 0;
}


int jdk_rs232::rs232_out( int c )
{
  unsigned char d=c;
  
  return rs232_write( &d, 1 );
}

int  jdk_rs232::rs232_out_string( const char *s )
{
  size_t len = strlen(s);
  return rs232_write( s, len )==len;
}

int
jdk_rs232::rs232_stat( void )
{
  
  return 0;
}


void 
jdk_rs232::rs232_clear( int secs )
{
  unsigned char dummy;
  Sleep( secs*1000 );
  while( rs232_stat() )
  {
    rs232_read( &dummy, 1 );
  }
  
}

#else

#include <termios.h>
#include <fcntl.h>

jdk_rs232::jdk_rs232()
  : rs232_handle(-1)
{
  
}

jdk_rs232::~jdk_rs232()
{
  if( rs232_handle!=-1 )
    rs232_close();
}




int
jdk_rs232::rs232_open( const char *port, int bd )
{
  rs232_handle=open(port,O_RDWR | O_NOCTTY);
  
  if( rs232_handle<0 )
  {
    //fprintf( stderr, "Error opening serial port: %s\n", 
    //	strerror( errno ) );
    return 0;
  }
  else
  {
    struct termios t;
    
    tcgetattr( rs232_handle, &t );
    
    switch( bd )
    {
    case 1200:
      cfsetospeed( &t, B1200 );
      cfsetispeed( &t, B1200 );
      break;
    case 38400:
      cfsetospeed( &t, B38400 );
      cfsetispeed( &t, B38400 );
      break;
    case 57600:
      cfsetospeed( &t, B57600 );
      cfsetispeed( &t, B57600 );
      break;
    case 115200:
      cfsetospeed( &t, B115200 );
      cfsetispeed( &t, B115200 );
      break;
    case 19200:
      cfsetospeed( &t, B19200 );
      cfsetispeed( &t, B19200 );
      break;
      
    default:
    case 9600:
      cfsetospeed( &t, B9600 );
      cfsetispeed( &t, B9600 );
      break;
    }
    t.c_lflag &= ~(ICANON | ECHO | ECHOE 
                   | ECHOK | ECHONL | ISIG 
                   | IEXTEN );
    t.c_iflag  &= ~(INPCK | ISTRIP | IGNCR | 
                    ICRNL | INLCR | IXOFF | IXON | IMAXBEL );
    t.c_iflag  |= ( IGNBRK );
    
    t.c_cflag &= ~( HUPCL | PARENB | CRTSCTS | CSIZE );
    t.c_cflag |= ( CS8 | CLOCAL );
    
    t.c_oflag &= ~OPOST;
    
    
    
    tcsetattr( rs232_handle,TCSANOW, &t );
    
    return 1;
  }
}


void 
jdk_rs232::rs232_close( void )
{
  
  if( rs232_handle!=-1 )
  {
    usleep(100000);
    tcdrain( rs232_handle );
    close( rs232_handle );
    rs232_handle = -1;
  }
}



int
jdk_rs232::rs232_in( void )
{
  int sel_ret;
  unsigned char c;
  
  fd_set wait_read;
  timeval wait_time;
  
  /* clear all sets */
  
  FD_ZERO( &wait_read );
  FD_SET( rs232_handle, &wait_read );
  
  
  /* set time out to 2 seconds */
  
  wait_time.tv_sec = 2;
  wait_time.tv_usec = 0;
  
  
  sel_ret = select( 
    FD_SETSIZE, 
    &wait_read, 
    0, 
    0, 
    &wait_time 
    );
  
  /* if time out return -1 */
  
  if( sel_ret!=1 )
    return -1;
  
  /* if can read, then read */
  
  if( read( rs232_handle, &c, 1 ) < 0 )
  {
    return -1;
  }
  
  return (int)c;
}

int  jdk_rs232::rs232_read( void *buf, int len )
{
  int ret;
  
  ret = read( rs232_handle, buf, len );
  tcdrain( rs232_handle );
  return ret;
}

int  jdk_rs232::rs232_write( const void *buf, int len )
{
  int ret;
  
  ret = write( rs232_handle, buf, len );
  tcdrain( rs232_handle );
  return ret;
}


int jdk_rs232::rs232_out( int c )
{
  unsigned char d=c;
  
  int r=write( rs232_handle, &d, 1 );
  tcdrain( rs232_handle );
  return r;
}

int jdk_rs232::rs232_out_string( const char *s )
{
  int len=strlen(s);
  int r = (write( rs232_handle, s, len )==len);
  tcdrain( rs232_handle );
  return r;
}

int
jdk_rs232::rs232_stat( void )
{
  fd_set wait_read;
  timeval wait_time;
  
  /* clear all sets */
  
  FD_ZERO( &wait_read );
  FD_SET( rs232_handle, &wait_read );
  
  
  /* set time out to 0 seconds */
  
  wait_time.tv_sec = 0;
  wait_time.tv_usec = 0;
  
  if( select( 
        FD_SETSIZE, 
        &wait_read, 
        0, 
        0,
        &wait_time 
        )==1 )
  {
    return 1;
  }
  
  return 0;
}


void 
jdk_rs232::rs232_clear( int secs )
{
  unsigned char dummy;
  sleep( secs );
  while( rs232_stat() )
  {
    read( rs232_handle, &dummy, 1 );
  }
  
}

#endif

#endif
