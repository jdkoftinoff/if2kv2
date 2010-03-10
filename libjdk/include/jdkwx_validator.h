#ifndef __JDKWX_VALIDATOR_H
#define __JDKWX_VALIDATOR_H

#include "jdk_string.h"
#include "jdk_settings.h"

class jdkwx_string_validator : public wxValidator
{
  jdk_settings &settings;
  jdk_setting_key key;
  jdk_setting_value value;

public:
  jdkwx_string_validator(
    const jdkwx_string_validator &o 
    )
    :
    settings( (jdk_settings &)o.settings ),
    key( o.key ),
    value( o.value )
    {
//      fprintf( stderr, "Copy constructor\n" );
    }

  jdkwx_string_validator( 
    jdk_settings &settings_, 
    const char *key_,
    const char *value_
    ) 
    :
    settings( settings_ ),
    key(key_),
    value(value_)
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  jdkwx_string_validator( 
    jdk_settings &settings_, 
    const char *key_
    ) 
    :
    settings( settings_ ),
    key(key_),
    value( settings.get(key) )
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  wxObject *Clone() const
    {
//      fprintf( stderr, "Clone()\n" );
      return new jdkwx_string_validator( *this );
    }

  bool TransferFromWindow()
    {
      wxString s=((wxTextCtrl*)m_validatorWindow)->GetValue();
      value.cpy( s.c_str() );
//      fprintf( stderr, "TransferFromWindow():key: %s value: %s\n", key.c_str(), value.c_str() );
      settings.set( key, value );
      return true;
    } 

  bool TransferToWindow()
    {
      value = settings.get(key);
//      fprintf( stderr, "TransferToWindow():key: %s value: %s\n", key.c_str(), value.c_str() );
      wxString buf=wxString::Format("%s",value.c_str());  
      ((wxTextCtrl*)m_validatorWindow)->SetValue(buf);
      return true;
    }

  bool Validate(wxWindow* parent)
    {
      return true;
    }
};


class jdkwx_int_validator : public wxValidator
{
  jdk_settings &settings;
  jdk_setting_key key;
  long value;
  long minvalue;
  long maxvalue;

public:
  jdkwx_int_validator(
    const jdkwx_int_validator &o 
    )
    :
    settings( (jdk_settings &)o.settings ),
    key( o.key ),
    value( o.value ),
    minvalue( o.minvalue ),
    maxvalue( o.maxvalue )
    {
//      fprintf( stderr, "Copy constructor\n" );
    }

  jdkwx_int_validator( 
    jdk_settings &settings_, 
    const char *key_,
    long value_,
    int minvalue_,
    int maxvalue_
    ) 
    :
    settings( settings_ ),
    key(key_),
    value(value_),
    minvalue( minvalue_ ),
    maxvalue( maxvalue_ )
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  jdkwx_int_validator( 
    jdk_settings &settings_, 
    const char *key_,
    int minvalue_,
    int maxvalue_
    ) 
    :
    settings( settings_ ),
    key(key_),
    value( settings.get_long(key) ),
    minvalue( minvalue_ ),
    maxvalue( maxvalue_ )
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  wxObject *Clone() const
    {
//      fprintf( stderr, "Clone()\n" );
      return new jdkwx_int_validator( *this );
    }

  bool TransferFromWindow()
    {
      wxString s=((wxTextCtrl*)m_validatorWindow)->GetValue();
      value = strtol( s.c_str(), 0, 10 );
//      fprintf( stderr, "TransferFromWindow():key: %s value: %s\n", key.c_str(), value.c_str() );
      settings.set_long( key, value );
      return true;
    } 

  bool TransferToWindow()
    {
      value = settings.get_long(key);
//      fprintf( stderr, "TransferToWindow():key: %s value: %s\n", key.c_str(), value.c_str() );
      wxString buf=wxString::Format("%ld",value);  
      ((wxTextCtrl*)m_validatorWindow)->SetValue(buf);
      return true;
    }

  bool Validate(wxWindow* parent)
    {
      bool r=value>=minvalue && value<=maxvalue;
      if( !r )
      {
        value=minvalue;
        TransferToWindow();
      }
      return r;
    }
};


class jdkwx_bool_validator : public wxValidator
{
  jdk_settings &settings;
  jdk_setting_key key;
  long value;
  long minvalue;
  long maxvalue;

public:
  jdkwx_bool_validator(
    const jdkwx_bool_validator &o 
    )
    :
    settings( (jdk_settings &)o.settings ),
    key( o.key ),
    value( o.value ),
    minvalue( o.minvalue ),
    maxvalue( o.maxvalue )
    {
//      fprintf( stderr, "Copy constructor\n" );
    }

  jdkwx_bool_validator( 
    jdk_settings &settings_, 
    const char *key_,
    long value_,
    int minvalue_=0,
    int maxvalue_=1
    ) 
    :
    settings( settings_ ),
    key(key_),
    value(value_),
    minvalue( minvalue_ ),
    maxvalue( maxvalue_ )
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  jdkwx_bool_validator( 
    jdk_settings &settings_, 
    const char *key_,
    int minvalue_=0,
    int maxvalue_=1
    ) 
    :
    settings( settings_ ),
    key(key_),
    value( settings.get_long(key) ),
    minvalue( minvalue_ ),
    maxvalue( maxvalue_ )
    {
//      fprintf( stderr, "validator():key: %s value: %s\n", key.c_str(), value.c_str() );
    }

  wxObject *Clone() const
    {
//      fprintf( stderr, "Clone()\n" );
      return new jdkwx_bool_validator( *this );
    }

  bool TransferFromWindow()
    {
      value = ((wxCheckBox*)m_validatorWindow)->GetValue();
      settings.set_long( key, value );
      return true;
    } 

  bool TransferToWindow()
    {
      value = settings.get_long(key);
      ((wxCheckBox*)m_validatorWindow)->SetValue(value);
      return true;
    }

  bool Validate(wxWindow* parent)
    {
      return true;
    }
};


#endif
