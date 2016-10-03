#pragma once

// includes
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>

#include "renderManager.h"

// namespaces
using namespace std;

static HHOOK	hook;
extern bool		isOpen;
extern char*	hookCommand;

// PROTOTYPES

enum TYPES {
	T_INT, T_STRING, T_FLOAT
};

struct command {
	string text;
	DWORD color;
};

struct varCommand {
	string				command;
	TYPES				type;
	LPVOID				variable;
};

struct callback {
	string		command;
	string		description;
	SIZE_T		argCount;
	LPVOID		func;

	BOOL		hasReturn;
};

class d3Console {

public:	
	// KEYBOARD HOOK
	static LRESULT CALLBACK keyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

	d3Console(size_t posX, size_t posY, size_t sizeX, size_t sizeY);

	// GUI/INIT
	bool			initilize(rManager* rMGR, HINSTANCE hInst );
	void			render();

	// INPUT
	void			sendInput( string message );
	void			receveInput();
	void			registerCommand( string command,	LPVOID variable,	TYPES type		);
	void			registerCommand( string command,	LPVOID func,		SIZE_T argCount, string description, BOOL hasReturn );


private:
	// PROCESSING
	void			processCommand();
	void			printHints();

	bool			checkOfficialCommands( string command, vector<string> args );

	rManager*		rMGR = nullptr;

	bool			hasFocus = false;
	int				timeout;

	vector<command>	prevCommands;
	vector<command>	commandHints;

	string			currentCommand;

	// COMMAND LIST
	vector<varCommand*>		varCommands;
	vector<callback*>		callbacks;
	vector<string>			officialCommands;

	// POSITION/SIZE/ATTRIBUTES
	size_t			posX;
	size_t			posY;
	size_t			sizeX;
	size_t			sizeY;

	LPD3DXFONT		titleFont;
	LPD3DXFONT		textFont;
	LPD3DXFONT		carrotFont;
	
	DWORD			background		= D3DCOLOR_ARGB( 240,62,62,62 );
	DWORD			outterLayer		= D3DCOLOR_ARGB( 255, 40, 40, 40 );
};