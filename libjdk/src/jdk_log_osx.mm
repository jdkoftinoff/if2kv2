
#if JDK_IS_MACOSX
#include <Cocoa/Cocoa.h>

#include "jdk_world.h"
#include "jdk_log.h"

extern "C" void jdk_log_NSLog(const char *prefix, const char *buf)
{
  NSLog( @"%s: %s", prefix, buf );
}
#endif
