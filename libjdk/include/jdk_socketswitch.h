#ifndef _JDK_SOCKETSWITCH_H
#define _JDK_SOCKETSWITCH_H

#include "jdk_socket.h"
#include "jdk_pair.h"
#include "jdk_array.h"
#include "jdk_string.h"

/* a jdk_socketswitch object allows you to register an ascii name with a
 * client socket type builder function that takes a string like a url
 * and constructs and tries to connect the socket to the specified destination.
 */

typedef jdk_client_socket * (*jdk_socketswitch_maker)( const jdk_string &url );

typedef jdk_pair< jdk_str<64>, jdk_socketswitch_maker > jdk_socketswitch_pair;

class jdk_socketswitch
{
	public:
		jdk_socketswitch();
		virtual ~jdk_socketswitch();
	
		void add_socket_type( 
						const jdk_string &name, 
						jdk_socketswitch_maker maker_proc 
						);

		jdk_client_socket * make_socket( const jdk_string &url );
	private:
		jdk_array< jdk_socketswitch_pair > maker_map;
};


#endif
