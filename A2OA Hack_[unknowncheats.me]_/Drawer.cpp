#include "Drawer.h"

using namespace A2OA;

void render( LPVOID args )
{
	D3D9_item*		item	= (D3D9_item*)args;
	Memory*			m		= (Memory*)item->arguments;

	auto			world		= World::Singleton();

	if( !world->getCameraOn()->getUnit()->getBase() )
		return;

	auto			locPlayer	= world->getCameraOn()->getUnit()->getPlayer();
	if( !locPlayer->getBase() )
		return;

	auto			transData	= TransData::Singleton();

	auto			entityTablePtr		= world->getEntityTable();
	auto			entityTable			= entityTablePtr->getTable();
	auto			entityTableSize		= entityTablePtr->getTableSize();

	auto			networkMgr			= NetworkManager::Singleton();
	auto			scoreboard			= networkMgr->getScoreboard();
	auto			scoreboardSize		= scoreboard->getTableSize();
	auto			scoreboardTable		= scoreboard->getScoreboardTable();

	string			plyrName			= "";
	
	transData->refresh();
	for( DWORD i = 0; i < entityTableSize; i++ ){
		auto UnitInfo = entityTable->getUnitInfoById( i );
		auto Unit	  = UnitInfo->getUnit();
		auto UnitBase = Unit->getBase();

		if( UnitBase ){
			if( Unit->getID() > 1 && displayPlayers ){ // PLAYER
				auto Entity		= Unit->getPlayer();

				if( Entity->getBase() == locPlayer->getBase() )
					continue;

				if( Unit->getID( ) == 1 ){
					plyrName = "AI";
				} else {
					for( INT l = 0; l < scoreboardSize; l++ ){
						auto scoreboardEntity = scoreboardTable->getEntryById( l );

						if( scoreboardEntity->getID() == Unit->getID() ){
							plyrName = scoreboardEntity->getString()->getString();
						}
					}
				}

				D3DXVECTOR3 screenCords = transData->WorldToScreen( Entity->getPos() );
				if( screenCords.z <= 0.01 )
					continue;

				float distance	= transData->Distance3D( Entity->getPos(), locPlayer->getPos() );
				if( distance > item->value )
					continue;

				plyrName += " [" + to_string( (int) distance ) + "m]";

				if( distance > 700 ) distance = 700;
				if( distance < 5 ) distance = 15;

				float drawY		= ( 1050.0f  / distance ) / transData->Projection2.y;
				float drawX		= ( 800.0f  /  distance ) / transData->Projection1.x;

				float screenX = screenCords.x - ( drawX / 2 );
				float screenY = screenCords.y - drawY;
				
				d3d9.drawBox( (int)( screenX - ( distance / 100 ) ), (int)( screenY - ( distance / 90 ) ), (int)( drawX - ( distance / 100 ) ), (int)( drawY - ( distance / 90 ) ), D3DCOLOR_ARGB( 255, 17, 59, 245 ) ); // BOX
				d3d9.drawTextEx( plyrName.c_str( ), (int)(screenCords.x - ( distance / 100.0f ) - plyrName.length( ) ), (int)( screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) ) ), D3DCOLOR_RGBA(255,0,0,255), font );

			} else if( Unit->getID() == 0 ){ // VEHICLE

				auto VehicleInfo	= UnitInfo->getVehicle();
				auto Entity			= VehicleInfo->getDriver();
				auto Vehicle		= VehicleInfo->getVehicle();

				if( !Entity->getBase() && displayEmptyCars ){
					plyrName = VehicleInfo->getEntityName()->getString();

					D3DXVECTOR3 screenCords = transData->WorldToScreen( Vehicle->getPos( ) );
					if( screenCords.z <= 0.01 )
						continue;

					float distance = transData->Distance3D( Vehicle->getPos( ), locPlayer->getPos( ) );
					if( distance > item->value )
						continue;

					plyrName += " [" + to_string( (int) distance ) + "m]";

					if( distance > 700 ){
						distance = 700;
					} else if( distance < 5 ){
						distance = 5; }

					float drawY = ( 1050.0f / distance ) / transData->Projection2.y;
					float drawX = ( 800.0f / distance ) / transData->Projection1.x;

					float screenX = screenCords.x - ( drawX / 2 );
					float screenY = screenCords.y - drawY;

					d3d9.drawTextEx( plyrName.c_str( ), (int) ( screenCords.x - ( distance / 100.0f ) - plyrName.length( ) ), (int) ( screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) ) ), D3DCOLOR_RGBA(255,0,0,255), font ); // NAME
				} else if( Entity->getBase() && displayCars ){

					if( VehicleInfo->getBase() == locPlayer->getBase() )
						continue;

					if( Entity->getID() == 1 ){
						plyrName = "AI";
					} else {
						for( INT l = 0; l < scoreboardSize; l++ ){
							auto scoreboardEntity = scoreboardTable->getEntryById( l );

							if( scoreboardEntity->getID() == Entity->getID() ){
								plyrName = scoreboardEntity->getString()->getString();
							}
						}
					}
						

					D3DXVECTOR3 screenCords = transData->WorldToScreen( Vehicle->getPos() );
					if( screenCords.z <= 0.01 )
						continue;

					float distance = transData->Distance3D( Vehicle->getPos(), locPlayer->getPos() );
					if( distance > item->value )
						continue;

					plyrName += " [" + to_string( (int)distance ) + "m]";
					

					if( distance > 700 ) distance = 700;
					if( distance < 5 ) distance = 15;

					float drawY = ( 1050.0f / distance ) / transData->Projection2.y;
					float drawX = ( 800.0f / distance ) / transData->Projection1.x;

					float screenX = screenCords.x - ( drawX / 2 );
					float screenY = screenCords.y - drawY;


					string carType = VehicleInfo->getEntityName()->getString();

					d3d9.drawTextEx( plyrName.c_str(), (int) ( screenCords.x - ( distance / 100.0f ) - plyrName.length() ), (int) ( screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f )) ), D3DCOLOR_RGBA( 255, 0, 0, 255 ), font );
					d3d9.drawTextEx( carType.c_str( ), (int) ( screenCords.x - ( distance / 100 ) - carType.length( ) ), (int) ( screenCords.y - ( ( drawY * 1.3f ) + ( distance / 90.0f ) + 12 ) ), D3DCOLOR_RGBA( 0, 255, 0, 255 ), font );
				}
				
			}
		}
	}

	Sleep( 10 );
}
