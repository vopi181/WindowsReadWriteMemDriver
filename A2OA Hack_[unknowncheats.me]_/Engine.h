#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "Memory.h"

#pragma warning( disable : 4101 ) // UNREFERENCED PARAMATER

extern Memory*		m;
extern d3Console*	console;
extern LPD3DXFONT	font;

namespace A2OA
{

	namespace{
		DWORD		transOffsets[ 2 ]	= { 0xDD8A14, 0x90 };
		DWORD		worldOffset			= 0xDAD8C0; //0xDA8208
		DWORD		scoreboardOffset	= 0xD9F5C0; //0xD99F08
		DWORD		weaponTableOffset	= 0x1D568E8;
	}

	class ArmaString
	{
		DWORD address;

	public:
		ArmaString( DWORD dwAddress ){
			address = dwAddress;
		}

		int getLength(){
			if( !address )
				return 0;
			try
			{
				return m->read<int>( address + 0x4 );
			} catch (ERROR_MEM e) {
				console->sendInput("Unit table: " + to_string(e));
				return 0;
			}
		}
		std::string getString(){
			if( !address )
				return "";
		try {
			return m->readString( address + 0x8, this->getLength() );
		} catch (ERROR_MEM e) {
			console->sendInput("Unit table: " + to_string(e));
			return "";
		}
			
		}
		std::string getStringUnformated() {
			if (!address)
				return "";
			try {
				return m->readStringUnformated(address + 0x8, this->getLength());
			} catch (ERROR_MEM e) {
				console->sendInput("Unit table: " + to_string(e));
				return "";
			}

		}
	};

	class ScoreboardEntity{
		DWORD address;
	public:
		ScoreboardEntity( DWORD dwAddress ){
			address = dwAddress;
		}
		int getID(){
			try
			{
				return m->read<DWORD>( address + 0x4 );
			} catch ( ERROR_MEM e )
			{
				return 0;
			}
			
		}
		std::unique_ptr<ArmaString> getString(){
			try
			{
				return std::unique_ptr<ArmaString>( new ArmaString( m->read<DWORD>(address + 0xA8) ) );
			} catch (ERROR_MEM e )
			{
				console->sendInput("Scoreboard table: " + to_string(e));
				return std::unique_ptr<ArmaString>(new ArmaString(0));
			}
			
		}
	};

	class ScoreboardTable{
		DWORD address;
	public:
		ScoreboardTable( DWORD dwAddress ){
			address = dwAddress;
		}
		std::unique_ptr<ScoreboardEntity> getEntryById( DWORD id ){
			try
			{
				return std::unique_ptr<ScoreboardEntity>( new ScoreboardEntity( address + (id*0x118) ));
			} catch ( ERROR_MEM e )
			{
				return std::unique_ptr<ScoreboardEntity>(new ScoreboardEntity(0));
			}
			
		}
	};

	class Scoreboard{
		DWORD address;
	public:
		Scoreboard( DWORD dwAddress ){
			address = dwAddress;
		}

		std::unique_ptr<ScoreboardTable> getScoreboardTable(){
			try
			{
				return std::unique_ptr<ScoreboardTable>( new ScoreboardTable( m->read<DWORD>( address + 0x18 )) );
			} catch ( ERROR_MEM e )
			{
				return std::unique_ptr<ScoreboardTable>(new ScoreboardTable(0));
			}
			
		}
		INT getTableSize(){
			try {
				return m->read<int>( address + 0x1C );
			} catch (ERROR_MEM e) {
				return 0;
			}
			
		}
	};

	class NetworkManager{
		DWORD address;
	private:
		static DWORD getBase(){
			return scoreboardOffset;
		}
	public:
		NetworkManager(){
			address = this->getBase();
		}
		static std::unique_ptr<NetworkManager> Singleton(){
			return std::unique_ptr<NetworkManager>( new NetworkManager() );
		}

		std::unique_ptr<Scoreboard> getScoreboard(){
			try
			{
				return std::unique_ptr<Scoreboard>( new Scoreboard( m->read<DWORD>( address + 0x24 ) ) );
			} catch (const std::exception&)
			{
				return std::unique_ptr<Scoreboard>(new Scoreboard(0));
			}
		}
	};

