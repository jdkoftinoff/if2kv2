#ifndef __WNPEN_H
#define __WNPEN_H




class WNPen
{
public:
	WNPen( HDC dc, int style, int width, int r, int g, int b );
	WNPen( HDC dc, HPEN p );

	~WNPen();

        operator HPEN();
protected:

	HPEN old;
	HPEN cur;
        HDC dc;
};


inline	WNPen::WNPen( HDC dc_, HPEN p )
{
	dc=dc_;
	cur=p;
        old=(HPEN)SelectObject(dc,p);
}

inline 	WNPen::WNPen( HDC dc_, int style, int width, int r, int g, int b )
{
	dc=dc_;
	cur=CreatePen( style, width, RGB(r,g,b) );
	old=(HPEN)SelectObject(dc,cur);
}

inline 	WNPen::~WNPen()
{
	SelectObject(dc,old);
        DeleteObject(cur);
}

inline	WNPen::operator HPEN()
{
	return cur;
}


#endif
