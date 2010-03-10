/*
 *  libjdk_testmacosxplist.cpp
 *  fullfilter
 *
 *  Created by jeffk on Wed Nov 21 2001.
 *  Copyright (c) 2001 J.D. Koftinoff Software, Ltd. All rights reserved.
 *
 */

#include "jdk_world.h"
#include "jdk_settings.h"

int main ( int argc, char **argv ) 
{
#if JDK_IS_MACOSX
  jdk_settings_macosx_plist settings(0,argc,argv);
  settings.save_plist( "settings.plist" );
  settings.save_file( stdout );
#endif	
  return 0;
}



