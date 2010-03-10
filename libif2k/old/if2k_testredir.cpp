#if JDK_IS_WIN32
#define DEBUG_REDIR
#include "../shared/if2k_redir.cpp"
#include "jdk_socket.h"


int main(int argc, char **argv )
{
	InitializeCriticalSection(&crit_load_settings);

	patch_connect();	
	load_settings();	

	
	{		
		jdk_inet_client_socket s;
		char *dest="192.168.147.254:3128";
		if( argc>1 )
		  	dest=argv[1];
		s.make_connection( dest, 0, false );
		
		jdk_str<256> actual;
		if( s.get_remote_addr( actual.c_str(), 255 ) )
		{
			printf( "remote addr is: %s\n", actual.c_str() );	
		}
		
	}
	
	unpatch_connect();
	return 0;
}
#else
int main()
{
	return 0;	
}

#endif
