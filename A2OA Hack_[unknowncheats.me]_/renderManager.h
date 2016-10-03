#ifndef HEADER_RMANAGER
#define HEADER_RMANAGER

#include <d3d9.h>
#include <d3dx9.h>

#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <iostream>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")


class rManager
{
public:

	LPDIRECT3D9			d3d;
	LPDIRECT3DDEVICE9	device;

	~rManager();

	void			Initilize( LPDIRECT3D9 d3d, LPDIRECT3DDEVICE9 device );
	bool			Initilize( HWND hWND, unsigned short width, unsigned short height );

	void			drawText( const LPCSTR text, int x, int y, DWORD color );
	void			drawTextEx( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font );

	void			drawBox( int x, int y, int width, int height, DWORD color );
	void			drawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, DWORD color );

	void			FillRGB( int x, int y, int width, int height, DWORD color );
	void			drawBorder( int x, int y, int width, int height, int size, DWORD color1, DWORD color2 );

	void			createFont( LPSTR fontFamily, int height, int width, bool isBold, LPD3DXFONT *font );
	void			clear();
	void			present();

	int				calculateWidth( const LPCSTR text, ID3DXFont* font );

private:

	LPD3DXFONT			d_Font;
	ID3DXLine*			d_Line;

	void			_drawText( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font );
	void			_drawLine( ID3DXLine* line, int x, int y, int width, int height, DWORD color );
};


#endif