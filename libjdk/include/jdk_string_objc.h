#ifndef __JDK_STRING_OBJC_H
#define __JDK_STRING_OBJC_H

#ifdef __OBJC__

#import <Cocoa/Cocoa.h>
#include "jdk_string.h"

inline NSString * jdk_string_to_NSString( const jdk_string &s )
{
  return [NSString stringWithCString: s.c_str()];  
}

template <class T>
inline T jdk_string_from_NSString( NSString *s )
{
  return T([s lossyCString]);
}

#endif

#endif
