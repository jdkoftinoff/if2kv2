#import <Cocoa/Cocoa.h>

@interface NSIf2kRemoverGui : NSObject 
{
    IBOutlet NSText *messageText;
    IBOutlet NSApplication *myApplication;

}
- (IBAction)cancelPressed:(id)sender;
- (IBAction)removePressed:(id)sender;
@end
