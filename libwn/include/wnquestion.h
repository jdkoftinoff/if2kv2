#ifndef __WNQUESTION_H
#define __WNQUESTION_H

#include "wninstall.h"
#include "wnreg.h"
#include "wnwindow.h"
#include "jdk_string.h"
#include "jdk_util.h"
#include "wnmain.h"

struct WNDlgAskTextQuestion
{
	const char *question;
	const char *initial_value;
	bool is_password;		
};


struct WNDlgAskTextQuestionList
{
	WNDlgAskTextQuestionList(int num, WNDlgAskTextQuestion *question_)
	{
		width=400;
		num_questions=num;
		info_lines=3;
		questions=question_;
		answers = new jdk_str<1024>[num];
	}
	
	~WNDlgAskTextQuestionList()
	{
		delete [] answers;
	}
	

	int width;
	int num_questions;
	int info_lines;
	jdk_str<1024> title;
	jdk_str<1024> info;
	WNDlgAskTextQuestion *questions;
	jdk_str<1024> *answers;
};

class WNDlgAskTextQuestionEvent
{
public:
	WNDlgAskTextQuestionEvent() {}
	virtual ~WNDlgAskTextQuestionEvent() {}

	virtual void ButtonOK( const WNDlgAskTextQuestionList &info ) = 0;
	virtual void ButtonCancel() = 0;
};

#if DEBUG			
class WNDlgAskTextQuestionEventDummy : public WNDlgAskTextQuestionEvent
{
public:
	void ButtonOK( const WNDlgAskTextQuestionList &info )
		{

			for( int i=0; i<info.num_questions; ++i )
			{				
				fprintf( stdout, "OK %d: %s\n", i+1, info.answers[i].c_str() );
			}

			PostQuitMessage(0);
		}

	void ButtonCancel()
		{
			fprintf( stdout, "Cancel\n" );
			PostQuitMessage(0);			
		}

};
#endif

class AskTextWindow : public WNWindow
{
public:

	AskTextWindow( 
				  WNDlgAskTextQuestionList &question_, 
				  WNDlgAskTextQuestionEvent &event_
				  );

	~AskTextWindow();

    virtual	bool	Create();

protected:
	virtual	bool	ProcessAccelerator( MSG *m );   
	virtual	LPARAM	CommandMsg( WPARAM cmd, HWND hCtl );		// WM_COMMAND
    virtual LPARAM  NextDlgCtlMsg( WPARAM w, LPARAM l ); // WM_NEXTDLGCTL
    HWND label_info;
    HWND label_question[20];
    HWND edit_value[20];
    HWND button_ok;
    HWND button_cancel;

	WNDlgAskTextQuestionList &qlist;
	WNDlgAskTextQuestionEvent &event;
	int width;
    int focus_list_len;
    HWND focus_list[22];
    int current_focus_item;
   
private:
    int add_focus_list( HWND w )
    {
		int i = focus_list_len;
		focus_list[ focus_list_len++ ] = w;
		return i;
    }
    void set_focus( int i )
    {
		if (SendMessage( focus_list[ current_focus_item ], WM_GETDLGCODE, 0, 0 ) & DLGC_HASSETSEL)
			SendMessage( focus_list[ current_focus_item ], EM_SETSEL, (WPARAM)-1, 0 );
		
		::SetFocus( focus_list[i] );
		current_focus_item = i;
		
		if (SendMessage( focus_list[i], WM_GETDLGCODE, 0, 0 ) & DLGC_HASSETSEL)
			SendMessage( focus_list[i], EM_SETSEL, 0, (LPARAM)-1 );
		
    }
    void find_focus()
    {		
		HWND w = ::GetFocus();
		if( w )
		{
			for( int i=0; i<focus_list_len; ++i )
			{
				if( focus_list[i] == w )
				{
					current_focus_item=i;
					break;
				}
			}
		}
		else
		{
			current_focus_item=0;
		}
    }
    void next_focus()
    {
		find_focus();
		int i = current_focus_item+1;
		if( i >= focus_list_len )
		{
			i=0;
		}
		set_focus(i);
    }
};


#endif
