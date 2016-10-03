#include "D3D9_Menu.h"

///////////////////////////////////////////////////////////////
///////             DRAWING FUNCTIONS               ///////////
//////////////////////////////////////////////////////////////

#pragma region DRAWING

int	 D3D9Menu::calculateWidth( const LPCSTR text, ID3DXFont* font )
{
	if( !font )
	{
		if( !this->d_Font )
		{
			this->createFont( "Arial", 18, 0, &this->d_Font );
			font = this->d_Font;
		}
	}
	

	char buffer[ 200 ] = { '\0' };
	RECT rect = { 0, 0, 0, 0 };

	va_list va_lst;
	va_start( va_lst, text );
	vsprintf_s( buffer, text, va_lst );
	va_end( va_lst );

	font->DrawTextA( NULL, buffer, -1, &rect, DT_CALCRECT, D3DCOLOR_ARGB( 0,0,0,0 ) );
	return rect.right - rect.left;
}

void D3D9Menu::_drawText( const LPCSTR text, int x, int y, DWORD color, ID3DXFont* font )
{
	if( !font || x < 0 || y < 0)
		return;

	char buffer[ 200 ] = { '\0' };
	RECT rect = { x, y, x + 1000, y + 1000 };

	va_list va_lst;
	va_start( va_lst, text );

	vsprintf_s( buffer, text, va_lst );

	va_end( va_lst );

	font->DrawTextA( NULL, buffer, -1, &rect, DT_NOCLIP | FF_DONTCARE, color );
	return;
}

void D3D9Menu::_drawLine( ID3DXLine* line, int x, int y, int width, int height, DWORD color )
{
	if( !line || x < 0 || y < 0 )
		return;

	D3DXVECTOR2 vec2[ 2 ];
	vec2[ 0 ].x = (float)x;
	vec2[ 0 ].y = (float)y;

	vec2[ 1 ].x = (float)width;
	vec2[ 1 ].y = (float)height;

	line->Draw( vec2, 2, color );
}

void D3D9Menu::drawBox( int x, int y, int width, int height, DWORD color )
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

void D3D9Menu::drawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, DWORD color )
{
	if( this->d_Line == NULL )
	{
		D3DXCreateLine( device, &d_Line );
	}

	_drawLine( d_Line, x1, y1, x2, y2, color );
	_drawLine( d_Line, x1, y1, x3, y3, color );
	_drawLine( d_Line, x2, y2, x3, y3, color );
}

void  D3D9Menu::drawText( const LPCSTR text, int x, int y, DWORD color )
{
	if( this->d_Font == NULL )
		this->createFont( "Arial", 18, 0, &this->d_Font );

	if( this->d_Font == NULL )
	{
		return;
	}

	this->_drawText( text, x, y, color, this->d_Font );
}

void D3D9Menu::drawTextEx( const LPCSTR text, int x, int y, DWORD color, ID3DXFont *font )
{
	this->_drawText( text, x, y, color, font );
}

void D3D9Menu::FillRGB( int x, int y, int width, int height, DWORD color )
{
	D3DRECT rect = { x, y, x + width, y + height };
	this->device->Clear( 1, &rect, D3DCLEAR_TARGET, color, NULL, NULL );
}

void D3D9Menu::drawBorder( int x, int y, int width, int height, int size, DWORD color1, DWORD color2 )
{
	FillRGB( x, y, width, size, color1 ); // TOP LINE -----
	FillRGB( x, y, size, height, color2 ); // | LEFT LINE
	FillRGB( x, ( y + height - size ), width, size, color2 ); // | RIGHT LINE
	FillRGB( (x + width - size), y, size, height, color1 ); // BOTTOM LINE ----
}

#pragma endregion DRAWING

///////////////////////////////////////////////////////////////
///////             DEVICE FUNCTIONS                ///////////
//////////////////////////////////////////////////////////////

#pragma region DEVICE

void D3D9Menu::createFont( LPSTR fontFamily, int height, int width, LPD3DXFONT *font )
{
	D3DXCreateFontA( device, height, width, 350, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFamily, font );
}

void D3D9Menu::clearDevice( )
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

