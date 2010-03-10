#import "If2kSettingsGui.h"
#include "jdk_settings_objc.h"
#include "jdk_string_objc.h"

inline void TransferStringValue( bool to_gui, id gui_element, jdk_settings &map, const char *map_entry )
{
  if( to_gui )
  {
    [gui_element setStringValue:
      map.get_NSString(map_entry) ];
  }
  else
  {
    map.set( map_entry, [gui_element stringValue] );
  }
}

inline void TransferStringValue( bool to_gui, id gui_element, jdk_string &s )
{
  if( to_gui )
  {
    [gui_element setStringValue:
      jdk_string_to_NSString(s) ];
  }
  else
  {
    s = jdk_string_from_NSString< jdk_str<256> >( [gui_element stringValue] );
  }
}


inline void TransferStringValue( bool to_gui, id gui_element, jdk_buf &buf )
{
  if( to_gui )
  {
    [gui_element setString:
      jdk_buf_to_NSString(buf) ];
  }
  else
  {
    jdk_buf_from_NSString( buf, [gui_element string] );
  }
}


@implementation If2kSettingsGui

- (void)awakeFromNib
{
  last_installed_value = -1;
  loggedin=false;
  if2k = new if2k_mini_client;
  if2k_collection = new if2k_mini_client_collection(*if2k);

  [self prepareTextView: textBadUrls];
  [self prepareTextView: textGoodUrls];
  [self prepareTextView: textPhrases];
  
  [textPassword setStringValue:@"password"];
  [textServer setStringValue:@"localhost"];
  [self settingsEnable: FALSE];
  [self updateInstallButtons];
  
  my_timer = [[NSTimer scheduledTimerWithTimeInterval:1.0
                                               target:self
                                             selector:@selector(timerTick:)
                                             userInfo:nil
                                              repeats:YES] retain];
}

- (void)dealloc
{
  delete if2k_collection;
  delete if2k;
  [super dealloc];
}


- (void)prepareTextView: (NSTextView *)v
{
  NSFont *fnt = [NSFont fontWithName:@"Monaco" size:0.0f ];
  
  [ v setRichText: FALSE ];
  [ v setUsesFontPanel: TRUE ];
  [ v setContinuousSpellCheckingEnabled: FALSE];
  [ v setUsesFindPanel: TRUE];
  if( fnt )
    [ v setFont: fnt ];  
}

- (BOOL)settingsRead
{
  if2k->set_password( [[textPassword stringValue] lossyCString] );
  
  jdk_string_host server;
  
  server.form( "%s:8000", [[textServer stringValue] lossyCString] );
  
  if2k->set_server_address( server.c_str() );
  
  loggedin = if2k_collection->get_all_settings();
  
  [ self settingsEnable: loggedin];
  
#if 0
  [d setValue: 
    [NSString stringWithCString: 
      (const char *)if2k_collection->badurl[0].get_data() length: if2k_collection->badurl[0].get_data_length() ]
    forKey: @"badurls"];
  [guiController didChangeValueForKey: @"selection.badurls" ];    

  [d setValue: 
    [NSString stringWithCString: 
      (const char *)if2k_collection->goodurl[0].get_data() length: if2k_collection->goodurl[0].get_data_length() ]
    forKey: @"goodurls"];
  [guiController didChangeValueForKey: @"selection.goodurls" ];    

  [d setValue: 
    [NSString stringWithCString: 
      (const char *)if2k_collection->badphr[0].get_data() length: if2k_collection->badphr[0].get_data_length() ]
    forKey: @"phrases"];
  [guiController didChangeValueForKey: @"selection.phrases" ];    
#endif
  
  if( loggedin )
  {
    [self mainSettingsToGui];
    [self remoteSettingsToGui];
    [self customizeSettingsToGui];
    [self licenseSettingsToGui];
  }
  else
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];    
    [alert setMessageText:@"Error reading data from the filter. Perhaps your password is incorrect or there is a problem with the filter installation."];    
    [alert setAlertStyle:NSWarningAlertStyle];    
    [alert runModal];
    [alert release];
  }  
  return loggedin;
}