	class TransData
	{
	public:
		DWORD		address				= 0;
		D3DXVECTOR3 InvViewRight		= D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 InvViewForward		= D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 InvViewTranslation	= D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 InvViewUp			= D3DXVECTOR3( 0, 0, 0 );

		
		D3DXVECTOR3 ViewPortMatrix		= D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 Projection1			= D3DXVECTOR3( 0, 0, 0 );
		D3DXVECTOR3 Projection2			= D3DXVECTOR3( 0, 0, 0 );

	private:
		static DWORD getBase(){
			return m->readPointer( transOffsets, 2 );
		}
	public:
		TransData(){
			address = this->getBase();
		}

		void refresh()
		{
			try
			{
				InvViewRight = m->read<D3DXVECTOR3>(address + 0x4);
				InvViewForward = m->read<D3DXVECTOR3>(address + 0x1C);
				InvViewTranslation = m->read<D3DXVECTOR3>(address + 0x28);
				InvViewUp = m->read<D3DXVECTOR3>(address + 0x10);

				ViewPortMatrix = m->read<D3DXVECTOR3>(address + 0x54);
				Projection1 = m->read<D3DXVECTOR3>(address + 0xCC);
				Projection2 = m->read<D3DXVECTOR3>(address + 0xD8);
			} catch (ERROR_MEM e )
			{
				return;
			}

		}

		D3DXVECTOR3 WorldToScreen( D3DXVECTOR3 curPos )
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

		float Distance3D( D3DXVECTOR3 v1, D3DXVECTOR3 v2 )
		{
			D3DXVECTOR3 out( 0, 0, 0 );
			out = v1 - v2;

			return D3DXVec3Length( &out );
		}

		static std::unique_ptr<TransData> Singleton(){
			return std::unique_ptr<TransData>( new TransData( ) );
		}
			
	};

	class Entity{
		DWORD address;

	public:
		Entity( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD		getBase(){
			return address;
		}
		D3DXVECTOR3 getPos(){
			try
			{
				return m->read<D3DXVECTOR3>( address + 0x28 );
			} catch ( ERROR_MEM e )
			{
				return D3DXVECTOR3( 0,0,0 );
			}
		}
		D3DXVECTOR3 getAccerelation()
		{
			try
			{
				return m->read<D3DXVECTOR3>(address + 0x48);
			} catch (ERROR_MEM e)
			{
				return D3DXVECTOR3(0, 0, 0);
			}
		}
		float		getViewX(){
			try
			{
				return  m->read<float>( address + 0x1C );
			} catch (ERROR_MEM e )
			{
				return 0.0f;
			}
		}
		float		getViewY(){
			try
			{
				return  m->read<float>(address + 0x24);
			} catch (ERROR_MEM e)
			{
				return 0.0f;
			}
		}

		void		setPos( D3DXVECTOR3 vec ){
			m->write( address + 0x28, vec.x );
			m->write( address + 0x2C, vec.y );
			m->write( address + 0x30, vec.z );
		}
	};
	
	class Inventory{
		DWORD address;

	public:
		Inventory( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD getBase(){
			return address;
		}
		DWORD calculateDistance( SIZE_T id ){
			return address + ( id * 0x4 );
		}
	};

	class InventoryTable{
		DWORD address;

	public:
		InventoryTable( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD getTableSize(){
			return m->read<DWORD>( address + 0x68C );
		}
		DWORD getTableMaxSize(){
			return m->read<DWORD>( address + 0x690 );
		}

		DWORD getConsumableTableSize(){
			return m->read<DWORD>( address + 0x6A4 );
		}
		DWORD getConsumableTableMaxSize(){
			return m->read<DWORD>( address + 0x6A8 );
		}

		void  setTableSize( SIZE_T size ){
			m->write( address + 0x68C, size );
		}
		void  setConsumableTableSize( SIZE_T size ){
			m->write(address + 0x6A4, size);
		}
		
		std::unique_ptr<Inventory> getInventory(){
			return unique_ptr<Inventory>( new Inventory( m->read<DWORD>( address + 0x688 ) ) );
		}
		std::unique_ptr<Inventory> getConsumableInventory(){
			return unique_ptr<Inventory>( new Inventory( m->read<DWORD>( address + 0x6A0 ) ) );
		}
	};

	class Unit
	{
		DWORD address;

