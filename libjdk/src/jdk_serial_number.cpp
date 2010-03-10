#include "jdk_world.h"

#include "jdk_serial_number.h"

char jdk_serial_number::printable_char_map[32] = 
{
  'X', '1', '2', '3', 'K', '5', '6', 'W', 
  '8', '9', 'A', 'B', 'C', 'R', 'P', 'H',
};

void jdk_serial_number::munge_field( unsigned char *result, const field_t &f, int num )
{
  size_t i;
  size_t len = f.len();
  for( i=0; i<len; ++i )
  {
    size_t funky_bitrev_i = 
      (i&1)<<5    // bit 0 goes to bit 5
      | (i&2)<<3  // bit 1 goes to bit 4
      | (i&4)<<1  // bit 2 goes to bit 3
      | (i&8)>>1  // bit 3 goes to bit 2
      | (i&16)>>3 // bit 4 goes to bit 1
      | (i&32)>>5;// bit 5 goes to bit 0

    size_t pos = (funky_bitrev_i+num)&0xf;
    result[pos] ^= f.get(i);
  }
}

jdk_serial_number::code_t jdk_serial_number::convert_munge_to_code( unsigned char *munge )
{
  code_t result;
  size_t i;
  for( i=0; i<16; i+=4 )
  {
    char c;
    c=printable_char_map[(munge[i+0] ^ (munge[(i+0)^8]>>4)) & 0xf ];
    result.cat( c );
    c=printable_char_map[(munge[i+1] ^ (munge[(i+1)^8]>>4)) & 0xf ];
    result.cat( c );
    c=printable_char_map[(munge[i+2] ^ (munge[(i+2)^8]>>4)) & 0xf ];
    result.cat( c );
    c=printable_char_map[(munge[i+3] ^ (munge[(i+3)^8]>>4)) & 0xf ];
    result.cat( c );
    if( i!=12 )
      result.cat( '-' );
  }
  return result;
}

bool jdk_serial_number::is_valid_code( const jdk_string &product_id )
{
  code_t proper_code = calculate_code( product_id);
  return ( proper_code.icmp( serial )==0 );
}

jdk_serial_number::code_t jdk_serial_number::calculate_code( const jdk_string &product_id)
{
  int i;
  unsigned char munge[16];
  for( i=0; i<(int)sizeof(munge); ++i )
  {
    munge[i]=0;
  }
 
  munge_field( munge, name, 0 );
  munge_field( munge, email, 2 );
  munge_field( munge, address, 4 );
  munge_field( munge, city, 6 );
  munge_field( munge, region, 7 );
  munge_field( munge, zipcode, 8 );
  munge_field( munge, phone, 10 );
  munge_field( munge, website, 12 );
  munge_field( munge, info, 14 );
  munge_field( munge, expires_code, 1 );
  munge_field( munge, product_id, 9 );

  return convert_munge_to_code( munge );
}
  
bool jdk_serial_number::load( jdk_settings &settings )
{
  name = settings.get( "license.name" );  
  name.strip_begendws();
  email = settings.get( "license.email" ); 
  email.strip_begendws();
  address = settings.get( "license.address" ); 
  address.strip_begendws();
  city = settings.get( "license.city" ); 
  city.strip_begendws();
  region = settings.get( "license.region" ); 
  region.strip_begendws();
  zipcode = settings.get( "license.zipcode" ); 
  zipcode.strip_begendws();
  phone  = settings.get( "license.phone" ); 
  phone.strip_begendws();
  website  = settings.get( "license.website" ); 
  website.strip_begendws();
  info  = settings.get( "license.info" ); 
  info.strip_begendws();
  expires  = settings.get( "license.expires" ); 
  expires.strip_begendws();
  expires_code  = settings.get( "license.expires.code" ); 
  expires_code.strip_begendws();
  serial  = settings.get( "license.serial" ); 
  serial.strip_begendws();
  return true;
}

bool jdk_serial_number::load( const jdk_string_filename &fname )
{
  jdk_settings settings( fname );
  return load( settings );
}

bool jdk_serial_number::save( jdk_settings &settings )
{
  name.strip_begendws();
  email.strip_begendws();
  address.strip_begendws();
  city.strip_begendws();
  region.strip_begendws();
  zipcode.strip_begendws();
  phone.strip_begendws();
  website.strip_begendws();
  info.strip_begendws();
  expires.strip_begendws();
  expires_code.strip_begendws();
  serial.strip_begendws();

  settings.set( "license.name", name );
  settings.set( "license.email", email );
  settings.set( "license.address", address );
  settings.set( "license.city", city );
  settings.set( "license.region", region );
  settings.set( "license.zipcode", zipcode );
  settings.set( "license.phone", phone );
  settings.set( "license.website", website );
  settings.set( "license.info", info );
  settings.set( "license.expires", expires );
  settings.set( "license.expires.code", expires_code );
  settings.set( "license.serial", serial );
  return true;
}

bool jdk_serial_number::save( const jdk_string_filename &fname )
{
  bool r=false;
  jdk_settings settings;
  if( save( settings ) )
  {
    if( settings.save_file( fname, "\r\n" ) )
    {
      r=true;
    }
  }
  return r;
}
