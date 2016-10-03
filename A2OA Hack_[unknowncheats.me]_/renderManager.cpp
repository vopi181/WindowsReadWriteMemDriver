#include "renderManager.h"

rManager::~rManager( )
{
	if( this->device != NULL )
		this->device->Release();

	if( this->d3d != NULL )
		this->d3d->Release();

	if( this->d_Font != NULL )
		this->d_Font->Release();

	if( this->d_Line != NULL )
		this->d_Line->Release();
}

bool	rManager::Initilize( HWND hWND, unsigned short width, unsigned short height )
{
	if( FAILED( d3d = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		MessageBox( nullptr, L"Failed to create D3D9.", L"Error", 0 );
		return false;
	}

	D3DPRESENT_PARAMETERS dxParams;
	ZeroMemory( &dxParams, sizeof( dxParams ) );

	dxParams.hDeviceWindow = hWND;
	dxParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	dxParams.Windowed = TRUE;
	dxParams.BackBufferFormat = D3DFMT_A8R8G8B8;
	dxParams.BackBufferHeight = height;
	dxParams.BackBufferWidth = width;

	if( FAILED( d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWND, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &dxParams, &device ) ) )
	{
		MessageBox( nullptr, L"Failed to create D3D9 device.", L"Error", 0 );
		return false;
	}

	return true;
}

void	rManager::Initilize( LPDIRECT3D9 d3d, LPDIRECT3DDEVICE9 device )
{
	if( d3d == nullptr || device == nullptr )
	{
		MessageBox( nullptr, L"Failed to create D3D9 because nullptr was passed.", L"Error", 0 );
		return;
	}

	this->d3d		= d3d;
	this->device	= device;
}

void	rManager::createFont( LPSTR fontFamily, int height, int width, bool isBold, LPD3DXFONT *font )
{
	D3DXCreateFontA( device, height, width, 350, (isBold ? FW_BOLD : FW_NORMAL), FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFamily, font );
}

int		rManager::calculateWidth( const LPCSTR text, ID3DXFont* font )
{
	if( !font )
	{
		if( !this->d_Font )
		{
			this->createFont( "Arial", 18, 0, FALSE, &this->d_Font );
			font = this->d_Font;
		}
	}


	char buffer[ 200 ] = { '\0' };
	RECT rect = { 0, 0, 0, 0 };

	va_list va_lst;
	va_start( va_lst, text );
	vsprintf_s( buffer, text, va_lst );
	va_end( va_lst );

	font->DrawTextA( NULL, buffer, -1, &rect, DT_CALCRECT, D3DCOLOR_ARGB( 0, 0, 0, 0 ) );
	return rect.right - rect.left;
}

void	rManager::_drawText( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font )
{
	if( !font || x < 0 || y < 0 )
		return;

	char buffer[ 500 ] = { '\0' };
	RECT rect = { x, y, x + 1000, y + 1000 };

	va_list va_lst;
	va_start( va_lst, text );
	vsprintf_s( buffer, text, va_lst );
	va_end( va_lst );

	font->DrawTextA( NULL, buffer, -1, &rect, DT_NOCLIP | FF_DONTCARE, color );
	return;
}

void	rManager::_drawLine( ID3DXLine* line, int x, int y, int width, int height, DWORD color )
{
	if( !line || x < 0 || y < 0 )
		return;

	D3DXVECTOR2 vec2[ 2 ];
	vec2[ 0 ].x = (float) x;
	vec2[ 0 ].y = (float) y;

	vec2[ 1 ].x = (float) width;
	vec2[ 1 ].y = (float) height;

	line->Draw( vec2, 2, color );
}

void	rManager::drawBox( int x, int y, int width, int height, DWORD color )
{
	if( this->d_Line == NULL )
	{
		D3DXCreateLine( device, &d_Line );
	}


	if( this->d_Line == NULL )
		return;

	_drawLine( d_Line, x, y, width + x, y, color ); // TOP LINE -----
	_drawLine( d_Line, x, y, x, y + height, color ); // | LEFT LINE
	_drawLine( d_Line, x + width, y, x + width, y + height, color ); // | RIGHT LINE
	_drawLine( d_Line, x, y + height, width + x, y + height, color ); // BOTTOM LINE ----
}

void	rManager::drawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, DWORD color )
{
	if( this->d_Line == NULL )
	{
		D3DXCreateLine( device, &d_Line );
	}

	_drawLine( d_Line, x1, y1, x2, y2, color );
	_drawLine( d_Line, x1, y1, x3, y3, color );
	_drawLine( d_Line, x2, y2, x3, y3, color );
}

void	rManager::drawText( const LPCSTR text, int x, int y, DWORD color )
{
	if( this->d_Font == NULL )
		this->createFont( "Tahoma", 18, 0, FALSE, &this->d_Font );

	this->_drawText( text, x, y, color, this->d_Font );
}

void	rManager::drawTextEx( const LPCSTR text, int x, int y, DWORD color, ID3DXFont *font )
{
	this->_drawText( text, x, y, color, font );
}

void	rManager::FillRGB( int x, int y, int width, int height, DWORD color )
{
	D3DRECT rect = { x, y, x + width, y + height };
	this->device->Clear( 1, &rect, D3DCLEAR_TARGET, color, NULL, NULL );
}

void	rManager::drawBorder( int x, int y, int width, int height, int size, DWORD color1, DWORD color2 )
{
	FillRGB( x, y, width, size, color1 ); // TOP LINE -----
	FillRGB( x, y, size, height, color2 ); // | LEFT LINE
	FillRGB( x, ( y + height - size ), width, size, color2 ); // | RIGHT LINE
	FillRGB( ( x + width - size ), y, size, height, color1 ); // BOTTOM LINE ----
}

void	rManager::clear() {
	this->device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	this->device->BeginScene();
}

void	rManager::present() {
	this->device->EndScene();
	this->device->Present(NULL, NULL, NULL, NULL);
}