#include "jdk_world.h"
#include "wnworld.h"

#include "wnquestion.h"


static WNRegister WNDlgAskText(
	"jdk_ask_text",
	0,
	IDI_APPLICATION,
	IDC_ARROW,
	COLOR_BTNFACE
	);

AskTextWindow::AskTextWindow( 
							 WNDlgAskTextQuestionList &qlist_,
							 WNDlgAskTextQuestionEvent &event_
	) 
	: 
	WNWindow( "jdk_ask_text" ),
	qlist( qlist_ ),
	event( event_ ),
	width( qlist.width ),
    focus_list_len(0),
    current_focus_item(0)
{
	SetInitialStyle( WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX );
	SetInitialX(CW_USEDEFAULT);
    SetInitialY(CW_USEDEFAULT);
	SetInitialW(width);
    SetInitialH(qlist.info_lines*20+20+(qlist.num_questions)*25 + 100 );
}

AskTextWindow::~AskTextWindow()
{
}

bool AskTextWindow::Create()
{
    bool r = WNWindow::Create();
    if( r )
    {
		
		SetText( qlist.title.c_str() );
		label_info=AddLabel( 10, 10, width-30, qlist.info_lines*20, qlist.info.c_str() , SS_CENTER );
		
		int y=qlist.info_lines*20 + 20;
		for( int i=0; i<qlist.num_questions; ++i )
		{			
			const char *q = qlist.questions[i].question;
			const char *v = qlist.questions[i].initial_value;
#if DEBUG			
			fprintf( stderr, "q #%d: %s = %s\n", i, q, v );
#endif					
			label_question[i]=AddLabel( 
										20, y, 
										width/2-40, 20, 
										q, 
										SS_LEFT 
										);
			edit_value[i]=AddEdit( 
								   width/2, y, 
								   width/2-20, 20,
								   v,
								   qlist.questions[i].is_password ? ES_PASSWORD : 0
								   );
			add_focus_list( edit_value[i] );
			y+=25;
		}

		y+=30;
		button_cancel=AddButton( width-160, y, 60, 20, "Cancel",2 );
		add_focus_list( button_cancel );
		
		button_ok=AddButton( width-80, y, 60, 20, "OK",1, BS_DEFPUSHBUTTON );
		add_focus_list( button_ok );
		
		RECT rc;
		GetWindowRect( GetHWND(), &rc );
		SetWindowPos( GetHWND(), 0, 
					  ((GetSystemMetrics(SM_CXSCREEN)- (rc.right-rc.left))/2),
					  ((GetSystemMetrics(SM_CYSCREEN)- (rc.bottom-rc.top))/2),
					 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
		set_focus(0);

	}
	return r;
}

bool	AskTextWindow::ProcessAccelerator( MSG *m )
{
	if( m->message == WM_KEYDOWN )
	{
		if( m->wParam == VK_TAB )
		{
			SendMessage( 
						  GetHWND(),
						  WM_NEXTDLGCTL,
						  (GetKeyState(VK_SHIFT) & 0x8000), 0 
						  );
			
			return true;
		}
		if( m->wParam == VK_RETURN )
		{
			SendMessage( 
						  GetHWND(),
						  WM_COMMAND,
						  1, (LPARAM)button_ok
						  );
			
			return true;
		}
		if( m->wParam == VK_ESCAPE )
		{
			SendMessage( 
						  GetHWND(),
						  WM_COMMAND,
						  2, (LPARAM)button_cancel
						  );
			
			return true;
		}
		
	}
	return false;
}


LPARAM AskTextWindow::CommandMsg( WPARAM id, HWND but )
{
    switch( id )
    {
		case 1: // OK button
        {
            // get text value
			for( int i=0; i<qlist.num_questions; ++i )
			{				
				char buf[2048];
	            GetWindowText(edit_value[i], buf, 2047 );
				qlist.answers[i].cpy( buf );
			}
			
			event.ButtonOK( qlist );
            return 1;
        }
	case 2: // Cancel button
        {
			event.ButtonCancel();
            return 1;
        }
    }
	
    return WNWindow::CommandMsg( id, but );
}


LPARAM	AskTextWindow::NextDlgCtlMsg( WPARAM k, LPARAM l )		// WM_NEXTDLGCTL
{	
	next_focus();
	return 1;
//	return DefaultHandler();
}

