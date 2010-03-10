#ifndef _JDK_SMTP_SEND_H
#define _JDK_SMTP_SEND_H

#include "jdk_string.h"
#include "jdk_dynbuf.h"

bool jdk_smtp_send( 
	               const jdk_string &smtp_helo,
				   const jdk_string &smtp_server,
				   const jdk_string &from,
				   const jdk_string &to,
				   const jdk_string &subject,
				   const jdk_buf &body
				   );

#endif