- (BOOL)settingsWrite
{
  if2k_collection->badurl[0].clear();
  if2k_collection->goodurl[0].clear();
  if2k_collection->badphr[0].clear();
 
#if 0
  if2k_collection->badurl[0].append_from_string( [[d valueForKey: @"badurls"] lossyCString] );
  if2k_collection->goodurl[0].append_from_string( [[d valueForKey: @"goodurls"] lossyCString] );
  if2k_collection->badphr[0].append_from_string( [[d valueForKey: @"phrases"] lossyCString] );
#endif

  if2k->set_password( [[textPassword stringValue] lossyCString] );

  printf( "pre: %s\n", if2k_collection->additional_settings.get( "kernel.settings.update.rate" ).c_str() );

  [self mainSettingsFromGui];
  printf( "post: %s\n", if2k_collection->additional_settings.get( "kernel.settings.update.rate" ).c_str() );
  
  bool r = if2k_collection->send_all_settings();
  
  if( !r )
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];    
    [alert setMessageText:@"Error applying data for the filter"];    
 //   [alert setInformativeText:@"Deleted records cannot be restored."];
    [alert setAlertStyle:NSWarningAlertStyle];    
    [alert runModal];
    [alert release];
  }
  return r;
}

- (void)settingsEnable : (BOOL) b
{
  [self mainSettingsEnable: b ];
  [self licenseSettingsEnable: b ];
  [self remoteSettingsEnable: b ];
  [self customizeSettingsEnable: b ];
  [buttonPasswordApply setEnabled:b];
  [buttonPasswordCancel setEnabled:b];  
  [textNewPassword setEnabled: b];
}


- (IBAction)pressApply:(id)sender
{
  if( [self settingsWrite] )
  {
    if2k->trigger_restart();
    sleep(4);
    [self settingsRead];
  }
}

- (IBAction)pressChangePassword:(id)sender
{
  [windowPassword makeKeyAndOrderFront:self];
}

- (IBAction)pressCustomize:(id)sender
{
  [windowCustomize makeKeyAndOrderFront: self];
  [self customizeSettingsToGui];
}

- (IBAction)pressCustomizeCancel:(id)sender
{
  [self customizeSettingsToGui];
  [windowCustomize close];  
}

- (IBAction)pressCustomizeOk:(id)sender
{
  [self customizeSettingsFromGui];
  [windowCustomize close];      
}

- (void)timerTick : (NSTimer *)timer
{
  [self updateInstallButtons];
}

- (bool)isInstalled
{
  struct stat sbuf;
  
  if( stat(IF2K_MINI_STARTUP_PATH,&sbuf) == 0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

- (void)updateInstallButtons
{
#if IF2K_IS_SERVER
  [buttonInstall setHidden: true ];
  
#else
  bool installed = [self isInstalled];
  
  if( installed != last_installed_value )
  {
    last_installed_value = (int)installed;
    
    if( installed )
    {
      [buttonInstall setEnabled: false ];
//      [buttonInstall setTitle: @"Uninstall" ];
    }
    else
    {
      [buttonInstall setTitle: @"Install" ];
    }
    [buttonLogin setEnabled: installed ];
    [textPassword setEditable: installed ];
    [textBadUrls setEditable: installed ];
    [textGoodUrls setEditable: installed ];
    [textPhrases setEditable: installed ];
    [self settingsEnable: installed && loggedin ];
  }
#endif
}


- (IBAction)pressInstall:(id)sender
{
  char c_cmd_line[4096+256];
  
  [self updateInstallButtons];
  
  if( [self isInstalled] )
  {
/*
    {
      NSAlert *alert = [[NSAlert alloc] init];
      
      [alert addButtonWithTitle:@"Yes"];    
      [alert addButtonWithTitle:@"No"];    
      
      [alert setMessageText:@"Are you sure you want to uninstall the filter? You will have to restart your computer after the filter has been uninstalled."];    
      [alert setAlertStyle:NSWarningAlertStyle];    
      if( [alert runModal] == NSAlertFirstButtonReturn )
      {
        loggedin=false;
        system( "\"" IF2K_MINI_UNINSTALL_PATH "\"" );
        {
          NSAlert *alert = [[NSAlert alloc] init];
          
          [alert addButtonWithTitle:@"Ok"];    
          
          [alert setMessageText:@"The Internet Filter has been uninstalled. You will have to restart your computer now for the change to take effect."];    
          [alert setAlertStyle:NSWarningAlertStyle];    
          [alert runModal];
          [alert release];
        }
        
        sleep(1);
        [self updateInstallButtons];
      }
      [alert release];
    }
  */  
  }
  else
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"Yes"];    
    [alert addButtonWithTitle:@"No"];    
    
    [alert setMessageText:@"Are you sure you want to install the Internet Filter?"];    
    [alert setAlertStyle:NSWarningAlertStyle];    
    if( [alert runModal] == NSAlertFirstButtonReturn )
    {
      char c_pkg_path[4096];

      NSString *pkg_path = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: @"/Contents/Resources/if2ksystem.pkg"];
      NSLog( pkg_path );
      strcpy( c_pkg_path, [pkg_path UTF8String] );
      sprintf( c_cmd_line, "\"%s\" \"%s\"", "/usr/bin/open", c_pkg_path );
      system( c_cmd_line );
    }
    
    [alert release];

  }
}

