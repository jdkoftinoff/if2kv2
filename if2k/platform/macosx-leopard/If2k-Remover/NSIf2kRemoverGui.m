#include <stdio.h>
#include <Security/Authorization.h>
#import "NSIf2kRemoverGui.h"

@implementation NSIf2kRemoverGui
- (IBAction)cancelPressed:(id)sender {
  NSLog( @"Cancel Pressed" );
  [myApplication stop: self];
}

- (IBAction)removePressed:(id)sender {
  NSLog( @"Remove Pressed");
  
  char process_path[4096] = "/usr/bin/id";
  NSString *resource_path = [[NSBundle mainBundle] resourcePath];
  NSLog( @"resource path is '%@'", resource_path );
  if( [resource_path getFileSystemRepresentation: process_path maxLength: sizeof(process_path) ] )
  {
    strcat( process_path, "/if2k_remove_tool" );
    NSLog( @"process_path is '%s'", process_path );
  }
  else
  {
    NSLog( @"getFileSystemRepresentation failed" );
    return;
  }
  AuthorizationRef auth_ref = NULL;
  OSStatus status;
  
  status = AuthorizationCreate(
                               NULL, 
                               kAuthorizationEmptyEnvironment, 
                               kAuthorizationFlagDefaults, 
                               & auth_ref
                               );
  if ( status != errAuthorizationSuccess )
  {
    NSLog( @"Create error: %d", status );    
  }
  else
  {
    char *subprocess_arguments[] = {NULL};
    
    FILE *subprocess_pipe;
    
    status = AuthorizationExecuteWithPrivileges(
                                                auth_ref, 
                                                process_path, 
                                                kAuthorizationFlagDefaults, 
                                                subprocess_arguments, 
                                                &subprocess_pipe
                                                );
    
    if ( status != errAuthorizationSuccess )
    {
      NSLog(@"Execute error: %i", status );
    }
    else
    {
      char buf[4096*8];
      size_t buf_len=0;
      size_t buf_size=sizeof(buf)-1;
      while( buf_len<sizeof(buf) )
      {
        size_t cnt = fread( &buf[buf_len], 1, buf_size-buf_len, subprocess_pipe );
        if( cnt==0 )
          break;
        buf_len+=cnt;
      }  
      
      buf[buf_len] = '\0';
      NSLog( @"Received data: %s", buf );
      
      fclose(subprocess_pipe);
      
      status = AuthorizationFree( 
                                 auth_ref, 
                                 kAuthorizationFlagDefaults
                                 );
      if ( status != errAuthorizationSuccess )
      {
        NSLog( @"Free error: %i", status );
        //    printf("Free error: %i\n", status );
      }
      // TODO: Make reboot notification
      [myApplication stop: self];
    }
  }
}


@end
