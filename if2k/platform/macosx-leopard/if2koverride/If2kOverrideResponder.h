/* If2kOverrideResponder */

#import <Cocoa/Cocoa.h>

#include "if2k_mini_client.h"

@interface If2kOverrideResponder : NSObject
{
    IBOutlet NSButton *overrideCheckbox;
    if2k_mini_client *if2k;
}
- (void)awakeFromNib;
- (void)dealloc;
- (IBAction)overrideClicked:(id)sender;
- (void)showError;
@end
