#include "Entity.h"

void Player::setPlayer( int playerID )
{
	entity = m->read<int>( dObjPtr + ( playerID * 0x34 ) );
	entity = m->read<int>( entity + 0x4 );
}

void Player::refresh()
{

	localPlayer = m->readPointer(dLocalPlayerOffsets, 3);
	transData = m->readPointer(transOffsets, 2);
	dObjPtr = m->readPointer(dObject, 3);

	InvViewRight = m->read<D3DXVECTOR3>(transData + 0x4);
	InvViewUp = m->read<D3DXVECTOR3>(transData + 0x10);
	InvViewForward = m->read<D3DXVECTOR3>(transData + 0x1C);
	InvViewTranslation = m->read<D3DXVECTOR3>(transData + 0x28);

	ViewPortMatrix = m->read<D3DXVECTOR3>(transData + 0x54);
	Projection1 = m->read<D3DXVECTOR3>(transData + 0xCC);
	Projection2 = m->read<D3DXVECTOR3>(transData + 0xD8);

}

D3DXVECTOR3 Player::getPos()
{
	D3DXVECTOR3 plyrPos;

	if( localPlayer + 0x18 < 0x3000 )
		return D3DXVECTOR3( 0, 0, 0 );

	plyrPos = m->read<D3DXVECTOR3>( m->read<int>( entity + 0x18 ) + 0x28 );
	return plyrPos;
}

D3DXVECTOR3 Player::getLocalPos()
{
	D3DXVECTOR3 plyrPos;

	if( localPlayer + 0x18 < 0x3000 )
		return D3DXVECTOR3( 0, 0, 0 );

	plyrPos = m->read<D3DXVECTOR3>( m->read<int>( localPlayer + 0x18 ) + 0x28 );
	return plyrPos;
}

std::string Player::getName()
{
	if( entity == NULL )
		return "Invalid Player";

	if( !this->isPlayer() )
		return "AI";

	DWORD plyr = entity;
	int plyrID = 0;

	if( this->isInCar() )
		plyr = m->read<int>( entity + 0xAB0 );

	plyrID = m->read<int>( plyr + 0xAC8 );

	DWORD scoreboardCount = 0;
	scoreboardCount = m->readPointer( dScoreboardSize, 2 );
		
	DWORD scoreboardBase = 0;
	scoreboardBase = m->readPointer( dScoreboardObject, 2 );
		
	for( SIZE_T i = 0; i < scoreboardCount; i++ )
	{
		int sPlyrID = 0;
		int scoreBase = scoreboardBase + ( i * 0x118 );

		sPlyrID = m->read<int>( scoreBase + 0x4 );

		if( sPlyrID == plyrID )
		{
			DWORD plyrNamePointer = 0;
			plyrNamePointer = m->read<int>( scoreBase + 0xA8 );

			return m->readString( plyrNamePointer + 0x8, m->read<int>( plyrNamePointer + 0x4 ) );
		}
	}

	return "AI";
}

std::string Player::getType()
{
	if( entity == NULL )
		return "Invalid Player";


}

std::string Player::getWeapon()
{
	if( entity == NULL )
		return "Invalid Player";


}

std::string Player::getWeaponType()
{
	if( entity == NULL )
		return "Invalid Player";
}

std::string Player::getCar()
{
	if( !this->isInCar() )
		return "";

	DWORD carBase = 0;
	DWORD offset[1]  = {0x30};

	carBase = m->readPointer<DWORD>( entity + 0x3C, offset, 1 );	

	return m->readString( carBase + 0x8, m->read<int>( carBase + 0x4 ) );
}

std::string Player::getCarName()
{
	DWORD carBase = 0;
	DWORD offset[ 1 ] = { 0x30 };

	carBase = m->readPointer<DWORD>( entity + 0x3C, offset, 1 );
	return m->readString( carBase + 0x8, m->read<int>( carBase + 0x4 ) );
}

bool Player::isPlayer()
{
	if( this->getID() == 0 )
	{
		if( this->isInCar() )
			return true;
	}

	if( this->getID() > 1 )
		return true;

	return false;
}

bool Player::isCar()
{
	if( this->getID() != 0 )
		return false;
	
	DWORD isInside = 0;
	isInside = m->read<int>( entity + 0xAB0 );
	if( isInside == NULL )
		return true;

	return false;
}

int Player::getID()
{
	if( entity == NULL )
		return 1;

	int isPlyr = 1;
	isPlyr = m->read<int>( entity + 0xAC8 );

	if( isPlyr == -1 )
		return 1;

	return isPlyr;
}

bool Player::isAlive()
{
	if( entity == NULL )
		return 1;

	int isAlive = 1;
	isAlive = m->read<int>( entity + 0x20C );

	if( isAlive == 1 )
		return false;

	return true;
}

bool Player::isInCar()
{
	if( entity == NULL )
		return false;

	if( this->getID() != 0 )
		return false;

	DWORD isInside = 0;
	isInside = m->read<int>( entity + 0xAB0 );

	if( isInside == NULL )
		return false;

	return true;
}