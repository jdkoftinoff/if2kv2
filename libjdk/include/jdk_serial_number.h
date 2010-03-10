#ifndef __JDK_SERIAL_NUMBER_H
#define __JDK_SERIAL_NUMBER_H

#include "jdk_string.h"

#include "jdk_settings.h"

struct jdk_serial_number
{
  typedef jdk_str<64> code_t;
  typedef jdk_str<256> field_t;

  field_t name, email, address, city, region, zipcode, phone, website, info, expires, expires_code;
  code_t serial;

  bool is_valid_code( const jdk_string &product_id );
  code_t calculate_code( const jdk_string &product_id);
  
  bool load( jdk_settings &settings );
  bool load( const jdk_string_filename & );
  bool save( jdk_settings &settings );
  bool save( const jdk_string_filename & );

  private:
  static char printable_char_map[32];
  void munge_field( unsigned char *result, const field_t &f, int num );
  code_t convert_munge_to_code( unsigned char *munge );
};



#endif