	public:
		Unit( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD	getBase(){
			return address;
		}
		BYTE	isAlive(){
			try {
				return m->read<BYTE>( address + 0x20C );
			} catch (ERROR_MEM e) {
				return 0;
			}
		}
		int		getID(){
			try {
				return m->read<DWORD>(address + 0xAC8);
			} catch (ERROR_MEM e) {
				return 0;
			}
		}
		INT		getWeaponID() {
			return m->read<DWORD>( address + 0x6E0 );
		}

		std::unique_ptr<InventoryTable> getInventoryTable( ){
			return std::unique_ptr<InventoryTable>( new InventoryTable( address ) );
		}
		std::unique_ptr<Entity> getPlayer(){
			try
			{
				return std::unique_ptr<Entity>( new Entity( m->read<DWORD>( address + 0x18 ) ) );
			} catch (ERROR_MEM e)
			{
				return std::unique_ptr<Entity>(new Entity(0));
			}
			
		}
	};

	class Vehicle{
		DWORD address;
	public:
		Vehicle( DWORD dwAddress ){
			address = dwAddress;
		}

		D3DXVECTOR3		getPos(){
			try
			{
				return m->read<D3DXVECTOR3>( address + 0x28 );
			} catch (ERROR_MEM e)
			{
				return D3DXVECTOR3(0,0,0);
			}
		}
		D3DXVECTOR3 getAccerelation()
		{
			try
			{
				return m->read<D3DXVECTOR3>(address + 0x48);
			} catch (ERROR_MEM e)
			{
				return D3DXVECTOR3(0, 0, 0);
			}
		}

	};

	class VehicleInfo{
		DWORD address;
	public:
		VehicleInfo( DWORD dwAddress ){
			address = dwAddress;
		}

		void			unlock(){
			m->write( address + 0xA98, 1 );
		}
		DWORD			getBase(){
			return address;
		}

		std::unique_ptr<ArmaString>	getEntityName(){
			try
			{
				DWORD base = m->read<DWORD>( m->read<DWORD>( address + 0x3C ) + 0x30 );
				return std::unique_ptr<ArmaString>( new ArmaString(base) );
			} catch (ERROR_MEM e)
			{
				return std::unique_ptr<ArmaString>(new ArmaString(0));
			}
		}

		std::unique_ptr<Unit> getDriver(){
			try {
				return std::unique_ptr<Unit>( new Unit( m->read<DWORD>( address + 0xAB0 ) ) );
			} catch (ERROR_MEM e) {
				console->sendInput("Unit table: " + to_string(e));
				return std::unique_ptr<Unit>(new Unit(0));
			}
			
		}
		std::unique_ptr<Vehicle> getVehicle( ){
			try
			{
				return std::unique_ptr<Vehicle>( new Vehicle( m->read<DWORD>( address + 0x18 ) ) );
			} catch (ERROR_MEM e)
			{
				return std::unique_ptr<Vehicle>(new Vehicle(0));
			}
		}
	};

	class UnitInfo{
		DWORD address;
	public:
		UnitInfo( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD			getBase(){
			return address;
		}

		std::unique_ptr<Unit> getUnit(){
			try{
				return std::unique_ptr<Unit>( new Unit( m->read<DWORD>( address + 0x4 ) ) );
			} catch( ERROR_MEM e ){
				console->sendInput( "Unit table: " + to_string(e) );
				return std::unique_ptr<Unit>( new Unit( 0 ) );
			}
			
		}
		std::unique_ptr<VehicleInfo> getVehicle( ){
			try
			{
				return std::unique_ptr<VehicleInfo>( new VehicleInfo( m->read<DWORD>( address + 0x4 ) ) );
			} catch (ERROR_MEM e)
			{
				return std::unique_ptr<VehicleInfo>(new VehicleInfo(0));
			}
		}
	};

	class EntityTable{
		DWORD address;
	public:
		EntityTable( DWORD dwAddress ){
			address = dwAddress;
		}

		std::unique_ptr<UnitInfo> getUnitInfoById( DWORD id ){
			try {
				return std::unique_ptr<UnitInfo>( new UnitInfo( m->read<DWORD>( address + (id * 0x34) ) ) );
			} catch (ERROR_MEM e) {
				console->sendInput("UnitInfo table: " + to_string(e));
				return std::unique_ptr<UnitInfo>(new UnitInfo(0));
			}
			
		}
	};

