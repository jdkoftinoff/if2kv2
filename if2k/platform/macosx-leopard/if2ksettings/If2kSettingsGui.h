/* If2kSettingsGui */

#import <Cocoa/Cocoa.h>

#include "if2k_mini_client.h"

@interface If2kSettingsGui : NSObject
{
    int last_installed_value;
    bool loggedin;
    
    if2k_mini_client *if2k;
    if2k_mini_client_collection *if2k_collection;

    NSTimer *my_timer;
    
    // in main window
    IBOutlet NSButton *buttonInstall;
    IBOutlet NSButton *buttonLogin;
    IBOutlet NSButton *buttonChangePassword;
    IBOutlet NSButton *buttonLicense;
    IBOutlet NSButton *buttonApply;
    IBOutlet NSButton *buttonRemoteSettings;
    IBOutlet NSButton *buttonCustomize;
    
    IBOutlet NSButton *buttonEnableFiltering;
    IBOutlet NSButton *buttonAllowOverride;

    IBOutlet NSTextField *textPassword;
    IBOutlet NSTextField *textServer;
    
    IBOutlet NSTextView *textBadUrls;
    IBOutlet NSTextView *textGoodUrls;
    IBOutlet NSTextView *textPhrases;
    
    // in Customization window
    IBOutlet NSButtonCell *buttonBlockBadSites;
    IBOutlet NSButtonCell *buttonBlockUnknownSites;
    IBOutlet NSButtonCell *buttonCensorBadURLs;
    IBOutlet NSButtonCell *buttonCensorBadPhrases;
    IBOutlet NSButton *buttonAllowContentViaReferer;
    IBOutlet NSCell *textMatchCount;
    
    IBOutlet NSCell *textBlockPageTitle;
    IBOutlet NSCell *textBlockPageImage;
    IBOutlet NSCell *textBlockPageImageLink;
    IBOutlet NSCell *textBlockPageText;
    IBOutlet NSCell *textBlockPageURL;
    
    IBOutlet NSButton *buttonRedirectImages;
    IBOutlet NSCell *textRedirectJPG;
    IBOutlet NSCell *textRedirectGIF;
    IBOutlet NSCell *textRedirectPNG;
    
    IBOutlet NSButton *buttonCustomizationCancel;
    IBOutlet NSButton *buttonCustomizationApply;
    
    // In Remote Settings window
    IBOutlet NSCell *textHttpProxy;
    IBOutlet NSCell *textRemoteDBURL;
    IBOutlet NSCell *textRemoteSettingsURL;
    IBOutlet NSCell *textUpdateRateInMinutes;
    
    IBOutlet NSButton *buttonRemoteCancel;
    IBOutlet NSButton *buttonRemoteApply;
    
    // In License Window
    IBOutlet NSCell *textLicenseName;
    IBOutlet NSCell *textLicenseEmail;
    IBOutlet NSCell *textLicenseAddress;
    IBOutlet NSCell *textLicenseCity;
    IBOutlet NSCell *textLicenseRegion;
    IBOutlet NSCell *textLicenseZipcode;
    IBOutlet NSCell *textLicensePhone;
    IBOutlet NSCell *textLicenseWebsite;
    IBOutlet NSCell *textLicenseInfo;
    IBOutlet NSCell *textLicenseExpires;
    IBOutlet NSCell *textLicenseSerial;
    
    IBOutlet NSButton *buttonLicenseCancel;
    IBOutlet NSButton *buttonLicenseLoad;
    IBOutlet NSButton *buttonLicenseApply; 
    
    // In Password Window
    IBOutlet NSCell *textNewPassword;
    IBOutlet NSButton *buttonPasswordCancel;
    IBOutlet NSButton *buttonPasswordApply;
            
    // Other windows
    IBOutlet NSWindow * windowCustomize;
    IBOutlet NSWindow * windowLicense;
    IBOutlet NSWindow * windowMain;
    IBOutlet NSWindow * windowRemote;
    IBOutlet NSWindow * windowPassword;
    IBOutlet NSWindow * windowUsers;
    IBOutlet NSWindow * windowLog;
}

- (void)tableView:(NSTableView *)aTableView 
   setObjectValue:anObject
   forTableColumn:(NSTableColumn *)aTableColumn 
              row:(int)rowIndex;

- (int) numberOfRowsInTableView:(NSTableView *)inTableView;

- (id)          tableView:(NSTableView *)inTableView
objectValueForTableColumn:(NSTableColumn *)inTableColumn
                      row:(int)inRow;

- (void)timerTick: (NSTimer *)timer;
- (bool)isInstalled;
- (void)updateInstallButtons;
- (IBAction)pressInstall:(id)sender;
- (IBAction)pressLogin:(id)sender;
- (IBAction)pressApply:(id)sender;

- (IBAction)pressChangePassword:(id)sender;
- (IBAction)pressPasswordOk:(id)sender;
- (IBAction)pressPasswordCancel:(id)sender;

- (IBAction)pressCustomize:(id)sender;
- (IBAction)pressCustomizeCancel:(id)sender;
- (IBAction)pressCustomizeOk:(id)sender;

- (IBAction)pressLicense:(id)sender;
- (IBAction)pressLicenseCancel:(id)sender;
- (IBAction)pressLicenseLoad:(id)sender;
- (IBAction)pressLicenseOk:(id)sender;

- (IBAction)pressRemoteCancel:(id)sender;
- (IBAction)pressRemoteOk:(id)sender;
- (IBAction)pressRemoteSettings:(id)sender;

- (BOOL)settingsRead;
- (BOOL)settingsWrite;
- (void)settingsEnable : (BOOL) b;

- (void)licenseSettingsToGui;
- (void)licenseSettingsFromGui;
- (void)licenseSettingsEnable: (BOOL)b;

- (void)mainSettingsTransfer: (BOOL)to_gui;
- (void)mainSettingsToGui;
- (void)mainSettingsFromGui;
- (void)mainSettingsEnable: (BOOL)b;

- (void)remoteSettingsToGui;
- (void)remoteSettingsFromGui;
- (void)remoteSettingsEnable: (BOOL)b;

- (void)customizeSettingsToGui;
- (void)customizeSettingsFromGui;
- (void)customizeSettingsTransfer: (BOOL)to_gui;
- (void)customizeSettingsEnable: (BOOL)b;

- (void)prepareTextView: (NSTextView *)v;

@end
