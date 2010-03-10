#include "jdk_world.h"
#include "jdk_settings.h"

#if JDK_IS_WIN32

int main( int argc, char **argv )
{
  //i_am_a_dummy();
  jdk_settings_win32registry settings( 
    HKEY_LOCAL_MACHINE,
    "SOFTWARE\\Turner and Sons\\if2redir"
    );
  
  //settings.save("libjdk_testwin32reg-result.txt" );
  return 0;
}


#else
int main()
{
  return 0;
}

#endif
