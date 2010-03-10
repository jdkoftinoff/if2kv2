#ifndef __JDK_SYSTEMSETTINGS_MACOSX_H
#define __JDK_SYSTEMSETTINGS_MACOSX_H

#if JDK_IS_MACOSX
#include <SystemConfiguration/SystemConfiguration.h>

struct macosx_http_proxy_config_t
{
  macosx_http_proxy_config_t()
  {
    proxies = (NSDictionary *)SCDynamicStoreCopyProxies(NULL);
    
    HTTPEnabled = [[proxies objectForKey:(NSString *)kSCPropNetProxiesHTTPEnable] boolValue];
    HTTPHost = [proxies objectForKey:(NSString *)kSCPropNetProxiesHTTPProxy];
    HTTPPort = [proxies objectForKey:(NSString *)kSCPropNetProxiesHTTPPort];
  }

  NSDictionary *proxies;
  BOOL HTTPEnabled;
  NSString *HTTPHost;
  NSString *HTTPPort;
};

#endif
#endif