bool D3D9Menu::initDevice( rManager* rMGR )
{
	if( rMGR == nullptr )
		return false;

	this->rMGR		= rMGR;

	this->device	= rMGR->device;
	this->d3d		= rMGR->d3d;

	return true;
}

#pragma endregion DEVICE

///////////////////////////////////////////////////////////////
///////             MENU FUNCTIONS                 ///////////
//////////////////////////////////////////////////////////////

#pragma region MENU

void D3D9Menu::render()
{
	processItems();
	int tabY = menu.y + 30, itemY = menu.x + 40, menuW = 450, sliderY = menu.y + menuW - 20, menuH = 500;

	if( menu.isEnabled )
	{
		FillRGB( menu.x, menu.y, menuW, menuH, D3DCOLOR_ARGB( 255, 34, 35, 39 ) );

		FillRGB( menu.x, menu.y, menuW, 20, D3DCOLOR_ARGB( 255, 44, 45, 46 ) );
		drawText( menu.name.c_str( ), ( menu.x + menuW ) / 2 - ( calculateWidth( menu.name.c_str(), this->d_Font ) ) / 2, menu.y + 2, D3DCOLOR_ARGB( 255, 255, 255, 255 ) );

		drawBorder( menu.x + 200, menu.y + 20, 250, menuH - 20, 10, D3DCOLOR_ARGB( 255, 18, 19, 20 ), D3DCOLOR_ARGB( 255, 18, 19, 20 ) );
		drawBorder( menu.x, menu.y, menuW, menuH, 1, D3DCOLOR_ARGB( 255, 0, 0, 0 ), D3DCOLOR_ARGB( 255, 0, 0, 0 ) );
		
		
		for( D3D9_tab* tab : menu.tabs )
		{
			drawTab( tab, menu.x, tabY );

			if( tab->isSelected )
			{
				for( D3D9_item* item : tab->items )
				{
					drawItem( item, menu.x + 200, itemY );
					itemY += 30;
				}

				for( D3D9_checkBox* checkBox : tab->checkBoxes )
				{
					drawCheckBox( checkBox, menu.x + 200, itemY );
					itemY += 30;
				}

				for( D3D9_slider* slider : tab->sliders )
				{
					drawSlider( slider, menu.x + 200, sliderY );
					sliderY -= 60;
				}

				for( D3D9_textBox* textBox : tab->textBoxes )
				{
					drawTextBox( textBox, menu.x + 200, sliderY );
					sliderY -= 60;
				}
			}

				
			tabY += 30;
		}
	}
}

void D3D9Menu::processItems()
{

	for( D3D9_tab* tab : menu.tabs )
	{
		for( D3D9_item* item : tab->items )
		{
			if( item->isEnabled && item->isPassive && !item->isThreaded )
			{
				func function = (func) item->lpFunction;

				if (function != NULL) {
					function( item );
				}
			} else if( item->isEnabled && item->isThreaded )
			{
				if( item->hThread )
				{
					DWORD threadStatus = WaitForSingleObject( item->hThread, 0 );

					if( threadStatus != WAIT_OBJECT_0 )
						continue;
				}

				item->hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)item->lpFunction, item, NULL, NULL );
			}
		}
	}
}

// ADDING

D3D9_item* D3D9Menu::addItem( string name, int value, string itemVal, LPVOID lpFunction, LPVOID arguments, bool isPassive, bool isThreaded, int tabNumber )
{

	D3D9_item* item = new D3D9_item;

	item->name				= name;
	item->value				= value;
	item->itemVal			= itemVal;
	item->lpFunction		= lpFunction;
	item->arguments			= arguments;
	item->isPassive			= isPassive;
	item->isThreaded		= isThreaded;
	item->tabNumber			= tabNumber;

	if( tabNumber > -1 )
	{
		D3D9_tab* tab = getTab( tabNumber );
		if( !tab )
			return NULL;

		tab->items.push_back( item );
	}

	return item;
}

void D3D9Menu::addMenu( std::string name, int x, int y, int hotKey )
{
	menu.name		= name;
	menu.x			= x;
	menu.y			= y;
	menu.hotKey		= hotKey;
}

