/*
**      $Id: wnrect.h 1632 2004-06-30 17:55:17Z jeffk $
**
**	Copyright 1986 to 1995 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/

#ifndef __WNRECT_H
#define __WNRECT_H

// Point and Rectangle classes.




///
class WNPoint
{

	
public:
    ///
	WNPoint(int a,int b )
						{ x=a;y=b;		}
			  WNPoint()		{ x=0;y=0;		}

    ///
	int     GetX() const		{ return x;		}
    ///
	int     GetY() const		{ return y;		}
    ///
	void    SetX( int a )   	{ x=a; 			}
    ///
	void    SetY( int b )   	{ y=b; 			}

    ///
	void    Add( const WNPoint &p )
						{ x+=p.x; y+=p.y;	}
    ///
	void    Subtract( const WNPoint &p )
						{ x-=p.x; y+=p.y; 	}

    ///
	bool     operator==( const WNPoint &p1 ) const
				{ if (p1.x==x && p1.y==y) return true;
				  else return false; }
				  
    ///
	bool     operator!=( const WNPoint &p1 ) const
				{ if (p1.x==x && p1.y==y) return false;
				  else return true;
				}

    ///
	int x;
	
	///
	int y;
};




///
class WNRect : public WNPoint 
{
public:

    ///
	WNRect( int a, int b, int c, int d ) : WNPoint ( a,b )
				{ w=c; h=d; }

    ///
	WNRect()	{ w=0; h=0; }

    ///
	WNRect( WNPoint  &a, WNPoint  &b ) : WNPoint ( a )
			  	{ w=b.x-a.x;  h=b.y-a.y; }

    ///
	void    SetCoords( int x1, int y1, int x2, int y2 )
				{ x=x1; y=y1; w=x2-x1+1; h=y2-y1+1; }

    ///
	void    Set( int x1, int y1, int w1, int h1 )
				{ x=x1; y=y1; w=w1; h=h1; }

    ///
	int     GetW()  const		{ return w; 	}
    ///	
	int     GetH()  const		{ return h;	}
    ///
	int     GetX1() const		{ return x+w-1; }
    ///
	int     GetY1() const		{ return y+h-1; }

    ///
	void    SetW( int a )   	{ w=a; 		}
    ///
	void    SetH( int b )   	{ h=b; 		}
    ///
	void    SetX1( int c )  	{ w=c-x+1; 	}
    ///
	void    SetY1( int d )  	{ h=d-y+1; 	}


    ///
	bool     Contains( const WNPoint &p ) const
				{
				if( p.x>=x && p.y>=y &&
					p.x<=x+w && p.y<=y+h )
						return true;
				else
						return false;
				}

    ///
	bool     operator==( const WNRect &r1 ) const
				{
				if( r1.x==x && r1.y==y && r1.w==w && r1.h==h )
					return true;
				else
					return false;
				}

    ///
	bool     operator!=( const WNRect &r1 ) const
				{
				if( r1.x==x && r1.y==y && r1.w==w && r1.h==h )
					return false;
				else
					return true;
				}

    ///
	int w;
	
	///
	int h;
};



#endif

