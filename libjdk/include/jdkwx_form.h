#ifndef __JDKWX_FORM_H
#define __JDKWX_FORM_H

#include "wx/calctrl.h"

#include "jdkwx_validator.h"

class jdkwx_form
{
public:
  jdkwx_form( wxWindow *parent_,jdk_settings &settings_, wxSizer *topsizer_, int border_=1 )
    : 
    parent( parent_ ),
    settings( settings_ ),
    sizer( new wxFlexGridSizer( 2, 2, 2 ) ),
    topsizer( topsizer_ ),
    border( border_ ),
    inner_border( 0 )
    {
      sizer->SetFlexibleDirection( wxHORIZONTAL );
      sizer->AddGrowableCol( 1 );
    }

  ~jdkwx_form()
    {
      delete sizer;
    }

  void AddVSpace()
    {
      sizer->Add(0,0,1);
      sizer->Add(0,0,1);
    }

  void AddTitle( const char *title )
    {
      topsizer->Add(
        new wxStaticText(parent, wxID_ANY, title),
        0,
        wxALL | wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL,
        inner_border
        );
    }

  void AddSubtitle( const char *subtitle )
    {
      AddVSpace();
      sizer->Add(
        new wxStaticText(parent, wxID_ANY, subtitle),
        0,
        wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        inner_border
        );
      sizer->Add(0,0,1);
    }
  void AddFieldText( const char *title, const char *field_name )
    {
      sizer->Add(
        new wxStaticText(parent, wxID_ANY, title),
        1,
        wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        inner_border
        );

      wxTextCtrl *textfield =     
        new wxTextCtrl(
          parent, 
          wxID_ANY, 
          "", 
          wxDefaultPosition, 
          wxDefaultSize, 
          0, 
          jdkwx_string_validator( settings, field_name ),
          title
          );
      
      sizer->Add( 
        textfield,
        3,
        wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 
        inner_border
        );     
    }

  void AddFieldInt( const char *title, const char *field_name, long minvalue, long maxvalue )
    {
      sizer->Add(
        new wxStaticText(parent, wxID_ANY, title),
        0,
        wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
        inner_border
        );

      wxTextCtrl *textfield =     
        new wxTextCtrl(
          parent, 
          wxID_ANY, 
          "", 
          wxDefaultPosition, 
          wxDefaultSize, 
          0, 
          jdkwx_int_validator( settings, field_name, minvalue, maxvalue ),
          title
          );
      
      sizer->Add( 
        textfield,
        0,
        wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 
        inner_border
        );     

    }

  void AddFieldCheckbox( const char *title, const char *field_name  )
    {
      sizer->Add(0,0,1);

      wxCheckBox *c =     
        new wxCheckBox(
          parent, 
          wxID_ANY, 
          title, 
          wxDefaultPosition, 
          wxDefaultSize, 
          0, 
          jdkwx_bool_validator( settings, field_name ),
          title
          );
      
      sizer->Add( 
        c,
        0,
        wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 
        inner_border
        );     

    }


  void Finish() 
    {
      topsizer->Add( sizer, 0, wxALL | wxEXPAND | wxALIGN_LEFT, border );
      parent->SetAutoLayout(TRUE);
      parent->SetSizer( topsizer );
    }

  wxSizer *GetSizer() { return sizer; }
private:
  wxWindow *parent;
  jdk_settings &settings;
  wxFlexGridSizer *sizer;
  wxSizer *topsizer;
  int border;
  int inner_border;
};

#endif
