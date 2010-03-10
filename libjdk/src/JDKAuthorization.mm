#if JDK_IS_MACOSX
#import "JDKAuthorization.h"

#include "unistd.h"

@implementation JDKAuthorization

static AuthorizationFlags myFlags = 
kAuthorizationFlagDefaults |
kAuthorizationFlagInteractionAllowed |
kAuthorizationFlagPreAuthorize |
kAuthorizationFlagExtendRights;


static AuthorizationItem myItems = 
{
  kAuthorizationRightExecute, 0, NULL, 0
};

static AuthorizationRights myRights = 
{
  1, &myItems
};

- (BOOL) doAuth
{
  OSStatus myStatus;
  
  // This will pre-authorize the authentication
  myStatus = AuthorizationCopyRights(myAuthorizationRef, &myRights, NULL, myFlags, NULL);
  
  if (myStatus != errAuthorizationSuccess) 
  {
    return FALSE;
  }
  return TRUE;
}

- init 
{
  [super init];
  
  OSStatus myStatus;
  
  myStatus = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, myFlags, &myAuthorizationRef);
  
  if( myStatus!=0 )
    return nil;
  else
    return self;
}

- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray 
{
  return [self executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray synchronous:TRUE];
}

- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray synchronous: (BOOL) sync 
{
  BOOL r=FALSE;
  if( [self doAuth] )
  {
    
    FILE *communicationStream = NULL;
    char **copyArguments = NULL;
    unsigned int i;
    OSStatus myStatus;
    char outputString[1024];
    
    copyArguments = (char **)malloc(sizeof(char *) * ([argumentArray count]+1));
    if( !copyArguments )
    {
      NSLog(@"Error: malloc() failed");
      return FALSE;
    }
    
    for (i=0;i<[argumentArray count];i++) 
    {
      copyArguments[i] = (char *) [[argumentArray objectAtIndex:i] lossyCString];
    }
    copyArguments[i] = NULL;
    
    myStatus = AuthorizationExecuteWithPrivileges(
                                                  myAuthorizationRef, 
                                                  [command lossyCString], 
                                                  kAuthorizationFlagDefaults,
                                                  copyArguments, 
                                                  (sync ? &communicationStream : NULL)
                                                  );
    
    if (sync && communicationStream ) 
    {
      while (!feof(communicationStream)) 
      {
        fgets(outputString, 1024, communicationStream);
        if (strlen(outputString) > 1)
        {
//          NSLog(@"JDKAuthorization: %s",outputString);
        }
      }
      fclose(communicationStream);
    }
    
    free(copyArguments);
    
    if (myStatus != errAuthorizationSuccess)
      NSLog(@"Error: Executing %@ with Authorization: %d", command, myStatus);
    
    r= (myStatus == errAuthorizationSuccess);
    
  }
  return r;
  
}

- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray withStdIn: (const jdk_dynbuf *)inputs
{
  BOOL r=FALSE;
  if( [self doAuth] )
  {
    
    FILE *communicationStream = NULL;
    char **copyArguments = NULL;
    unsigned int i;
    OSStatus myStatus;
    
    copyArguments = (char **)malloc(sizeof(char *) * ([argumentArray count]+1));
    if( !copyArguments )
    {
      NSLog(@"Error: malloc() failed");
      return FALSE;
    }
    
    for (i=0;i<[argumentArray count];i++) 
    {
      copyArguments[i] = (char *) [[argumentArray objectAtIndex:i] lossyCString];
    }
    copyArguments[i] = NULL;
    
    myStatus = AuthorizationExecuteWithPrivileges(
                                                  myAuthorizationRef, 
                                                  [command lossyCString], 
                                                  kAuthorizationFlagDefaults,
                                                  copyArguments, 
                                                  &communicationStream
                                                  );
    
    if( myStatus==errAuthorizationSuccess )
    {
      inputs->extract_to_stream( communicationStream);
      fflush( communicationStream );
      fclose( communicationStream );
    }
    else
    {
      ;
    }
    
    free(copyArguments);
    
    if (myStatus != errAuthorizationSuccess)
      NSLog(@"Error: Executing %@ with Authorization: %d", command, myStatus);
    
    r=(myStatus == errAuthorizationSuccess);
    
  }
  return r;
}

- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray withStdOut: (jdk_dynbuf *)inputs
{
  BOOL r=FALSE;
  if( [self doAuth] )
  {
    
    FILE *communicationStream = NULL;
    char **copyArguments = NULL;
    unsigned int i;
    OSStatus myStatus;
    
    inputs->clear();
    
    copyArguments = (char **)malloc(sizeof(char *) * ([argumentArray count]+1));
    if( !copyArguments )
    {
      NSLog(@"Error: malloc() failed");
      return FALSE;
    }
    
    for (i=0;i<[argumentArray count];i++) 
    {
      copyArguments[i] = (char *) [[argumentArray objectAtIndex:i] lossyCString];
    }
    copyArguments[i] = NULL;
    
    myStatus = AuthorizationExecuteWithPrivileges(
                                                  myAuthorizationRef, 
                                                  [command lossyCString], 
                                                  kAuthorizationFlagDefaults,
                                                  copyArguments, 
                                                  &communicationStream
                                                  );
    
    if( communicationStream )
    {
      inputs->append_from_stream(communicationStream );
      fclose(communicationStream);
    }
    
    free(copyArguments);
    
    if (myStatus != errAuthorizationSuccess)
      NSLog(@"Error: Executing %@ with Authorization: %d", command, myStatus);
    
    r= (myStatus == errAuthorizationSuccess);
    
  }
  return r;
}


- (void) dealloc 
{
  AuthorizationFree (myAuthorizationRef, kAuthorizationFlagDefaults);
  [super dealloc];
}

@end
#endif