- (IBAction)pressLicense:(id)sender
{
  [windowLicense makeKeyAndOrderFront: self];
  [self licenseSettingsToGui];
  
}

- (IBAction)pressLicenseCancel:(id)sender
{
  [self licenseSettingsToGui ];
  [windowLicense close];
}

- (IBAction)pressLicenseLoad:(id)sender
{
  int result;
  NSOpenPanel *oPanel = [NSOpenPanel openPanel];
  NSArray *fileTypes = nil;
  
  [oPanel setAllowsMultipleSelection:NO];
  
  result = [oPanel runModalForTypes:fileTypes ];
  if (result == NSOKButton)
  {
    jdk_string_filename f( [[oPanel filename] lossyCString] );
    if( if2k_collection->license_settings.load_file( f ) )
    {
      [self licenseSettingsToGui];
    }
  }
}

- (IBAction)pressLicenseOk:(id)sender
{
  [self licenseSettingsFromGui];
  [windowLicense close];
}

- (IBAction)pressLogin:(id)sender
{
  [self settingsRead];
}

- (IBAction)pressRemoteCancel:(id)sender
{
  [self remoteSettingsToGui];
  [windowRemote close];
}

- (IBAction)pressRemoteOk:(id)sender
{
  [self remoteSettingsFromGui];
  [windowRemote close];
}

- (IBAction)pressPasswordCancel:(id)sender
{
  [textNewPassword setStringValue: @""];
  [windowPassword close];
}

- (IBAction)pressPasswordOk:(id)sender
{
  jdk_str<256> praw;
  TransferStringValue( false, textNewPassword, praw );
  jdk_str<512> p;
  if2k_hash_for_password( p, praw );

  if2k_collection->install_settings.set( "kernel.remote.update.password", p );
/*  TransferStringValue( false, textNewPassword, if2k_collection->install_settings, "kernel.remote.update.password" );  */
  
  if( if2k->post_install_settings( if2k_collection->install_settings ) )
  {
    bool r;
    r=if2k->trigger_restart();
    if( r )
    {
      sleep(3);
      [textPassword setStringValue: [textNewPassword stringValue]];
      [self pressLogin: self];
    }
    
  }
  else
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];    
    [alert setMessageText:@"Error setting the new password"];    
    [alert setAlertStyle:NSWarningAlertStyle];    
    [alert runModal];
    [alert release];    
  }
  
  [textNewPassword setStringValue: @""];    
  [windowPassword close];
}

- (IBAction)pressRemoteSettings:(id)sender
{
  [windowRemote makeKeyAndOrderFront: self];
  [self remoteSettingsToGui];

}


- (void)licenseSettingsToGui
{
  [textLicenseName setStringValue:
    if2k_collection->license_settings.get_NSString("license.name") ];
  
  [textLicenseEmail setStringValue:
    if2k_collection->license_settings.get_NSString("license.email" ) ];
  
  [textLicenseAddress setStringValue:
    if2k_collection->license_settings.get_NSString("license.address" ) ];
  
  [textLicenseCity setStringValue:
    if2k_collection->license_settings.get_NSString("license.city" )];
  
  [textLicenseRegion setStringValue:
    if2k_collection->license_settings.get_NSString("license.region" )];
  
  [textLicenseZipcode setStringValue:
    if2k_collection->license_settings.get_NSString("license.zipcode" )];
  
  [textLicensePhone setStringValue:
    if2k_collection->license_settings.get_NSString("license.phone" )];
  
  [textLicenseWebsite setStringValue:
    if2k_collection->license_settings.get_NSString("license.website" )];
  
  [textLicenseInfo setStringValue:
    if2k_collection->license_settings.get_NSString("license.info" )];
  
  [textLicenseExpires setStringValue:
    if2k_collection->license_settings.get_NSString("license.expires" )];  
  
  [textLicenseSerial setStringValue:
    if2k_collection->license_settings.get_NSString("license.serial" )];   
}


