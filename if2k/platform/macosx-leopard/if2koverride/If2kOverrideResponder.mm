#import "If2kOverrideResponder.h"

@implementation If2kOverrideResponder

- (void)awakeFromNib
{
//  NSLog( @"Awake!" );
  if2k = new if2k_mini_client;
  bool o=false;
  if( !if2k->get_override_mode(o) )
  {
    NSLog( @"Error getting override mode" );
  }
  [overrideCheckbox setState: o ? NSOnState : NSOffState ];
}

- (void)dealloc
{
  delete if2k;
  [super dealloc];
}

- (IBAction)overrideClicked:(id)sender
{
//  NSLog( @"Click!" );
  bool o=([ overrideCheckbox state ] == NSOnState);
  if( !if2k->post_override_mode( o ) )
  {
    NSLog( @"Error setting override mode" );
    [self showError];
  }
  if( !if2k->get_override_mode(o) )
  {
    NSLog( @"Error getting override mode" );
    o=false;
  }
  [overrideCheckbox setState: o ? NSOnState : NSOffState ];
}

- (void)showError
{
  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"Cancel"];
  [alert setMessageText:@"Unable to change If2k Override Mode"];
  [alert setInformativeText:@"Either the If2k system is not running or the Override feature is disabled."];
  [alert setAlertStyle:NSWarningAlertStyle];
  [alert runModal];
  [alert release];  
}


@end
