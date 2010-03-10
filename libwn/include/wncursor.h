#ifndef __WNCURSOR_H
#define __WNCURSOR_H


#include "wnmain.h"




class WNCursorBlock
{
public:
	WNCursorBlock( HCURSOR c );
	WNCursorBlock( LPCSTR predefined_cursor = IDC_WAIT );
	~WNCursorBlock();

protected:
	HCURSOR prev;
};


inline 	WNCursorBlock::WNCursorBlock( HCURSOR c )
{
	prev=SetCursor( c );
}

inline	WNCursorBlock::WNCursorBlock( LPCSTR predefined_cursor )
{
	HCURSOR c = LoadCursor( 0, predefined_cursor );
        prev=SetCursor( c );
}

inline	WNCursorBlock::~WNCursorBlock()
{
	SetCursor( prev );
}





#endif

