#ifndef HEADER_D3D9MENU
#define HEADER_D3D9MENU

#include <d3d9.h>
#include <d3dx9.h>

#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <iostream>

#include "renderManager.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

using namespace std;

typedef int( *func ) ( LPVOID args );

struct D3D9_checkBox
{
	string name;
	
	bool isEnabled;
	bool *value;

	D3D9_checkBox() :
		name( "NULL" ),
		isEnabled( false ),
		value( NULL )
	{}
};

struct D3D9_item
{
	std::string		name;
	std::string		itemVal;

	int				value, tabNumber;

	LPVOID			arguments;
	HANDLE			hThread;

	bool			isEnabled;
	bool			isPassive;
	bool			isThreaded;

	LPVOID			lpFunction;

	D3D9_item( ) :
		name( "NULL" ),
		itemVal( "NULL" ),
		arguments( NULL ),
		value( 0 ),
		hThread( NULL ),
		tabNumber( -1 ),
		isEnabled( false ),
		isPassive( false ),
		isThreaded( false ),
		lpFunction( NULL )
	{}
};

struct D3D9_textBox
{
	D3D9_item*  item;

	string		name, side;
	int			step;

	LPVOID		lpFunc;

	D3D9_textBox() :
		item( NULL ),
		name( "NULL" ),
		side( "NULL" ),
		step( 0 )
	{}
};

struct D3D9_slider
{
	D3D9_item*	item;
	string		name;

	int value, maxValue;

	D3D9_slider() :
		item( NULL ),
		name( "NULL" ),
		value( 0 ),
		maxValue( 0 )
	{}
};

struct D3D9_tab
{
	bool isSelected;
	string name;

	vector<D3D9_item*> items;
	vector<D3D9_slider*> sliders;
	vector<D3D9_textBox*> textBoxes;
	vector<D3D9_checkBox*> checkBoxes;

	D3D9_tab() :
		name( "NULL" ),
		isSelected( false )
	{}
};

struct D3D9_menu
{
	int					x, y, hotKey, currentTab;

	std::string			name;
	bool				isEnabled;

	vector<D3D9_tab*>	tabs;

	D3D9_menu() :
		name( "Empty" ),
		x( 0 ),
		y( 0 ),
		hotKey( 0 ),
		currentTab( 0 ),
		isEnabled( false )
	{}
};

class D3D9Menu
{
public:

	bool				initDevice( rManager* rMGR );
	void				clearDevice();
	LPDIRECT3DDEVICE9	getDevice() { return device; }

	int				calculateWidth( const LPCSTR text, ID3DXFont* font );

	void			drawText( const LPCSTR text, int x, int y, DWORD color );
	void			drawTextEx( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font );

	void			drawBox( int x, int y, int width, int height, DWORD color );
	void			drawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, DWORD color );

	void			FillRGB( int x, int y, int width, int height, DWORD color );
	void			drawBorder( int x, int y, int width, int height, int size, DWORD color1, DWORD color2 );

	void			createFont( LPSTR fontFamily, int height, int width, LPD3DXFONT *font );
	
	void			processItems( );

	void			render();

	void			addMenu( std::string name, int x, int y, int hotKey );
	D3D9_item*		addItem( string name, int value, string itemVal, LPVOID lpFunction, LPVOID arguments, bool isPassive, bool isThreaded, int tabNumber );
	void			addTab( string name );
	void			addSlider( D3D9_item* item, string name, int maxValue );
	void			addTextBox( D3D9_item* item, string name, LPVOID lpFunc );
	void			addCheckBox( string name, bool *value, int tabNumber );

	void			setTab( D3D9_tab* tab );
	
	D3D9_item*		getItem( int index, D3D9_tab *tab );
	D3D9_tab*		getTab( int index );

	void			drawSlider( D3D9_slider* slider, int sliderX, int sliderY );
	void			drawItem( D3D9_item* item, int itemX, int itemY );
	void			drawCheckBox( D3D9_checkBox* checkBox, int checkBoxX, int checkBoxY );
	void			drawTab( D3D9_tab* tab, int tabX, int tabY );
	void			drawTextBox( D3D9_textBox* textBox, int boxX, int boxY );

	int				handleMouse( int x, int y, int x2, int y2 );
	void			handleInput( );

private:
	void			_drawText( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font );
	void			_drawLine( ID3DXLine* line, int x, int y, int width, int height, DWORD color );

	LPDIRECT3D9			d3d;
	LPDIRECT3DDEVICE9	device;

	LPD3DXFONT			d_Font;
	ID3DXLine*			d_Line;

	D3D9_menu			menu;
	rManager*			rMGR;

};

class D3D9Console
{
public:

	D3D9Console( D3D9Menu* menu );

	void render( );
	void handleInput( );

private:
	string			command;
	vector<string>	prevCommands;

	D3D9Menu*		menu;

	int		carretPos, carretTick, cmdCount, cmdCurrent;
	bool	showCarret, isEnabled, isHandled;
};

#endif