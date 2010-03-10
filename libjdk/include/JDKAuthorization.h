#if JDK_IS_MACOSX
#import <Foundation/Foundation.h>

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include "jdk_dynbuf.h"

@interface JDKAuthorization : NSObject {
  AuthorizationRef myAuthorizationRef;
}

- init;
- (BOOL) doAuth;
- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray;
- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray synchronous: (BOOL) sync;
- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray withStdIn: (const jdk_dynbuf *)inputs;
- (BOOL) executeCommand:(NSString *) command withArgs: (NSArray *) argumentArray withStdOut: (jdk_dynbuf *)inputs;

@end
#endif
