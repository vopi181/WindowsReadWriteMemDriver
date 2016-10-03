#ifndef HEADER_ENTITY
#define HEADER_ENTITY

#include "Memory.h"
#include <iostream>

#include <d3dx9.h>
#pragma comment(lib,"d3dx9.lib")

namespace
{
	DWORD dObj			= 0xDAD8C0;
	DWORD dScoreboard	= 0xD9F5C0;
	DWORD dWeap			= 0xDA8130;
	DWORD transPtr		= 0xDD8A14;

	DWORD dLocalPlayerOffsets[ 3 ]	= { dObj, 0x13A8, 0x4 };
	DWORD dObjectSize[ 3 ]			= { dObj, 0x5FC, 0x4 };
	DWORD dObject[ 3 ]				= { dObj, 0x5FC, 0x0 };
	DWORD dScoreboardSize[ 3 ]		= { dScoreboard + 0x24, 0x1C };
	DWORD dScoreboardObject[ 3 ]	= { dScoreboard + 0x24, 0x18 };

	DWORD transOffsets[ 2 ] = { transPtr, 0x90 };

	DWORD localPlayer	= NULL;
	DWORD transData		= NULL;
	DWORD dObjPtr		= NULL;

	D3DXVECTOR3 InvViewRight			= D3DXVECTOR3( 0, 0, 0 );
	D3DXVECTOR3 InvViewForward			= D3DXVECTOR3( 0, 0, 0 );
	D3DXVECTOR3 InvViewTranslation		= D3DXVECTOR3( 0, 0, 0 );
	D3DXVECTOR3 InvViewUp				= D3DXVECTOR3( 0, 0, 0 );

	D3DXVECTOR3 Projection1				= D3DXVECTOR3( 0, 0, 0 );
	D3DXVECTOR3 ViewPortMatrix			= D3DXVECTOR3( 0, 0, 0 );
	D3DXVECTOR3 Projection2				= D3DXVECTOR3( 0, 0, 0 );

}


class Weapon
{
public:
	DWORD dAddress;
	string type;
	string name;

	Weapon( int dAddress, Memory* m ) 
	{ 
		this->dAddress = dAddress; 
		this->name = m->readString( m->read<int>( dAddress + 0x18 ) + 0x8, 80 );
		this->type = m->readString( m->read<int>( dAddress + 0x1C ) + 0x8, 80 );
	}

};

class Ammunation
{
public:
	DWORD dAddress;
	string type;
	string name;

	Ammunation( int dAddress, Memory* m )
	{
		this->dAddress = dAddress;
		this->name = m->readString( m->read<int>( dAddress + 0x14 ) + 0x8, 80 );//m->read<int>( m->read<int>( dAddress + 0x14 ) + 0x4 ) );
		this->type = m->readString( m->read<int>( dAddress + 0x20 ) + 0x8, 80 );//m->read<int>( m->read<int>( dAddress + 0x20 ) + 0x4 ) );
	}

};

class Player
{
public:
	Player( Memory* m )
	{
		this->m = m;

		try{
			localPlayer				= m->readPointer( dLocalPlayerOffsets, 3 );
			transData				= m->readPointer( transOffsets, 2 );
			dObjPtr					= m->readPointer( dObject, 3 );


			InvViewRight			= m->read<D3DXVECTOR3>( transData + 0x4 );
			InvViewUp				= m->read<D3DXVECTOR3>( transData + 0x10 );
			InvViewForward			= m->read<D3DXVECTOR3>( transData + 0x1C );
			InvViewTranslation		= m->read<D3DXVECTOR3>( transData + 0x28 );

			ViewPortMatrix			= m->read<D3DXVECTOR3>( transData + 0x54 );
			Projection1				= m->read<D3DXVECTOR3>( transData + 0xCC );
			Projection2				= m->read<D3DXVECTOR3>( transData + 0xD8 );
		} catch( ... )
		{

		}
	};

	DWORD entity = NULL;

	std::string getName( );
	std::string getType( );
	std::string getCar( );
	std::string getCarName();

	std::string getWeapon( );
	std::string getWeaponType( );

	D3DXVECTOR3 getPos( );
	D3DXVECTOR3 getLocalPos( );

	DWORD		getBase() { return entity; };

	int getID( );

	bool isPlayer( );
	bool isAlive( );
	bool isInCar( );
	bool isCar( );

	void setPlayer( int playerID );
	void refresh( );
private:
	Memory* m;
};

#endif