void D3D9Menu::addTab( string name )
{
	D3D9_tab* tab = new D3D9_tab;

	tab->name = name;
	menu.tabs.push_back( tab );
}

void D3D9Menu::addSlider( D3D9_item* item, string name, int maxValue )
{
	D3D9_slider* slider = new D3D9_slider;

	slider->item			= item;
	slider->name			= name;
	slider->maxValue		= maxValue;
	slider->value			= item->value;

	D3D9_tab* tab = getTab( item->tabNumber );
	if( !tab )
		return;

	tab->sliders.push_back( slider );
	return;
}

void D3D9Menu::addTextBox( D3D9_item* item, string name, LPVOID lpFunc )
{
	D3D9_textBox* textBox = new D3D9_textBox;

	textBox->item			= item;
	textBox->name			= name;
	textBox->step			= 0;
	textBox->lpFunc			= lpFunc;

	D3D9_tab* tab = getTab( item->tabNumber );
	if( !tab )
		return;

	tab->textBoxes.push_back( textBox );
	return;
}

void D3D9Menu::addCheckBox( string name, bool* value, int tabNumber )
{
	D3D9_checkBox* checkBox = new D3D9_checkBox;

	checkBox->name		= name;
	checkBox->value		= value;
	checkBox->isEnabled	= *value;

	D3D9_tab* tab = getTab( tabNumber );
	if( !tab )
		return;

	tab->checkBoxes.push_back( checkBox );
	return;
}

// GETTING

D3D9_item* D3D9Menu::getItem( int index, D3D9_tab *tab )
{

	return NULL;
}

D3D9_tab*  D3D9Menu::getTab( int index )
{
	D3D9_tab* tab = NULL;

	try{
		tab = menu.tabs.at( index );
	} catch( out_of_range e )
	{
		return NULL;
	}

	return tab;
}

void D3D9Menu::setTab( D3D9_tab* tab )
{
	for( D3D9_tab* tabs : menu.tabs )
		if( tabs->isSelected )
			tabs->isSelected = false;

	tab->isSelected = true;
}

void D3D9Menu::drawCheckBox( D3D9_checkBox* checkBox, int cehckBoxX, int cehckBoxY )
{
	int mouseState = 0;
	mouseState = handleMouse( cehckBoxX + 10, cehckBoxY, cehckBoxX + 240, cehckBoxY + 30 );

	if( mouseState == 2 )
	{
		checkBox->isEnabled = !checkBox->isEnabled;

		if( checkBox->value )
		{
			*checkBox->value = checkBox->isEnabled;
		}
	}

	if( checkBox->isEnabled )
	{
		if( mouseState == 1 )
		{
			FillRGB( cehckBoxX + 10, cehckBoxY - 3, 230, 30, D3DCOLOR_ARGB( 255, 43, 44, 50 ) );
		}

		FillRGB( cehckBoxX + 10, cehckBoxY + 2, 5, 20, D3DCOLOR_ARGB( 255, 194, 0, 31 ) );
		drawText( checkBox->name.c_str(), cehckBoxX + ( 240 - calculateWidth( checkBox->name.c_str(), d_Font ) ) / 2, cehckBoxY, D3DCOLOR_ARGB( 255, 162, 164, 168 ) );
		return;
	}

	if( mouseState == 1 )
	{
		FillRGB( cehckBoxX + 10, cehckBoxY - 3, 230, 30, D3DCOLOR_ARGB( 255, 43, 44, 50 ) );
	}

	drawText( checkBox->name.c_str(), cehckBoxX + ( 240 - calculateWidth( checkBox->name.c_str(), d_Font ) ) / 2, cehckBoxY, D3DCOLOR_ARGB( 255, 148, 149, 153 ) );

}

