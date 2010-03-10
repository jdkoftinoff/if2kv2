/* -*- mode: C; mode: fold; -*- */

#include "jdk_world.h"

#include "jdk_serialsocket.h"

#if 0
#if JDK_IS_UNIX || JDK_IS_WINE
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

jdk_serial_socket::jdk_serial_socket()
{
  baud_rate=38400;	
}


jdk_serial_socket::~jdk_serial_socket()
{
  if( fd!=-1 )
  {
    tcdrain( fd );
  }	
}


bool jdk_serial_socket::set_baud_rate( unsigned long baud_rate_ )
{
  baud_rate = baud_rate_;
  return true;
}


bool jdk_serial_socket::make_connection( const char *hostname, int port, jdk_dns_cache *, bool )
{
  strncpy( fname, hostname,sizeof(fname)-1 );
  fd = open( fname, O_RDWR | O_NOCTTY );
  
  if( fd >=0 )
  {
    struct termios t;
    
    tcgetattr( fd, &t );
    
    switch( baud_rate )
    {
    case 1200:
      cfsetospeed( &t, B1200 );
      cfsetispeed( &t, B1200 );
      break;
    case 2400:
      cfsetospeed( &t, B2400 );
      cfsetispeed( &t, B2400 );
      break;			
    case 9600:
      cfsetospeed( &t, B9600 );
      cfsetispeed( &t, B9600 );
      break;			
    case 19200:
      cfsetospeed( &t, B19200 );
      cfsetispeed( &t, B19200 );
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
#ifdef B230400
    case 230400:
      cfsetospeed( &t, B230400);
      cfsetispeed( &t, B230400 );
      break;					
#endif			  
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
    
    tcsetattr( fd,TCSANOW, &t );
    
    return true;
  }
  else
  {
    return false;	
  }
  
}


int jdk_serial_socket::read_data( void *buf, int len, int timeout )
{
  return jdk_fd_client_socket::read_data( buf, len );	
}


int jdk_serial_socket::write_data( const void *buf, int len, int timeout )
{
  return jdk_fd_client_socket::write_data( buf, len );
}

void jdk_serial_socket::close()
{	
  if( fd!=-1 )
  {
    tcdrain( fd );
    jdk_fd_client_socket::close();
  }	
}

#elif JDK_IS_WIN32 && !JDK_IS_WINE

jdk_serial_socket::jdk_serial_socket()
{
  baud_rate=38400;
  read_timeout=1;
}


jdk_serial_socket::~jdk_serial_socket()
{
}


bool jdk_serial_socket::set_baud_rate( unsigned long baud_rate_ )
{
  baud_rate = baud_rate_;
  return true;
}

bool jdk_serial_socket::set_read_timeout( unsigned long t )
{
  read_timeout = t;
  return true;
}


bool jdk_serial_socket::make_connection( const char *hostname, int, jdk_dns_cache *cache, bool x  )
{
  strncpy( fname, hostname,sizeof(fname)-1 );
  
  if( fd!=JDK_SOCKET_ERROR )
  {
    close();	
  }
  
  
  fd=(int)CreateFile(
    hostname,
    GENERIC_READ | GENERIC_WRITE,
    0, // share mode
    0, // security attributes
    OPEN_EXISTING,
    0,
    0
    );
  
  if( fd==(unsigned int)INVALID_HANDLE_VALUE )
    return false;
  
  SetupComm( (void *)fd, 8192, 8192 );
  
  {
    DCB dcb;		
    char modebuf[128];		
    
    dcb.DCBlength = sizeof( DCB );		
    GetCommState( (void *)fd, &dcb );
    
    sprintf( modebuf, "baud=%ld parity=N data=8 stop=1", baud_rate );
    
    if( BuildCommDCB( modebuf, &dcb )==false )
      return false;
    
    dcb.fBinary = 1;
    dcb.fOutX = 0;
    dcb.fInX = 0;
    dcb.fErrorChar = 0;
    dcb.fTXContinueOnXoff = 0;
    dcb.fOutxCtsFlow = 0;
    dcb.fOutxDsrFlow = 0;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = 0;
    dcb.fNull = 0;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = 0;
    
    
    if( SetCommState( (void *)fd, &dcb )==FALSE )
    {
      
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
        MessageBox( NULL, (char *)lpMsgBuf, "Error setting COMM state", MB_OK|MB_ICONINFORMATION );
        
        // Free the buffer.
        LocalFree( lpMsgBuf );
        
      }
      
      CloseHandle( (void *)fd );
      fd=(int)INVALID_HANDLE_VALUE;
      return false;
    }
  }
  
  
  {
    
    COMMTIMEOUTS tmout;
    
    tmout.ReadIntervalTimeout=1;
    tmout.ReadTotalTimeoutMultiplier=1;
    tmout.ReadTotalTimeoutConstant=read_timeout;
    tmout.WriteTotalTimeoutMultiplier=0;
    tmout.WriteTotalTimeoutConstant=0;
    
    if( SetCommTimeouts( (void *)fd, &tmout )==false )
    {
      CloseHandle( (void *)fd );
      fd=(int)INVALID_HANDLE_VALUE;
      return false;
    }		
  }
  
  return true;
}

int jdk_serial_socket::wait_readable()
{
  return 0;
}

int jdk_serial_socket::wait_writable()
{
  return 0;
}

int jdk_serial_socket::read_data( void *buf, int len, int timeout_seconds )
{
  DWORD actual_read;
  
  if( ReadFile( (void *)fd, buf, len, &actual_read, 0 ))
  {
    return (int)actual_read;
  }
  else
  {
    return 0;
  }	
}


int jdk_serial_socket::write_data( const void *buf, int len, int timeout_secs )
{
  DWORD actual_write;
  
  if( WriteFile( (void *)fd, buf, len, &actual_write, 0 )  )
  {
    return (int)actual_write;	
  }
  else
  {
    return 0;	
  }
  
}



void jdk_serial_socket::close()
{
  if( fd!=JDK_SOCKET_ERROR )
  {
    CloseHandle( (void*)fd );
    fd = JDK_SOCKET_ERROR;
  }
}



#elif JDK_IS_BEOS
#warn jdk_serialsocket not available yet
#else
#warn jdk_serialsocket not available yet
#endif
#endif
