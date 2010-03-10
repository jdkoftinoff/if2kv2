
#include "wnworld.h"

#include "wnwindow.h"
#include "wnmain.h"
#include "wnreg.h"


#if !DEBUG_WNREG
# undef DBG
# define DBG(a)
#endif



WNRegister *WNRegister::top=0;
	

WNRegister::WNRegister(
		LPCSTR 	classname_,
		LPCSTR	menuname_,
		LPCSTR	iconname_,
		LPCSTR	cursorname_,
                LONG	background_,
		UINT	style_,
                int	wextra_,
                WNDPROC	proc_
		)
{
	classname=classname_;
	style=style_;
	menuname=menuname_;
	iconname=iconname_;
	cursorname=cursorname_;
	background=background_;
	registered=FALSE;
	next=0;

	wextra=wextra_;

	if( proc_==0 )
		proc_=WNWindow::GetWNDPROC();

	proc=proc_;

	if( !top )
	{
		top=this;
	}
	else
	{
		WNRegister *r=top;

		while( r->next )
			r=r->next;

		r->next=this;	
	}
}

WNRegister::~WNRegister()
{
	if( registered )
		UnregisterClass( classname, hInstance );
}

BOOL WNRegister::RegisterAll()
{
	if( !hPrevInstance )
	{	
		WNDCLASS c;

		WNRegister *r=top;

		while( r )
		{			
			c.style=r->style;
			c.lpfnWndProc = r->proc;
			c.cbClsExtra = 0;
			c.cbWndExtra = r->wextra;
			c.hInstance = hInstance;

			if( HIWORD(r->iconname)==0 )
				c.hIcon=LoadIcon(0,r->iconname);
			else
				c.hIcon = LoadIcon( hInstance, r->iconname );

			if( HIWORD(r->cursorname)==0 )
				c.hCursor = LoadCursor(0, r->cursorname );
			else			
				c.hCursor = LoadCursor( hInstance, r->cursorname );

			c.lpszMenuName = r->menuname;
			c.lpszClassName = r->classname;

			c.hbrBackground = (HBRUSH)(r->background+1);

			if( !RegisterClass(&c) )
				return FALSE;
				
			r->registered=TRUE;
			
			r=r->next;
			
		}
	}
	return TRUE;
}