	class EntityTablePtr{
		DWORD address;
	public:
		EntityTablePtr( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD getTableSize(){
			try
			{
				return m->read<DWORD>( address + 0x4 );
			} catch (ERROR_MEM e)
			{
				return 0;
			}
		}
		std::unique_ptr<EntityTable> getTable(){
			try
			{
				return std::unique_ptr<EntityTable>( new EntityTable( m->read<DWORD>(address + 0x0) ) );
			} catch (ERROR_MEM e)
			{
				return std::unique_ptr<EntityTable>(new EntityTable(0));
			}
		}
	};

	class Ammo{
		DWORD address;

	public:
		Ammo( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD					getBase(){
			return address;
		}
		DWORD					getStats(){
			return m->read<DWORD>( address + 0x1FC );
		}
		D3DXVECTOR3				getPos(){
			return m->read<D3DXVECTOR3>( m->read<DWORD>( address + 0x18 ) + 0x28 );
		}

		void					setAcceleration(D3DXVECTOR3 vec) {
			m->write<D3DXVECTOR3>(m->read<DWORD>(address + 0x18) + 0x48, vec );
		}
		void					setPos( D3DXVECTOR3 vec ){
			m->write<D3DXVECTOR3>(m->read<DWORD>(address + 0x18) + 0x28, vec );
			/*
			m->write( m->read<DWORD>( address + 0x18 ) + 0x28, vec.x );
			m->write( m->read<DWORD>( address + 0x18 ) + 0x2C, vec.y );
			m->write( m->read<DWORD>( address + 0x18 ) + 0x30, vec.z );
			*/
		}
	};

	class Ammunition{
		DWORD address;
		
	public:
		Ammunition( DWORD dwAddress ){
			address = dwAddress;
		}

		std::unique_ptr<Ammo>	getAmmoById( DWORD i ){
			return std::unique_ptr<Ammo>( new Ammo( m->read<DWORD>( address + ( i * 0x4 ) ) ) );
		}
	};

	class Munition{
		DWORD address;

	public:
		Munition( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD getTableSize(){
			return m->read<DWORD>( address + 0x7E0 );
		}
		std::unique_ptr<Ammunition> getTable(){
			return std::unique_ptr<Ammunition>( new Ammunition( m->read<DWORD>( address + 0x7DC ) ) );
		}
	};

	class Weapon{
		DWORD address;

	public:
		Weapon( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD						getBase(){
			return address;
		}
		std::unique_ptr<ArmaString> getName(){
			return std::unique_ptr<ArmaString>( new ArmaString( m->read<DWORD>( address + 0x18 ) ) );
		}
		std::unique_ptr<ArmaString> getType(){
			return std::unique_ptr<ArmaString>( new ArmaString( m->read<DWORD>( address + 0x1C ) ) );
		}
	};

	class WeaponTable{
		DWORD address;

	private:
		static DWORD getBase(){
			return weaponTableOffset;
		}
	public:
		WeaponTable(){
			address = this->getBase();
		}

		DWORD getTableSize(){
			return m->read<DWORD>( address + 0x4 );
		}

		std::unique_ptr<Weapon> getEntryById( SIZE_T id ){
			return std::unique_ptr<Weapon>( new Weapon( m->read<DWORD>( m->read<DWORD>(address) + ( id * 0x4 ) ) ) );
		}

		static std::unique_ptr<WeaponTable> Singleton(){
			return std::unique_ptr<WeaponTable>( new WeaponTable() );
		}
	};

	class Consumable{
		DWORD address;

	public:
		Consumable( DWORD dwAddress ){
			address = dwAddress;
		}

		DWORD						getBase(){
			return address;
		}
		std::unique_ptr<ArmaString> getName(){
			return std::unique_ptr<ArmaString>( new ArmaString( m->read<DWORD>( address + 0x14 ) ) );
		}
		std::unique_ptr<ArmaString> getType(){
			return std::unique_ptr<ArmaString>( new ArmaString( m->read<DWORD>( address + 0x20 ) ) );
		}
	};

	class ConsumableTable{
		DWORD address;

	private:
		static DWORD getBase(){
			return weaponTableOffset;
		}
	public:
		ConsumableTable(){
			address = this->getBase();
		}

		DWORD getTableSize(){
			return m->read<DWORD>( address + 0xC4 );
		}

		std::unique_ptr<Consumable> getEntryById( SIZE_T id ){
			return std::unique_ptr<Consumable>( new Consumable( m->read<DWORD>( m->read<DWORD>( address + 0xC0 ) +( id * 0x4 ) ) ) );
		}
		static std::unique_ptr<ConsumableTable> Singleton( ){
			return std::unique_ptr<ConsumableTable>( new ConsumableTable( ) );
		}
	};

