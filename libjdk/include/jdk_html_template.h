#ifndef __JDK_HTML_TEMPLATE_H
#define __JDK_HTML_TEMPLATE_H

#include "jdk_dynbuf.h"
#include "jdk_settings.h"

bool jdk_html_template( 
  jdk_dynbuf &result, 
  const jdk_buf &src, 
  const jdk_settings *vars1, 
  const jdk_settings *vars2 =0
  );

#endif