- (void)licenseSettingsFromGui
{
  if2k_collection->license_settings.set( "license.name", [textLicenseName stringValue] );
  if2k_collection->license_settings.set( "license.email", [textLicenseEmail stringValue] );
  if2k_collection->license_settings.set( "license.address", [textLicenseAddress stringValue] );
  if2k_collection->license_settings.set( "license.city", [textLicenseCity stringValue] );
  if2k_collection->license_settings.set( "license.region", [textLicenseRegion stringValue] );
  if2k_collection->license_settings.set( "license.zipcode", [textLicenseZipcode stringValue] );
  if2k_collection->license_settings.set( "license.phone", [textLicensePhone stringValue] );
  if2k_collection->license_settings.set( "license.website", [textLicenseWebsite stringValue] );
  if2k_collection->license_settings.set( "license.info", [textLicenseInfo stringValue] );
  if2k_collection->license_settings.set( "license.expires", [textLicenseExpires stringValue] );
  if2k_collection->license_settings.set( "license.serial", [textLicenseSerial stringValue] );
}


- (void)licenseSettingsEnable: (BOOL)b
{
  [textLicenseName setEnabled: b];
  [textLicenseEmail setEnabled: b];
  [textLicenseAddress setEnabled: b];
  [textLicenseCity setEnabled: b];
  [textLicenseRegion setEnabled: b];
  [textLicenseZipcode setEnabled: b];
  [textLicensePhone setEnabled: b];
  [textLicenseWebsite setEnabled: b];
  [textLicenseInfo setEnabled: b];
  [textLicenseExpires setEnabled: b];
  [textLicenseSerial setEnabled: b];
  
  [buttonLicenseCancel setEnabled: b];
  [buttonLicenseLoad setEnabled: b];
  [buttonLicenseApply setEnabled: b]; 
}


- (void)mainSettingsTransfer: (BOOL)to_gui
{
  TransferStringValue( to_gui, buttonEnableFiltering, if2k_collection->additional_settings, "kernel.blocking.enable" ); 
  TransferStringValue( to_gui, buttonAllowOverride, if2k_collection->additional_settings, "kernel.override.allow" ); 

  TransferStringValue( to_gui, textBadUrls, if2k_collection->badurl[0]);
  TransferStringValue( to_gui, textGoodUrls, if2k_collection->goodurl[0]);
  TransferStringValue( to_gui, textPhrases, if2k_collection->badphr[0]);
}

- (void)mainSettingsToGui
{
  [self mainSettingsTransfer: true];
}


- (void)mainSettingsFromGui
{
  [self mainSettingsTransfer: false];  
}


- (void)mainSettingsEnable: (BOOL)b
{
  [buttonChangePassword setEnabled: b];
  [buttonLicense setEnabled: b];
  [buttonApply setEnabled: b];
  [buttonRemoteSettings setEnabled: b];
  [buttonCustomize setEnabled: b];
  [buttonEnableFiltering setEnabled: b];
  [buttonAllowOverride setEnabled: b];
//  [textBadUrls setEnabled: b];
//  [textGoodUrls setEnabled: b];
//  [textPhrases setEnabled: b];
}



- (void)remoteSettingsToGui
{
  [textHttpProxy setStringValue:
    if2k_collection->additional_settings.get_NSString("http.proxy") ];
  
  [textRemoteDBURL setStringValue:
    if2k_collection->additional_settings.get_NSString("kernel.db.user.remote") ];

  [textRemoteSettingsURL setStringValue:
    if2k_collection->additional_settings.get_NSString("kernel.settings.additional.remote") ];

  [textUpdateRateInMinutes setStringValue:
    if2k_collection->additional_settings.get_NSString("kernel.settings.update.rate") ];
}

