#ifndef HEADER_INC
#define HEADER_INC

#include <Dwmapi.h> 
#include <TlHelp32.h>
#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>

#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9_Menu.h"
#include "main.h"


#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"Dwmapi.lib")


	D3D9Menu			d3d9;
	LPD3DXFONT			font = NULL;

	int					itemRef				= 1;


	bool				displayEmptyCars	= false;
	bool				displayPlayers		= true;
	bool				displayCars			= true;
	bool				displayItems		= false;


	DWORD getPlayerByName( string plyrName, Memory* m )
	{
		Player p( m );

		DWORD entitySize = 0;
		entitySize = m->readPointer( dObjectSize, 3 );

		for( size_t i = 0; i < entitySize; i++ )
		{
			p.refresh();
			p.setPlayer( i );

			if( !p.isPlayer() || !p.isAlive() )
				continue;

			if( p.entity == localPlayer )
				continue;

			string pName = p.getName();
			if( pName.length() > 0 )
			{
				if( pName == plyrName )
				{
					DWORD retVal = 0;
					retVal = m->readPointer( dObject, 3 );

					return m->read<int>( retVal + ( i * 0x34 ) );
				}
			}

		}

		return NULL;
	}

	DWORD getPlayerByNumber( int nPlayer, Memory* m, string* pPtr )
	{
		if( nPlayer < 1 || !m->isAttached() )
			return NULL;

		Player p( m );

		int pCount = 0;
		DWORD entitySize = 0;
		entitySize = m->readPointer( dObjectSize, 3 );

		if( entitySize > 5000 )
			return NULL;

		for( size_t i = 0; i < entitySize; i++ )
		{
			p.refresh();
			p.setPlayer( i );

			if( !p.isPlayer() || !p.isAlive() )
				continue;

			if( p.entity == localPlayer )
				continue;

			pCount++;

			if( nPlayer == pCount )
			{
				string pName = p.getName();
				if( pName.length() > 0 )
				{
					if( pPtr != NULL )
						*pPtr = pName;

					// READING PLYR OBJ.
					DWORD retVal = 0;
					retVal = m->readPointer( dObject, 3 );

					return m->read<int>( retVal + ( i * 0x34 ) );
				}

			}
		}

		return NULL;
	}

	Ammunation* getAmmunation( int nAmmo, Memory *m )
	{
		size_t arraySize = m->read<int>( dWeap + 0xC4 );

		for( size_t i = nAmmo; i < arraySize; i++ )
		{
			DWORD wPtr = 0;
			DWORD offset[ 1 ] = { i * 0x4 };

			wPtr = m->readPointer<DWORD>( dWeap + 0xC0, offset, 1 );

			Ammunation* a = new Ammunation( wPtr, m );

			if( !a->name.empty() )
			{
				if( !a->type.empty() )
				{
					if( a->type.compare( "Magazine" ) )
					{
						return a;
					} else {
						return NULL;
					}
				}
			}

		}

		return NULL;
	}

	Weapon* getWeapon( int nWeap, Memory* m )
	{
		size_t arraySize = m->read<int>( dWeap + 0x4 );

		for( size_t i = nWeap; i < arraySize; i++ )
		{
			DWORD wPtr = 0;
			DWORD offset[ 1 ] = { i * 0x4 };

			wPtr = m->readPointer<DWORD>( dWeap, offset, 1 );

			Weapon* w = new Weapon( wPtr, m );

			if( !w->name.empty() )
			{
				if( !w->type.empty() )
				{
					if( w->type.compare( "Rifles" ) )
					{
						return w;
					} else {
						return NULL;
					}
				}
			}

		}

		return NULL;
	}

	float Distance3D( D3DXVECTOR3 v1, D3DXVECTOR3 v2 )
	{
		D3DXVECTOR3 out( 0, 0, 0 );
		out = v1 - v2;

		return D3DXVec3Length( &out );
	}

	const D3DXVECTOR3 WorldToScreen( D3DXVECTOR3 curPos )
	{
		D3DXVECTOR3 temp( 0, 0, 0 );
		D3DXVec3Subtract( &temp, &curPos, &InvViewTranslation );

		float x = 0, y = 0, z = 0;

		z = D3DXVec3Dot( &temp, &InvViewForward );
		y = D3DXVec3Dot( &temp, &InvViewUp );
		x = D3DXVec3Dot( &temp, &InvViewRight );

		temp.y = 2 * ( ViewPortMatrix.y * ( 1 - ( y / Projection2.y / z ) ) ) / 2;
		temp.x = 2 * ( ViewPortMatrix.x * ( 1 + ( x / Projection1.x / z ) ) ) / 2;
		temp.z = z;

		return temp;
	}

	int drawPlayer( Player* p, int value )
	{
		if( p->entity == localPlayer )
			return 0;

		D3DXVECTOR3 plyrPos = p->getPos();
		D3DXVECTOR3 screenCords( 0, 0, 0 );

		screenCords = WorldToScreen( plyrPos );
		if( screenCords.z <= 0.01 )
			return 0;

		float distance = Distance3D( plyrPos, p->getLocalPos() );
		if( distance > value )
			return 0;

		cout << Projection2.y << endl;
		float drawY = ( 1050.0f / distance ) / Projection2.y;
		float drawX = ( 800.0f / distance ) / Projection1.x;

		float screenX = screenCords.x - ( drawX / 2 );
		float screenY = screenCords.y - drawY;

		std::string sDistance = to_string( (int) distance );
		std::string sName = p->getName() + " [" + sDistance + "m]";

		if( distance > 700 ) distance = 700;
		if( distance < 5 ) distance = 15;

		if( !p->isInCar() && p->isAlive() )
			d3d9.drawBox( (int)( screenX - ( distance / 100 ) ), (int)( screenY - ( distance / 90 ) ), (int)( drawX - ( distance / 100 ) ), (int)(drawY - ( distance / 90 )), D3DCOLOR_ARGB( 255, 17, 59, 245 ) ); // BOX

		DWORD color;
		if( p->isAlive() )
			color = D3DCOLOR_ARGB( 255, 255, 0, 0 );
		else
			color = D3DCOLOR_ARGB( 255, 0, 0, 255 );

		if( p->isInCar() )
		{
			std::string carType = p->getCar();
			d3d9.drawTextEx( carType.c_str( ), (int) ( screenCords.x - ( distance / 100 ) - carType.length( ) ), (int)(screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) + 12 )), color, font );
			if( carType.length() > 0 )
				d3d9.drawTextEx( carType.c_str( ), (int) ( screenCords.x - ( distance / 100 ) - carType.length( ) ), (int)(screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) + 12 )), color, font );
			color = D3DCOLOR_ARGB( 255, 0, 255, 0 );
		}

		if( sName.length() > 0 )
		{
			d3d9.drawTextEx( sName.c_str( ), (int) ( screenCords.x - ( distance / 100.0f ) - sName.length( ) ), (int)(screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) )), color, font ); // NAME
		}

		return 0;
	}

	int drawCar( Player* p, int value )
	{

		D3DXVECTOR3 plyrPos = p->getPos();
		D3DXVECTOR3 screenCords( 0, 0, 0 );


		screenCords = WorldToScreen( plyrPos );

		if( screenCords.z <= 0.01 )
			return 0;

		float distance = Distance3D( plyrPos, p->getLocalPos() );
		if( distance > value )
			return 0;


		float drawY = ( 1050.0f / distance ) / Projection2.y;
		float drawX = ( 800.0f / distance ) / Projection1.x;

		float screenX = screenCords.x - ( drawX / 2 );
		float screenY = screenCords.y - drawY;


		std::string sDistance = to_string( (int) distance );
		std::string sName = p->getCarName() + " [" + sDistance + "m]";

		if( distance > 700 ) distance = 700;
		if( distance < 5 ) distance = 15;

		DWORD color = D3DCOLOR_ARGB( 255, 255, 0, 0 );
		if( sName.length() > 0 )
		{
			d3d9.drawTextEx( sName.c_str( ), (int) ( screenCords.x - ( distance / 100.0f ) - sName.length( ) ), (int)(screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) )), color, font ); // NAME
		}

		return 0;
	}

	int renderXRAY( LPVOID args )
	{
		D3D9_item* item			= (D3D9_item*)args;
		Memory* m				= (Memory*)item->arguments;
		Player p( m );

		DWORD entitySize = 0;
		entitySize = m->readPointer( dObjectSize, 3 );

		p.refresh();
		for( size_t i = 0; i < entitySize; i++ )
		{
			p.setPlayer( i );

			if( displayCars && p.isInCar() )
			{
				drawPlayer( &p, item->value );
				continue;
			} else if( displayPlayers && p.isPlayer() && !p.isInCar() )
			{
				drawPlayer( &p, item->value );
				continue;
			} else if( displayEmptyCars && !p.isPlayer() && p.isCar( ) )
			{
				drawCar( &p, item->value );
				continue;
			}
			

		}
		

		return 0;
	}



	int unlockCar( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		if( m->read<int>( localPlayer + 0xAC8 ) == 0 )
		{
			m->write<INT>( localPlayer + 0xA98, 1 );
			item->isEnabled = false;
			return 0;
		}


		DWORD entitySize = 0;
		DWORD offsets[ 2 ] = { 0x8, 0x24 };

		D3DXVECTOR3 curPos( 0, 0, 0 );
		D3DXVECTOR3 carPos( 0, 0, 0 );

		curPos = m->readPointer<D3DXVECTOR3>( dObj, offsets, 2 );
		entitySize = m->readPointer( dObjectSize, 3 );

		p.refresh();
		for( size_t i = 0; i < entitySize; i++ )
		{
			p.setPlayer( i );

			if( p.isCar() )
			{
				carPos = p.getPos();
				float distance = Distance3D( carPos, curPos );

				if( distance < 15 )
				{
					m->write<INT>( p.getBase() + 0xA98, 1 );
				}
			}
		}

		item->isEnabled = false;

		return 0;
	}

	int teleport( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;

		Player p( m );

		while( item->isEnabled )
		{
			if( GetAsyncKeyState( 0x57 ) )
			{
				p.refresh();

				float dirX = 1.0f, dirY = 1.0f;
				
				DWORD offset[1] = {0x1C};

				try{
					dirX = m->readPointer<float>( localPlayer + 0x18, offset, 1 );

					offset[ 0 ] = 0x24;
					dirY = m->readPointer<float>( localPlayer + 0x18, offset, 1 );
				} catch( ... ){
					item->isEnabled = false;
					continue;;
				}


				D3DXVECTOR3 plyrPos = p.getLocalPos();

				plyrPos.y = 0;
				plyrPos.x = plyrPos.x + ( dirX * item->value );
				plyrPos.z = plyrPos.z + ( dirY * item->value );

				DWORD dPlayer = m->read<int>( localPlayer + 0x18 );
				m->write<float>( dPlayer + 0x28, plyrPos.x );
				m->write<float>( dPlayer + 0x2C, plyrPos.y );
				m->write<float>( dPlayer + 0x30, plyrPos.z );

			} else if( GetAsyncKeyState( 0x53 ) )
			{
				p.refresh();

				float dirX = 1.0f, dirY = 1.0f;

				DWORD offset[ 1 ] = { 0x1C };
				try{
					dirX = m->readPointer<float>( localPlayer + 0x18, offset, 1 );

					offset[ 0 ] = 0x24;
					dirY = m->readPointer<float>( localPlayer + 0x18, offset, 1 );
				} catch( ... ){
					item->isEnabled = false;
					continue;;
				}

				D3DXVECTOR3 plyrPos = p.getLocalPos();

				plyrPos.y = 0;
				plyrPos.x = plyrPos.x - ( dirX * item->value );
				plyrPos.z = plyrPos.z - ( dirY * item->value );

				DWORD dPlayer = m->read<int>( localPlayer + 0x18 );
				m->write<float>( dPlayer + 0x28, plyrPos.x );
				m->write<float>( dPlayer + 0x2C, plyrPos.y );
				m->write<float>( dPlayer + 0x30, plyrPos.z );
			}


			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

		}


		return 0;
	}

	int setIndirect( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;

		Player p( m );

		p.refresh();

		DWORD wID = m->read<int>( localPlayer + 0x6E0 ), wPtr = 0;
		DWORD offsets[ 3 ] = { ( wID * 0x24 + 0x4 ), 0x8, 0x200 };

		wPtr = m->readPointer<DWORD>( localPlayer + 0x694, offsets, 3 );

		if( !item->isEnabled ) { m->write<float>( m->read<int>( wPtr + 0x144 ), 0 ); return 0; }

		m->write<float>( wPtr + 0x144, (float) item->value );

		return 0;
	}

	int setRange( LPVOID args )
	{

		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		p.refresh();

		DWORD wID = m->read<int>( localPlayer + 0x6E0 ), wPtr = 0;
		DWORD offsets[ 3 ] = { ( wID * 0x24 + 0x4 ), 0x8, 0x200 };

		wPtr = m->readPointer<DWORD>( localPlayer + 0x694, offsets, 3 );

		if( !item->isEnabled ) { m->write<float>( m->read<int>( wPtr + 0x148 ), 0 ); return 0; }

		m->write<float>( wPtr + 0x148, (float) item->value );

		return 0;
	}

	int framePlayer( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		while( item->isEnabled )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

			DWORD pBase = getPlayerByName( item->itemVal, m );
			if( pBase == NULL )
			{
				DWORD wID = m->read<int>( localPlayer + 0x6E0 ), wPtr = 0;
				DWORD offsets[ 3 ] = { ( wID * 0x24 + 0x4 ), 0x8, 0x200 };

				wPtr = m->readPointer<DWORD>( localPlayer + 0x694, offsets, 3 );

				m->write<float>( m->read<int>( wPtr + 0x144 ), 0 );
				m->write<float>( m->read<int>( wPtr + 0x148 ), 0 );

				item->itemVal = "NULL";
				item->isEnabled = false;

				break;
			}

			DWORD tablePtr = 0;
			DWORD offset[1] = {0x7DC};
			tablePtr = m->readPointer<DWORD>( dObj, offset, 1 );

			int tableSize = 0;
			offset[ 0 ] = 0x7E0;

			tableSize = m->readPointer<DWORD>( dObj, offset, 1 );

			for( int i = 0; i < tableSize; i++ )
			{
				DWORD ammoPtr = m->read<int>( tablePtr + ( i * 0x4 ) );
				DWORD ammoStats = ammoPtr + 0x1FC;

				DWORD ammo = 0;
				DWORD offset[ 1 ] = { 0x13A8 };

				ammo = m->readPointer<DWORD>( dObj, offset, 1 );

				if( ammo == m->read<int>( ammoStats ) )
				{

					p.refresh();


					D3DXVECTOR3 plyrPos = p.getLocalPos();
					D3DXVECTOR3 bltPos( 0, 0, 0 );

					bltPos.x = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x28 );
					bltPos.y = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x2C );
					bltPos.z = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x30 );

					double distance = sqrt( ( plyrPos.x * bltPos.x + plyrPos.y * bltPos.y + plyrPos.z * bltPos.z ) );

					if( distance > 1.8 )
					{
						int reference = m->read<int>( pBase );

						m->write<INT>( ammoStats, pBase );
						m->write<INT>( pBase, ( reference + 1 ) );
					}
				}
			}
		}

		return 0;
	}

	int killPlayer( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		DWORD pBaseOrig = getPlayerByName( item->itemVal, m );
	
		while( item->isEnabled )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

			DWORD pBase = getPlayerByName( item->itemVal, m );
			if( pBase == NULL || pBaseOrig != pBase )
			{
				pBaseOrig = getPlayerByName( item->itemVal, m );
				pBase = pBaseOrig;

				if( !pBaseOrig )
				{
					DWORD wID = m->read<int>( localPlayer + 0x6E0 ), wPtr = 0;
					DWORD offsets[ 3 ] = { ( wID * 0x24 + 0x4 ), 0x8, 0x200 };

					wPtr = m->readPointer<DWORD>( localPlayer + 0x694, offsets, 3 );

					m->write<float>( m->read<int>( wPtr + 0x144 ), 0 );
					m->write<float>( m->read<int>( wPtr + 0x148 ), 0 );

					item->itemVal = "NULL";
					item->isEnabled = false;

					break;
				}
			}

			DWORD tablePtr = 0;
			DWORD offset[1] = {0x7DC};

			tablePtr = m->readPointer<DWORD>( dObj, offset, 1 );

			int tableSize = 0;
			offset[0] = 0x7E0;

			tableSize = m->readPointer<int>( dObj, offset, 1 );

			for( int i = 0; i < tableSize; i++ )
			{
				DWORD ammoPtr = m->read<int>( tablePtr + ( i * 0x4 ) );
				DWORD ammoStats = ammoPtr + 0x1FC;

				DWORD ammo = 0;
				offset[0] = 0x13A8;
				ammo = m->readPointer<DWORD>( dObj, offset, 1 );

				if( ammo == m->read<int>( ammoStats ) )
				{

					p.refresh();

					D3DXVECTOR3 plyrPos = p.getLocalPos();
					D3DXVECTOR3 bltPos( 0, 0, 0 );

					bltPos.x = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x28 );
					bltPos.y = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x2C );
					bltPos.z = m->read<float>( m->read<int>( ammoPtr + 0x18 ) + 0x30 );

					double distance = sqrt( ( plyrPos.x * bltPos.x + plyrPos.y * bltPos.y + plyrPos.z * bltPos.z ) );

					if( distance > 1.4 )
					{
						int reference = m->read<int>( pBase );

						m->write<INT>( ammoStats, pBase );
						m->write<INT>( pBase, ( reference + 1 ) );

						D3DXVECTOR3 entityPos( 0, 0, 0 );
						DWORD offsets[2] = { 0x18, 0x28 };

						entityPos = m->readPointer<D3DXVECTOR3>( pBase + 0x4, offsets, 2 );

						m->write<float>( m->read<int>( ammoPtr + 0x18 ) + 0x28, entityPos.x );
						m->write<float>( m->read<int>( ammoPtr + 0x18 ) + 0x2C, entityPos.y );
						m->write<float>( m->read<int>( ammoPtr + 0x18 ) + 0x30, entityPos.z );
					}
				}
			}
		}


		return 0;
	}

	int setPlayer( LPVOID args )
	{
		D3D9_textBox* textBox = (D3D9_textBox*) args;
		D3D9_item* item = (D3D9_item*) textBox->item;
		Memory* m = (Memory*) item->arguments;

		if( item->isEnabled )
			return 0;

		if( textBox->side == "LEFT" )
		{
			if( textBox->step > 0 )
				textBox->step--;
		} else if( textBox->side == "RIGHT" )
			textBox->step++;

		string pName = "";
		DWORD pBase = getPlayerByNumber( textBox->step, m, &pName );

		if( pBase == NULL )
		{
			textBox->step = 0;

			item->isEnabled = false;
			item->itemVal = "NULL";

			return 0;
		}

		item->itemVal = pName;

		return 0;
	}

	int fixLegs( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		p.refresh();

		DWORD legPtr = m->read<int>( localPlayer + 0xC0 );
		legPtr += 0xC;

		m->write<float>( legPtr, 0.0f );

		item->isEnabled = false;
		return 0;
	}

	int setSpawnWeapon( LPVOID args )
	{
		D3D9_textBox* textBox = (D3D9_textBox*) args;
		D3D9_item* item = (D3D9_item*) textBox->item;
		Memory* m = (Memory*) item->arguments;

		if( item->isEnabled )
			return 0;

		if( textBox->side == "LEFT" )
		{
			if( textBox->step > 0 )
				textBox->step--;
		} else if( textBox->side == "RIGHT" )
			textBox->step++;

		Weapon* w = getWeapon( textBox->step, m );
		if( w != NULL )
		{
			item->value = textBox->step;
			item->itemVal = w->name;
		} else {
			item->itemVal = "NULL";
			textBox->step = 0;
		}

		return 0;
	}

	int setSpawnAmmunation( LPVOID args )
	{
		D3D9_textBox* textBox = (D3D9_textBox*) args;
		D3D9_item* item = (D3D9_item*) textBox->item;
		Memory* m = (Memory*) item->arguments;

		if( item->isEnabled )
			return 0;

		if( textBox->side == "LEFT" )
		{
			if( textBox->step > 0 )
				textBox->step--;
		} else if( textBox->side == "RIGHT" )
			textBox->step++;

		Ammunation* w = getAmmunation( textBox->step, m );
		if( w != NULL )
		{
			item->value = textBox->step;
			item->itemVal = w->name;
		} else {
			item->itemVal = "NULL";
			textBox->step = 0;
		}

		return 0;
	}

	int spawnWeapon( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		Weapon* w = getWeapon( item->value, m );
		if( w != NULL )
		{
			p.refresh();

			DWORD itemPtr = 0;
			int itemSize = 0;
			int itemMaxSize = 0;

			itemPtr = m->read<int>( localPlayer + 0x688 );
			itemSize = m->read<int>( localPlayer + 0x68C );
			itemMaxSize = m->read<int>( localPlayer + 0x690 );

			if( itemSize < itemMaxSize )
			{
				int distance = itemPtr + itemSize * 0x4;

				m->write<INT>( distance, w->dAddress );
				m->write<INT>( localPlayer + 0x68C, itemSize + 1 );
			}
		}

		item->isEnabled = false;

		return 0;
	}

	int spawnAmmunation( LPVOID args )
	{
		D3D9_item* item = (D3D9_item*) args;
		Memory* m = (Memory*) item->arguments;
		Player p( m );

		Ammunation* ammo = getAmmunation( item->value, m );
		if( ammo != NULL )
		{
			p.refresh();

			DWORD itemPtr = 0;
			int itemSize = 0;
			int itemMaxSize = 0;

			itemPtr = m->read<int>( localPlayer + 0x6A0 );
			itemSize = m->read<int>( localPlayer + 0x6A4 );
			itemMaxSize = m->read<int>( localPlayer + 0x6A8 );

			if( itemSize < itemMaxSize )
			{
				DWORD memory = NULL;//m->allocateMemory( NULL, 0x2C, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
				//m->write( memory, 0, 0x2C );

				int item1 = m->read<int>( m->read<int>( itemPtr + 40 ) );
				int item2 = m->read<int>( m->read<int>( itemPtr + 4 ) );

				m->write<INT>( memory, m->read<int>( itemPtr ) );
				m->write<INT>( memory + 4, item2 );
				m->write<INT>( memory + 8, ammo->dAddress );

				m->write<INT>( memory + 40, item1 );
				m->write<INT>( memory + 0x2C, itemRef );
				itemRef++;

				/* UNCHECKED */
				int ammoXOR = m->read<int>( ammo->dAddress + 0x2C ) ^ (int) ( 0xBABAC8B6L );
				int ammoVal = ammoXOR << 1;

				m->write<INT>( memory + 0xC, int( ammoXOR - ammoVal ) );
				m->write<INT>( memory + 0x24, ammoVal );
				/* END UNCHECKED */

				int distance = itemPtr + ( itemSize * 0x4 );

				m->write<INT>( distance, memory );
				m->write<INT>( localPlayer + 0x6A4, itemSize + 1 );

			}
		}

		item->isEnabled = false;

		return 0;
	}

#endif