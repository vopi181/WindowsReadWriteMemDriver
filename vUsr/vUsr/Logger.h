#ifndef HEADER_LOGGER
#define HEADER_LOGGER

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <vector>
#include <iostream>
#include <ctime>
#include "stdafx.h"
using namespace std;

typedef void( *inputHandler )( char* input );



class Logger
{
public:
	Logger()
	{
		if (!AllocConsole())
			return;

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);

		cInfo.dwSize.Y = 500;

		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);

		FILE* pFile;
		freopen_s(&pFile, "CONOUT$", "w", stdout);
		freopen_s(&pFile, "CONIN$", "r", stdin);

		ios::sync_with_stdio();

			cout << "Exploder's Console - ALL RIGHTS RESERVED" << endl;
	}

	
	
	void registerInput( LPVOID func )
	{
		if( func )
		{
			registedHandlers.push_back( func );
		}
	}
	void handleInput( )
	{
		std::thread t1( &Logger::listener, this );
		t1.detach();
	}

private:
	void listener()
	{
		char input[ 1000 ];

		while( true )
		{
			memset( input, 0, sizeof( input ) ); // clearing buffer.

			cout << "\n>";

			cin.getline( input, sizeof( input ) );

			for( LPVOID hanlder : registedHandlers )
			{
				inputHandler handle = (inputHandler) hanlder;
				handle( input );
			}

		}
	}

	vector<LPVOID> registedHandlers;
};

class FileLogger{

public:
	FileLogger()
	{ 
		file.open( "log.txt", ios::app );
	}
	~FileLogger(){
		file.close();
	}

	void Log( string text ){
		if( file.is_open() ){

			time_t t			= time( 0 );
			char buffer[ 90 ]	= { 0 };

			struct tm curTime; 
			localtime_s( &curTime, &t );


			strftime( buffer, sizeof( buffer ), "[%d %b][%X]: ", &curTime );

			file << buffer << text.c_str() << endl;
		}
	}
	void clearLog(){
		if( file.is_open() ){
			file.close();
			file.open( "log.txt" );
		}
	}

private:
	ofstream file;
};
#endif