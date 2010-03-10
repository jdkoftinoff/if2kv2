#ifndef __JDK_SETTINGS_OBJC_H
#define __JDK_SETTINGS_OBJC_H

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>

#include "jdk_string_objc.h"
#include "jdk_settings.h"

inline void jdk_settings_from_NSObjectController( jdk_settings &s, NSObjectController *o )
{
  int cnt = s.count();
  
  for( int i=0; i<cnt; ++i )
  {
    jdk_settings::pair_t *p = s.get(i);
    if( p )
    {      
      jdk_settings::key_t k( p->key );
      
      int len = k.len();
      for( int j=0; j<len; j++ )
      {
        if( k.get(j)=='.' )
          k.set(j,'_');
      }
      
      NSString *key = jdk_string_to_NSString( k );
      id v = [[o selection] valueForKey: key];
      
      NSString *valstr;
      if( [v isKindOfClass: [NSString class]] )
      {
        valstr = v;
      }
      else
      {
        valstr = [v stringValue];
      }
      p->value = jdk_string_from_NSString< jdk_settings::value_t >( valstr );
    }
  }
}


inline NSMutableDictionary *jdk_settings_to_NSObjectController( const jdk_settings &s, NSObjectController *o )
{
  NSMutableDictionary *d = [o content];
  int cnt = s.count();
  
  for( int i=0; i<cnt; ++i )
  {
    const jdk_settings::pair_t *p = s.get(i);
    if( p )
    {
      NSString *val = jdk_string_to_NSString( p->value );

      jdk_settings::key_t k( p->key );
      int len = k.len();
      for( int j=0; j<len; j++ )
      {
        if( k.get(j)=='.' )
          k.set(j,'_');
      }

      NSString *key = jdk_string_to_NSString( k );
      NSString *prefixed_key = [NSString stringWithFormat: @"selection.%@", jdk_string_to_NSString(p->key) ];
      [d setValue: val forKey: key];
      [o didChangeValueForKey: prefixed_key ];
    }
  }
  return d;
}


inline NSMutableDictionary *jdk_settings_to_NSDictionary( const jdk_settings &s, NSMutableDictionary *d )
{
  [d removeAllObjects];
  int cnt = s.count();
  
  for( int i=0; i<cnt; ++i )
  {
    const jdk_settings::pair_t *p = s.get(i);
    if( p )
    {
      NSString *val = jdk_string_to_NSString( p->value );
      NSString *key = jdk_string_to_NSString( p->key );
      [d setValue: val forKey: key];
    }
  }
  return d;
}


inline NSMutableDictionary *jdk_settings_to_NSDictionary( const jdk_settings &s )
{
  int cnt = s.count();
  
  NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: cnt];

  for( int i=0; i<cnt; ++i )
  {
    const jdk_settings::pair_t *p = s.get(i);
    if( p )
    {
      NSString *val = jdk_string_to_NSString( p->value );
      NSString *key = jdk_string_to_NSString( p->key );
      [d setValue: val forKey: key];
    }
  }
  return d;
}

inline void jdk_settings_from_NSDictionary( jdk_settings &s, NSDictionary *d )
{
  s.clear();
  
  NSEnumerator *enumerator = [d keyEnumerator];  
  id key;
  
  while ((key = [enumerator nextObject])) 
  {
    NSString *valstr = (NSString *)[d valueForKey: key];
    NSString *keystr = (NSString *)key;
    
    jdk_settings::key_t k( jdk_string_from_NSString< jdk_settings::key_t >(keystr) );
    jdk_settings::value_t v( jdk_string_from_NSString< jdk_settings::value_t >(valstr) );
    s.set( k, v );
  }
}

#endif
#endif
