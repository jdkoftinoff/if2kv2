#include "jdk_world.h"

#if defined(WIN32)
#include "jdk_win32svc.h"

jdk_win32svc_base *jdk_win32svc_base::global_service_object=0;
#endif
