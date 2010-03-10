#ifndef __WNBRUSH_H
#define __WNBRUSH_H




class WNBrush
{
public:
	WNBrush( HDC dc, int r, int g, int b );
	WNBrush( HDC dc, HBITMAP bitmap );
	WNBrush( HDC dc, int style, int r, int g, int b );
	WNBrush( HDC dc, HBRUSH b );

	~WNBrush();

        operator HBRUSH();
protected:

	HBRUSH old;
	HBRUSH cur;
        HDC dc;
};

inline	WNBrush::~WNBrush()
{
	SelectObject(dc,old);
        DeleteObject(cur);
}

inline	WNBrush::WNBrush( HDC dc_, HBRUSH b )
{
	dc=dc_;
	cur=b;
        old=(HBRUSH)SelectObject(dc,cur);
}


inline	WNBrush::WNBrush( HDC dc_, int r, int g, int b )
{
	dc=dc_;
	cur=CreateSolidBrush( RGB(r,g,b) );
        old=(HBRUSH)SelectObject( dc,cur );
}

inline	WNBrush::WNBrush( HDC dc_, HBITMAP bitmap )
{
	dc=dc_;
	cur=CreatePatternBrush( bitmap );
        old=(HBRUSH)SelectObject( dc,cur );
}

inline	WNBrush::WNBrush( HDC dc_, int style, int r, int g, int b )
{
	dc=dc_;
	cur=CreateHatchBrush( style, RGB( r,g,b ) );
        old=(HBRUSH)SelectObject( dc, cur );
}



inline	WNBrush::operator HBRUSH()
{
	return cur;
}


#endif