	class ScriptVM {
		DWORD address;
	public:
		ScriptVM(DWORD dwAddress) {
			address = dwAddress;
		}

		DWORD						getBase(){ return address; }
		std::unique_ptr<ArmaString> getScriptName() {
			try {
				return std::unique_ptr<ArmaString>(new ArmaString(m->read<DWORD>(address + 0x134)));
			} catch (ERROR_MEM e) {
				console->sendInput("UnitInfo table: " + to_string(e));
				return std::unique_ptr<ArmaString>(new ArmaString(0));
			}
		}
		std::unique_ptr<ArmaString> getScriptText() {
			try {
				return std::unique_ptr<ArmaString>(new ArmaString(m->read<DWORD>(address + 0x138)));
			} catch (ERROR_MEM e) {
				console->sendInput("UnitInfo table: " + to_string(e));
				return std::unique_ptr<ArmaString>(new ArmaString(0));
			}
		}
		std::unique_ptr<ArmaString> getScriptSource() {
			try {
				return std::unique_ptr<ArmaString>(new ArmaString(m->read<DWORD>(address + 0x140)));
			} catch (ERROR_MEM e) {
				console->sendInput("UnitInfo table: " + to_string(e));
				return std::unique_ptr<ArmaString>(new ArmaString(0));
			}
		}

		VOID						Terminate() {
			m->write<BYTE>( address + 0x28C, 1 );
		}
	};

	class ScriptTable {
		DWORD address;
	public:
		ScriptTable(DWORD dwAddress) {
			address = dwAddress;
		}
		std::unique_ptr<ScriptVM> getScriptByID(DWORD id) {
			try {
				return std::unique_ptr<ScriptVM>(new ScriptVM( m->read<DWORD>(address + (id * 0x8) )));
			} catch (ERROR_MEM e) {
				console->sendInput("Script table: " + to_string(e));
				return std::unique_ptr<ScriptVM>(new ScriptVM(0));
			}

		}
	};

	class World{
	private:
		static DWORD getBase(){
			try{
				return m->read<DWORD>( worldOffset );
			} catch( ERROR_MEM e ){
				console->sendInput( "Base: " + to_string(e) );
				return 0;
			}
		}
		DWORD address;

	public:
		World(){
			address = this->getBase();
		}

		std::unique_ptr<EntityTablePtr> getEntityTable( )
		{
			try{
				return std::unique_ptr<EntityTablePtr>( new EntityTablePtr( m->read<DWORD>( address + 0x5FC ) ) );
			} catch( ERROR_MEM e ){
				console->sendInput( "Entity table: " + to_string(e) );
				return std::unique_ptr<EntityTablePtr>( new EntityTablePtr( 0 ) );
			}
		}
		std::unique_ptr<UnitInfo>		getCameraOn(){
			try{
				return std::unique_ptr<UnitInfo>( new UnitInfo( m->read<DWORD>( address + 0x13A8 ) ) );
			} catch( ERROR_MEM e ){
				console->sendInput( "UnitInfo table: " + to_string(e) );
				return std::unique_ptr<UnitInfo>( new UnitInfo( 0 ) );
			}

		}
		std::unique_ptr<Munition>		getMunition( )
		{
			try{
				return std::unique_ptr<Munition>( new Munition( address ) );
			} catch( ERROR_MEM e ){
				console->sendInput( "Munition table: " + to_string(e) );
				return std::unique_ptr<Munition>( new Munition( 0 ) );
			}
		}
		std::unique_ptr<ScriptTable>	getScriptTable()
		{
			try {
				return std::unique_ptr<ScriptTable>(new ScriptTable( m->read<DWORD>(address + 0x718)));
			} catch (ERROR_MEM e) {
				console->sendInput("Munition table: " + to_string(e));
				return std::unique_ptr<ScriptTable>(new ScriptTable(0));
			}
		}
		DWORD							getScriptTableSize()
		{
			return m->read<DWORD>( address + 0x71C );
		}

		static std::unique_ptr<World>	Singleton(){
			return std::unique_ptr<World>( new World() );
		}
		DWORD							getWorldBase(){
			return address;
		}
		VOID							isServer(BYTE option) {
			m->write<BYTE>( address + 0x7B0, option );
		}
	};
}

#endif