void D3D9Menu::drawItem( D3D9_item* item, int itemX, int itemY )
{
	// ITEM LOOP
	int mouseState = 0;
	mouseState = handleMouse( itemX + 10, itemY, itemX + 240, itemY + 30 );
	
	if( mouseState == 2 )
	{
		item->isEnabled = !item->isEnabled;

		if( item->isEnabled && !item->isPassive && !item->isThreaded )
		{
			func function = (func) item->lpFunction;

			if( function != NULL )
				function( item );
		}
	}

	if( item->isEnabled )
	{
		if( mouseState == 1 )
		{
			FillRGB( itemX + 10, itemY - 3, 230, 30, D3DCOLOR_ARGB( 255, 43, 44, 50 ) );
		}

		FillRGB( itemX + 10, itemY + 2, 5, 20, D3DCOLOR_ARGB( 255, 148, 216, 0 ) );
		drawText( item->name.c_str( ), itemX + ( 240 - calculateWidth( item->name.c_str( ), d_Font ) ) / 2, itemY, D3DCOLOR_ARGB( 255, 162, 164, 168 ) );
		return;
	}

	if( mouseState == 1 )
	{
		FillRGB( itemX + 10, itemY - 3, 230, 30, D3DCOLOR_ARGB( 255, 43, 44, 50 ) );
	}

	drawText( item->name.c_str( ), itemX + ( 240 - calculateWidth( item->name.c_str( ), d_Font ) ) / 2, itemY, D3DCOLOR_ARGB( 255, 148, 149, 153 ) );
}

void D3D9Menu::drawSlider( D3D9_slider* slider, int sliderX, int sliderY )
{
	// SLIDER WIDTH - 170.
	int curX = sliderX + 40, curY = sliderY + 30;
	int sliderPos = 0;
	
	if( slider->value > 0 && slider->maxValue > 0 )
		sliderPos = (int)ceil(float(slider->value) / float(slider->maxValue) * 170.0f); // CALCULATING PERCENTAGE

	drawText( slider->name.c_str( ), sliderX + ( 240 - calculateWidth( slider->name.c_str( ), d_Font ) ) / 2, sliderY, D3DCOLOR_ARGB( 255, 247, 119, 7 ) );
	FillRGB( curX, curY, 170, 5, D3DCOLOR_ARGB( 255, 18, 19, 20 ) );
	drawText( to_string( slider->value ).c_str( ), sliderX + ( 240 - calculateWidth( to_string( slider->value ).c_str( ), d_Font ) ) / 2, sliderY + 40, D3DCOLOR_ARGB( 255, 247, 119, 7 ) );

	FillRGB( curX + sliderPos, sliderY + 23, 5, 20, D3DCOLOR_ARGB( 255, 18, 19, 20 ) );

	if( GetAsyncKeyState( VK_LBUTTON ) )
	{
		POINT p;
		GetCursorPos( &p );

		if( p.x >= curX && p.x <= curX + 170 )
		{
			if( p.y > sliderY + 23 && p.y < sliderY + 43 )
			{
				float value = (float(p.x) - float(curX) ) / 170.0f; // CALCULATING PERCENTAGE
				slider->value = (int)( slider->maxValue * value );

				slider->item->value = slider->value;
			}
		}
	}
}

void D3D9Menu::drawTab( D3D9_tab* tab, int tabX, int tabY )
{
	int mouseState = 0;
	mouseState = handleMouse( menu.x, tabY, 210, tabY + 30 );

	if( mouseState == 2 )
		setTab( tab );

	if( tab->isSelected )
	{
		FillRGB( tabX, tabY - 3, 200, 30, D3DCOLOR_ARGB( 255, 46, 47, 53 ) ); // WHITE BACKGROUND
		FillRGB( tabX, tabY + 2, 5, 20, D3DCOLOR_ARGB( 255, 148, 216, 0 ) ); // GREEN INDICATOR
		drawText( tab->name.c_str( ), tabX + 20, tabY, D3DCOLOR_ARGB( 255, 162, 164, 168 ) ); // TAB NAME
	} else{
		if( mouseState == 1 )
		{
			FillRGB( tabX, tabY - 3, 200, 30, D3DCOLOR_ARGB( 255, 43, 44, 50 ) );
			drawText( tab->name.c_str( ), tabX + 20, tabY, D3DCOLOR_ARGB( 255, 148, 149, 153 ) );
		} else{
			drawText( tab->name.c_str( ), tabX + 20, tabY, D3DCOLOR_ARGB( 255, 148, 149, 153 ) );
		}
	}
}

