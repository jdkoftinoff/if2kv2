#ifndef __WNBITMAP_H
#define __WNBITMAP_H


#include "wnmain.h"


///
class WNBitMap
{
public:
    ///
	WNBitMap( char *name );	
    ///
	WNBitMap();
    ///
	~WNBitMap();
	
    ///
	bool LoadRsrc( HMODULE module, const char *name );

    ///
	HBITMAP GetBitMap();
    ///
	HPALETTE GetPalette();
	
    ///
	operator HBITMAP();
    ///
	operator HPALETTE();
protected:
	HBITMAP bitmap;
	HPALETTE palette;
};

inline WNBitMap::WNBitMap()
{
    bitmap=0;	
	palette=0;
}


inline WNBitMap::WNBitMap( char *name )
{
	LoadRsrc( hInstance, name );
}


inline WNBitMap::~WNBitMap()
{
	if( bitmap )
    	DeleteObject( bitmap );
	if( palette )
    	DeleteObject( palette );
}

inline WNBitMap::operator HBITMAP()
{
 	return bitmap;
}

inline WNBitMap::operator HPALETTE()
{
 	return palette;
}


inline	HBITMAP WNBitMap::GetBitMap()
{
    return bitmap;	
}

inline  HPALETTE WNBitMap::GetPalette()
{
    return palette;
}


#endif

