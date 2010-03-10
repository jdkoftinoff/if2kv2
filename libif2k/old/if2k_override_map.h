#ifndef _IF2K_OVERRIDE_MAP_H
#define _IF2K_OVERRIDE_MAP_H

#include "jdk_thread.h"
#include "jdk_string.h"
#include "jdk_map.h"
#include "jdk_util.h"
#include "jdk_socket.h"

class if2k_override_map
{
    typedef jdk_map< jdk_str<256>, unsigned long >  map_t;
public:
    if2k_override_map() : map(), map_mutex()
    {
    }
    
    virtual ~if2k_override_map()
    {
    }
    
    virtual bool add_override( const jdk_string &client_, unsigned long timeout_secs )
    {
		jdk_str<1024> client(client_);
		client.strip_begws();
		client.strip_endws();
		if( !client.is_clear() )
		{			
			jdk_log( JDK_LOG_INFO, "Adding client '%s' to allow override", client.c_str() );
			jdk_synchronized(map_mutex);
			map.set( client, timeout_secs );
			return true;

#if 0
			jdk_str<128> client_ip;
			if( jdk_gethostbyname( client.c_str(), client_ip.c_str(), client_ip.getmaxlen() )==0 )
			{
				jdk_log( JDK_LOG_INFO, "Adding client '%s' to allow override", client.c_str() );
				jdk_synchronized(map_mutex);
				map.set( client, timeout_secs );
				return true;
			}
			else
			{
				jdk_log( JDK_LOG_INFO, "Unable to recognize client name '%s' to allow override", client.c_str() );
				return false;
			}
#endif
		}
		return false;
    }
    
    virtual bool is_client_in_override( const jdk_string &client_ )
    {
		jdk_str<1024> client(client_);
		client.strip_begws();
		client.strip_endws();
		
        jdk_synchronized(map_mutex);
	    int pos = map.find(client);
	    if( pos>=0 )
	    {
			bool r= jdk_get_time_seconds() < map.getvalue(pos);
//	        jdk_log( JDK_LOG_INFO, "checking client '%s' for allow override = %d", client.c_str(), (int)r );			
    	    return r;
	    }
	    else
	    {
//	        jdk_log( JDK_LOG_INFO, "couldnt find '%s' for allow override", client.c_str()  );
	        return false;
	    }
    }
    
protected:
    map_t map;
    jdk_mutex map_mutex;
};

#endif