void D3D9Menu::drawTextBox( D3D9_textBox* textBox, int boxX, int boxY )
{
	int curX = boxX + 45;

	FillRGB( curX, boxY, 160, 25, D3DCOLOR_ARGB( 255, 18, 19, 20 ) ); // BOX
	FillRGB( boxX + 20, boxY, 25, 25, D3DCOLOR_ARGB( 255, 162, 164, 168 ) ); // LEFT SLIDER
	FillRGB( curX + 160, boxY, 25, 25, D3DCOLOR_ARGB( 255, 162, 164, 168 ) ); // RIGHT SLIDER

	drawTriangle( boxX + 35, boxY + 3, boxX + 25, boxY + 13, boxX + 35, boxY + 23, D3DCOLOR_ARGB( 255, 18, 19, 20 ) );
	drawTriangle( boxX + 213, boxY + 3, boxX + 223, boxY + 13, boxX + 213, boxY + 23, D3DCOLOR_ARGB( 255, 18, 19, 20 ) );

	drawText( textBox->item->name.c_str( ), curX + 80 - calculateWidth( textBox->item->name.c_str( ), d_Font ) / 2, boxY - 18, D3DCOLOR_ARGB( 255, 247, 119, 7 ) );
	drawText( textBox->item->itemVal.c_str( ), curX + 80 - calculateWidth( textBox->item->itemVal.c_str( ), d_Font ) / 2, boxY + 3, D3DCOLOR_ARGB( 255, 163, 34, 8 ) );

	if( handleMouse( boxX + 20, boxY, boxX + 45, boxY + 25 ) == 2 ) // LEFT BOX
	{
		textBox->side = "LEFT";

		func function = (func)textBox->lpFunc;
		if( function != NULL )
		{
			function( textBox );
		}

	} else if( handleMouse( curX + 160, boxY, curX + 205, boxY + 25 ) == 2 ) // RIGHT BOX
	{
		textBox->side = "RIGHT";

		func function = (func) textBox->lpFunc;
		if( function != NULL )
		{
			function( textBox );
		}
	}

}

#pragma endregion MENU


///////////////////////////////////////////////////////////////
///////             INPUT FUNCTIONS                 ///////////
//////////////////////////////////////////////////////////////

#pragma region INPUT

/*
///////////////////////////////////////////////////////////////
 RETURNS 1 - MOUSE IS HOVERING 2 - MOUSE IS PRESSED
//////////////////////////////////////////////////////////////
*/

int D3D9Menu::handleMouse( int x, int y, int x2, int y2 )
{
	int		timer1		= clock();
	bool	btn_LMOUSE	= GetAsyncKeyState( VK_LBUTTON ) != 0;

	if( !menu.isEnabled )
		return 0;

	POINT p;
	GetCursorPos( &p );

	if( p.x > x && p.x < x2 )
	{
		if( p.y > y && p.y < y2 )
		{
			if( !btn_LMOUSE )
				return 1;

			while( true )
			{
				if( ( clock() - timer1 > 150 ) )
				{
					return 2;
				}
			}
		}
	}
		
	return 0;
}

void D3D9Menu::handleInput()
{
	bool btn_hotKey = GetAsyncKeyState( menu.hotKey ) & 1;

	if( btn_hotKey )
		menu.isEnabled = !menu.isEnabled;

	return;
}

#pragma endregion INPUT








///////////////////////////////////////////////////////////////
///////             D3D9 CONSOLE                    ///////////
//////////////////////////////////////////////////////////////

D3D9Console::D3D9Console( D3D9Menu* menu )
{
	this->menu = menu;
}

void D3D9Console::render()
{
	menu->FillRGB( 500, 500, 200, 20, D3DCOLOR_ARGB( 255, 0, 0, 0 ) );

	return;
}

void D3D9Console::handleInput()
{
	if( !isEnabled )
		return;

	int		timer1 = clock();

	bool	btn_LMOUSE		= GetAsyncKeyState( VK_LBUTTON ) != 0;
	bool	btn_enter		= GetAsyncKeyState( VK_RETURN )  != 0;
	bool	btn_backspace	= GetAsyncKeyState( VK_BACK )    != 0;

	
}