#ifndef __WNDC_H
#define __WNDC_H



#include "wnwindow.h"




class WNDC
{
public:
	WNDC( WNWindow *w );
	WNDC( HWND hWnd );
	WNDC( HDC dc_, BOOL deletable_ );
	WNDC();

	~WNDC();

	operator HDC ();

	void Set( WNWindow *w );
	void Set( HWND hWnd );
	void Set( HDC dc_, BOOL deletable_ );
	
protected:
	HDC	dc;
	HWND 	hWnd;
	BOOL deletable;
};

inline WNDC::WNDC()
{
	dc=0;
	hWnd=0;
	deletable=0;
}

inline WNDC::WNDC( WNWindow *w )
{
	hWnd=w->GetHWND();
	dc=GetDC( hWnd );
	deletable=FALSE;
}

inline WNDC::WNDC( HWND w )
{
	hWnd=w;
	dc=GetDC( hWnd );
	deletable=FALSE;
}

inline WNDC::WNDC( HDC dc_, BOOL deletable_ )
{
	dc=dc_;
	deletable=deletable_;
}

inline WNDC::~WNDC()
{
	if( deletable )
		DeleteDC( dc );
	else
		ReleaseDC(hWnd,dc);		
}

inline WNDC::operator HDC()
{
	return dc;
}

inline void WNDC::Set( WNWindow *w )
{
	hWnd=w->GetHWND();
	dc=GetDC( hWnd );
	deletable=FALSE;
}

inline void WNDC::Set( HWND hWnd )
{
	dc=GetDC( hWnd );
	deletable=FALSE;
}

inline void WNDC::Set( HDC dc_, BOOL deletable_ )
{
	dc=dc_;
	deletable=deletable_;
}


#endif
