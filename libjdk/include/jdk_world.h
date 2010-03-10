#ifndef _JDK_WORLD_H
#define _JDK_WORLD_H

// get platform specific defines

#include "autoconf.h"

#include "jdk_platform.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef JDK_HAS_SSL
#define JDK_HAS_SSL 0
#endif

// for win32 we need windows.h
#if JDK_IS_WIN32
# include <windows.h>
#if !JDK_IS_WINE
# include <winsock.h>
# include <io.h>
# include <process.h>
#endif
typedef LONGLONG jdk_int64;
typedef ULONGLONG jdk_uint64;
#endif


// define _REENTRANT if we have threads on this platform
#if JDK_HAS_THREADS && !defined(_REENTRANT)
#define _REENTRANT 1
#define __REENTRANT 1
#endif

// for UNIX we need unix specific headers
#ifdef __cplusplus
extern "C" {
#endif
   
#if JDK_IS_UNIX || JDK_IS_CYGWIN || JDK_IS_WINE || JDK_IS_MACOSX
# if JDK_HAS_THREADS
# define _PTHREADS 1
#  include <pthread.h>
# endif

#include <signal.h>
# include <sys/time.h>
# include <unistd.h>
# include <sys/wait.h>
#endif

// include the normal standard library stuff
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#ifdef __cplusplus
}
#endif


// for BeOS we need Be specific headers, and unistd.h
#if JDK_IS_BEOS
# include <Be.h>
# include <unistd.h>
typedef long long jdk_int64;
typedef unsigned long long jdk_uint64;
#endif


#if JDK_IS_UNIX 
typedef long long jdk_int64;
typedef unsigned long long jdk_uint64;
#endif

#ifdef __cplusplus
#include "jdk_util.h"
#endif

// if the C++ compiler doesn't have 'explicit' keyword, then
// define it away

#if !JDK_HAS_EXPLICIT
#define explicit
#endif


// normalize streams and strings for same usage with namespaces
// and without namespaces.  prefix all std:: stuff with std_

#if JDK_HAS_NAMESPACES
// dont redefine 'std'
#define BEGIN_NAMESPACE(x) namespace x {
#define END_NAMESPACE(x) }
#else
// make all references to std::xxxx be same as ::xxxx
# define std 
#define BEGIN_NAMESPACE(x) 
#define END_NAMESPACE(x) 
#endif

#ifndef int32_type
#define int32_type long
#endif
#ifndef uint32_type
#define uint32_type unsigned long
#endif
#ifndef int16_type
#define int16_type short
#endif
#ifndef uint16_type
#define uint16_type unsigned short
#endif
#ifndef jdk_float_type
#define jdk_float_type float
#endif

typedef int32_type int32;
typedef uint32_type uint32;
typedef int16_type int16;
typedef uint16_type uint16;
typedef jdk_float_type jdk_float;

// if our c++ library has a decent string, include it
# if JDK_HAS_STD_STRING
//#  include <string>
# endif

#ifdef __cplusplus

template <class T>
class jdk_auto_ptr
{
public:
    jdk_auto_ptr( T *p )
        : ptr(p)
    {
    }
    
    ~jdk_auto_ptr()
    {
        delete ptr;
    }
    
    T &get() { return *ptr; }
    const T &get() const { return *ptr; }
    
private:
    jdk_auto_ptr( const jdk_auto_ptr<T> &o );    // not allowed
    const jdk_auto_ptr<T> & operator = (const jdk_auto_ptr<T> &) const;    // not allowed
    
    T *ptr;
};

#endif


#endif

