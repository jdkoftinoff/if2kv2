#include "wnworld.h"
#include "jdk_world.h"
#include "jdk_string.h"
#include "wnloginquery.h"

int main(int argc, char **argv)
{
	jdk_str<4096> user_name;
	wn_get_logged_in_user_name( &user_name );
	fprintf( stdout, "User name is:%s", user_name.c_str() );

	return 0;
}