- (void)remoteSettingsFromGui
{
  if2k_collection->additional_settings.set( "http.proxy", [textHttpProxy stringValue] );
  if2k_collection->additional_settings.set( "kernel.db.user.remote", [textRemoteDBURL stringValue] );
  if2k_collection->additional_settings.set( "kernel.settings.additional.remote", [textRemoteSettingsURL stringValue] );
  if2k_collection->additional_settings.set( "kernel.settings.update.rate", [textUpdateRateInMinutes stringValue] );
}

- (void)remoteSettingsEnable: (BOOL)b
{
  [textHttpProxy setEnabled: b];
  [textRemoteDBURL setEnabled: b];
  [textRemoteSettingsURL setEnabled: b];
  [textUpdateRateInMinutes setEnabled: b];  
  [buttonRemoteCancel setEnabled: b];
  [buttonRemoteApply setEnabled: b];  
}

- (void)customizeSettingsTransfer: (BOOL)to_gui
{
  TransferStringValue( to_gui, buttonBlockBadSites, if2k_collection->additional_settings, "kernel.blocking.block.bad" ); 
  TransferStringValue( to_gui, buttonBlockUnknownSites, if2k_collection->additional_settings, "kernel.blocking.block.unknown" ); 
  TransferStringValue( to_gui, buttonCensorBadURLs, if2k_collection->additional_settings, "kernel.blocking.censor.url.content" ); 
  TransferStringValue( to_gui, buttonCensorBadPhrases, if2k_collection->additional_settings, "kernel.blocking.censor.phrase.content" ); 
  
  TransferStringValue( to_gui, buttonAllowContentViaReferer, if2k_collection->additional_settings, "kernel.blocking.referer.enable" ); 

  TransferStringValue( to_gui, textMatchCount, if2k_collection->additional_settings, "kernel.blocking.content.match.threshold" ); 
  
  TransferStringValue( to_gui, textBlockPageTitle, if2k_collection->additional_settings, "blockpage.title" ); 
  TransferStringValue( to_gui, textBlockPageImage, if2k_collection->additional_settings, "blockpage.image.url" ); 
  TransferStringValue( to_gui, textBlockPageImageLink, if2k_collection->additional_settings,  "blockpage.image.link"); 
  TransferStringValue( to_gui, textBlockPageText, if2k_collection->additional_settings,"blockpage.blockedmessage" ); 
  TransferStringValue( to_gui, textBlockPageURL, if2k_collection->additional_settings, "blockpage.url" ); 

  TransferStringValue( to_gui, textRedirectGIF, if2k_collection->additional_settings, "kernel.blocking.image.redirect.gif.url"); 
  TransferStringValue( to_gui, textRedirectJPG, if2k_collection->additional_settings, "kernel.blocking.image.redirect.jpg.url" ); 
  TransferStringValue( to_gui, textRedirectPNG, if2k_collection->additional_settings, "kernel.blocking.image.redirect.png.url"); 

  
  TransferStringValue( to_gui, buttonRedirectImages, if2k_collection->additional_settings, "kernel.blocking.image.redirect.enable"
 ); 

}

- (void)customizeSettingsToGui
{
  [self customizeSettingsTransfer: true];
}

- (void)customizeSettingsFromGui
{
  [self customizeSettingsTransfer: false];
}

- (void)customizeSettingsEnable: (BOOL)b
{
  [buttonBlockBadSites setEnabled: b];
  [buttonBlockUnknownSites setEnabled: b];
  [buttonCensorBadURLs setEnabled: b];
  [buttonCensorBadPhrases setEnabled: b];
  [buttonAllowContentViaReferer setEnabled: b];
  [textMatchCount setEnabled: b];
  
  [textBlockPageTitle setEnabled: b];
  [textBlockPageImage setEnabled: b];
  [textBlockPageImageLink setEnabled: b];
  [textBlockPageText setEnabled: b];
  [textBlockPageURL setEnabled: b];
  
  [buttonRedirectImages setEnabled: b];
  [textRedirectJPG setEnabled: b];
  [textRedirectGIF setEnabled: b];
  [textRedirectPNG setEnabled: b];
  
  [buttonCustomizationCancel setEnabled: b];
  [buttonCustomizationApply setEnabled: b];
}


- (int) numberOfRowsInTableView:(NSTableView *)inTableView
{
  return 8;
}

- (id) tableView:(NSTableView *)inTableView objectValueForTableColumn:(NSTableColumn *)inTableColumn row:(int)inRow
{
  return @"Boo!";
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
}



@end
