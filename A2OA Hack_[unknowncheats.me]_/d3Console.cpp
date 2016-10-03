#include "d3Console.h"

bool isOpen				= false;
char *hookCommand		= new char(']');

// KEYBOARD HOOK
LRESULT d3Console::keyboardHook(int nCode, WPARAM wParam, LPARAM lParam){
	KBDLLHOOKSTRUCT* keyboard = (KBDLLHOOKSTRUCT*)lParam;

	if (isOpen && keyboard->flags != LLKHF_UP ) {
		
		*hookCommand		= MapVirtualKey( keyboard->vkCode, MAPVK_VK_TO_CHAR );
		
		if (!GetKeyState(VK_CAPITAL)){
			*hookCommand = tolower( *hookCommand );
		}
	}
	
	return CallNextHookEx( hook, nCode, wParam, lParam );
}

// CONSTRUCTOR
d3Console::d3Console( size_t posX, size_t posY, size_t sizeX, size_t sizeY )
{
	this->posX			= posX + 10;
	this->posY			= posY + 25;
	this->sizeX			= sizeX;
	this->sizeY			= sizeY;

	this->officialCommands.push_back( "help" );
	this->officialCommands.push_back( "quit" );
	this->officialCommands.push_back( "clear" );
}

bool d3Console::initilize(rManager * rMGR, HINSTANCE hInst)
{
	if ( rMGR == nullptr )
		return false;

	this->rMGR = rMGR;
	this->rMGR->createFont( "sans-serif", 16, 8, false, &this->titleFont );
	this->rMGR->createFont( "Lucida Console", 14, 8, false, &this->textFont );
	this->rMGR->createFont("Tahoma", 16, 6, false, &this->carrotFont);

	//hook = SetWindowsHookEx(WH_KEYBOARD_LL, d3Console::keyboardHook, hInst, NULL);

	return true;
	return ( hook != nullptr );
	
}

void d3Console::render()
{
	if (!this->hasFocus) return;
	
	#pragma region CONSOLE

	//BORDER
	rMGR->FillRGB( posX-14, posY-25, sizeX + 24, 15, D3DCOLOR_ARGB(255, 162, 162, 162));

	//SIDES
	rMGR->FillRGB( posX - 14, posY-10, 4, sizeY + 24, D3DCOLOR_ARGB(255, 162, 162, 162) ); //left
	rMGR->FillRGB( posX + sizeX + 10, posY-25, 4, sizeY + 39, D3DCOLOR_ARGB(255, 162, 162, 162)); //right
	rMGR->FillRGB( posX - 10, posY + sizeY + 10, sizeX+20, 4, D3DCOLOR_ARGB(255, 162, 162, 162)); //bottom

	// OUTTER LAYER

	// upper
	rMGR->FillRGB( posX - 10, posY-10, sizeX+20, 10, outterLayer ); //box
	rMGR->FillRGB( posX, posY-1, sizeX, 2, D3DCOLOR_ARGB(255, 30, 30, 30) ); //line

	// left
	rMGR->FillRGB(posX - 10, posY, 10, sizeY+10, outterLayer); //box
	rMGR->FillRGB(posX - 1, posY, 2, sizeY+10, D3DCOLOR_ARGB(255, 30, 30, 30)); //line

	// right
	rMGR->FillRGB(posX + sizeX, posY, 10, sizeY + 10, outterLayer); //box
	rMGR->FillRGB(posX + sizeX, posY, 1, sizeY+1, D3DCOLOR_ARGB(255, 181, 181, 181)); //line

	// bottom
	rMGR->FillRGB(posX - 6, posY + sizeY, sizeX + 16, 10, outterLayer); //box
	rMGR->FillRGB(posX, posY + sizeY -1 , sizeX + 1, 2, D3DCOLOR_ARGB(255, 181, 181, 181)); //line

	//CONSOLE
	rMGR->FillRGB( posX, posY, sizeX, sizeY, background );
	
	//BORDER TITLE
	rMGR->drawTextEx( "Console", (posX - 10 + sizeX / 2) - (rMGR->calculateWidth("Console",this->titleFont )/2), posY-25, D3DCOLOR_ARGB(255, 255, 255, 255), this->titleFont );
	
	//TEXT INPUT BOX
		// TOP SIDE
	rMGR->FillRGB(posX, posY+sizeY-31, sizeX,6, outterLayer);
		// TOP LAYER
	rMGR->FillRGB(posX, posY + sizeY - 31, sizeX, 1, D3DCOLOR_ARGB(255, 181, 181, 181) );
		// BOTTOM LAYER
	rMGR->FillRGB(posX, posY + sizeY - 25, sizeX, 1, D3DCOLOR_ARGB(255, 30, 30, 30));

	#pragma endregion

	// CARROT LINE
	string carrotLine = ">" + this->currentCommand;
	rMGR->drawTextEx(carrotLine.c_str(), posX + 5, (posY+sizeY) - 20, D3DCOLOR_ARGB(255, 255, 255, 255), this->carrotFont );

	// COMMAND LINE ((posY + sizeY) - 31 is the last point!!)
	size_t commandSize		= prevCommands.size(); // (posY + 37 - posY - sizeY) / -14 = commandSize
	size_t maxCommandCount	= ( (-37 + sizeY) / (14) );
	if (commandSize > maxCommandCount)
	{
		prevCommands.erase( prevCommands.begin(), prevCommands.begin() + (commandSize - maxCommandCount) );
	}

	#pragma region CONSOLE_COMMANDS
	for (auto n : prevCommands)
	{
		string command = n.text;
		rMGR->drawTextEx(command.c_str(), posX + 5, -37 + (posY + sizeY) + (-14 * commandSize), n.color, this->textFont);
		commandSize--;
	}

	SIZE_T hintSize = commandHints.size();
	if (hintSize > 0)
	{
		rMGR->FillRGB(posX + rMGR->calculateWidth(carrotLine.c_str(), this->carrotFont) + 8, posY + sizeY + 5, 140, hintSize * 17, background);
		rMGR->drawBorder(posX + rMGR->calculateWidth(carrotLine.c_str(), this->carrotFont) + 8, posY + sizeY + 5, 140, hintSize * 17, 1, D3DCOLOR_ARGB(255, 181, 181, 181), D3DCOLOR_ARGB(255, 30, 30, 30));
	}

	size_t position = 0;
	for (auto n : this->commandHints)
	{
		rMGR->drawTextEx(n.text.c_str(), posX + rMGR->calculateWidth(carrotLine.c_str(), this->carrotFont) + 13, (posY + sizeY) + 6 + (position * 17), n.color, this->textFont);
		rMGR->FillRGB(posX + rMGR->calculateWidth(carrotLine.c_str(), this->carrotFont) + 9, (posY + sizeY) + 21 + (position * 17), 138, 1, D3DCOLOR_ARGB(255, 30, 30, 30));
		position++;
	}
	#pragma endregion


}

void d3Console::sendInput(string message)
{
	if ( message.length() > 0 )
	{
		command c;

		c.text		= message;
		c.color		= D3DCOLOR_ARGB(255, 0, 200, 100);

		this->prevCommands.push_back( c );
	}
}

void d3Console::receveInput()
{
	if (GetAsyncKeyState(VK_OEM_MINUS) & 1) {
		this->hasFocus = !this->hasFocus;
		isOpen = !isOpen;
	}
	
	if( !hasFocus )
		return;

	if (GetAsyncKeyState(VK_TAB) & 1) {
		if (this->commandHints.size() > 0) {
			this->currentCommand = commandHints.at(0).text;
		}
	}

	if ( *hookCommand != ']' && *hookCommand != '`' ) {
		
		switch (*hookCommand){
			case 8: 
			{ //BACKSPACE
				if( this->currentCommand.length() > 0 )
					this->currentCommand.pop_back();

				break;
			} 
			case 13: 
			{ //ENTER
				processCommand();
				break;
			}
			default:
			{
				if( isalnum( *hookCommand ) || *hookCommand == 32 /* SPACE */ )
				this->currentCommand += *hookCommand;
				break;
			}
		}

		printHints();
		*hookCommand = ']';
	}
}

void d3Console::registerCommand(string command, LPVOID variable, TYPES type)
{
	if ( variable != nullptr && !command.empty() && variable != nullptr ) {
		varCommand* c = new varCommand;

		c->command		= command;
		c->type			= type;
		c->variable		= variable;

		varCommands.push_back( c );
	}
}

void d3Console::registerCommand(string command, LPVOID func, SIZE_T argCount, string description, BOOL hasReturn)
{
	if (!command.empty() && func != nullptr) {
		callback* c = new callback;

		c->command		= command;
		c->description	= description;
		c->argCount		= argCount;
		c->func			= func;
		c->hasReturn	= hasReturn;

		callbacks.push_back( c );
	}
}

void d3Console::processCommand()
{
	command				c1, c2;
	vector<string>		splitString;
	BOOL				isRecognized = false;

	c1.text			= "> " + this->currentCommand;
	c1.color		= D3DCOLOR_ARGB(255, 0, 200, 100);
	c2.text			= "Unrecognized command!";
	c2.color		= D3DCOLOR_ARGB(255, 138, 21, 21);

	if (this->currentCommand.length() == 0)
	{
		c1.text = ">";
		prevCommands.push_back(c1);

		return;
	}
	prevCommands.push_back( c1 );

	stringstream ss(this->currentCommand);
	string tok;

	while (getline(ss, tok, ' ')) {
		splitString.push_back(tok);
	}


	#pragma region COMMAND_LOOP

	// TURNING COMMAND TO LOWERCASE
	string com;
	try {
		for (size_t i = 0; i < splitString.at(0).length(); i++) {
			com += tolower(splitString.at(0)[i]);
		}
	}
	catch (exception e) {
		c2.text = e.what();
		c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);

		prevCommands.push_back(c2);
		this->currentCommand = "";

		return;
	}

	#pragma region VARIABLE LOOP
	if (!isRecognized) {
		for each(varCommand* c in varCommands) {
			string varCommand;
			for (size_t i = 0; i < c->command.length(); i++) {
				varCommand += tolower(c->command[i]);
			}

			if (!com.compare(varCommand)) {

				switch (c->type)
				{
					case TYPES::T_FLOAT:
					{
						try {
							float var = stof(splitString.at(1));
							*(float*)(c->variable) = var;

							c2.text = "Float variable set to " + splitString.at(1);
							c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
						} catch (exception e) {
							c2.text = e.what();
							c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);
						}

						break;
					}
					case TYPES::T_INT:
					{
						try {
							int var = stoi(splitString.at(1));
							*(int*)(c->variable) = var;

							c2.text = "INT variable set to " + splitString.at(1);
							c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
						} catch (exception e) {
							c2.text = e.what();
							c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);
						}
						break;
					}
					case TYPES::T_STRING:
					{
						strcpy_s((char*)c->variable, splitString.at(1).length() + 1, splitString.at(1).c_str());

						c2.text = "String variable set to " + splitString.at(1);
						c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
						break;
					}
					default:

						c2.text = "Unrecognized type!";
						c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);

						break;
				}

				isRecognized = true;
				break;
			}
		}
	}
	#pragma endregion
	
	#pragma region FUNCTION LOOP
	if (!isRecognized) {
		for each(callback* c in callbacks) {
			string varCommand;
			for (size_t i = 0; i < c->command.length(); i++) {
				varCommand += tolower(c->command[i]);
			}

			if (!com.compare(varCommand)) {
				if (c->argCount > 0 && splitString.size() >= c->argCount + 1) { // ARGS

					if (c->hasReturn)
					{
						typedef LPVOID(*funct)(LPVOID args);
						if (c->func != nullptr) {
							funct function = (funct)c->func;
							vector<string> argArray(++splitString.begin(), splitString.end());

							vector<string>* retValue = (vector<string>*)function(&argArray);


							if (retValue != nullptr) {
								c2.text = "";
								for each(string text in *retValue)
								{
									c2.text += text + " ";
								}

								c2.color = D3DCOLOR_ARGB(255, 0, 255, 255);
							} else {
								c2.text = "RETURN VALUE WAS NULL";
								c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
							}
						}
					} else {
						typedef void(*funct)(LPVOID args);
						if (c->func != nullptr) {
							funct function = (funct)c->func;
							vector<string> argArray(++splitString.begin(), splitString.end());

							function(&argArray);

							c2.text = "Function exectued!";
							c2.color = D3DCOLOR_ARGB(255, 0, 200, 100);
						}
					}

				} else if (c->argCount > 0 && splitString.size() < c->argCount + 1)
				{ // NOT ENOUGH ARGS
					c2.text = splitString.at(0) + " - " + c->description;
					c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);
				} else { // NO ARGS

					if (c->hasReturn)
					{
						typedef LPVOID(*funct)(void);
						funct function = (funct)c->func;
						vector<string>* retValue = (vector<string>*)function();


						if (retValue != nullptr) {

							for each(string text in *retValue)
							{
								c2.text += text + ", ";
							}

							c2.color = D3DCOLOR_ARGB(255, 138, 21, 21);
						} else {
							c2.text = "RETURN VALUE WAS NULL";
							c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
						}


					} else {
						typedef void(*funct)(void);
						funct function = (funct)c->func;
						function();

						c2.text = "Function executed!";
						c2.color = D3DCOLOR_ARGB(255, 18, 128, 45);
					}
				}

				isRecognized = true;
				break;
			}
		}


	}
	#pragma endregion


	#pragma endregion
	
	#pragma region OFFICIAL COMMANDS
	if (!isRecognized)
	{
		if (checkOfficialCommands(com, vector<string>(++splitString.begin(), splitString.end())))
		{
			this->currentCommand = "";
			return;
		}
	}
	#pragma endregion


	prevCommands.push_back(c2);

	this->currentCommand = "";
}

void d3Console::printHints()
{
	vector<command>	commandName;

	string com;
	for (size_t i = 0; i < this->currentCommand.length(); i++) {
		com += tolower( this->currentCommand.at(i) );
	}

	for each( varCommand* vCommand in varCommands )
	{
		string currCommand;
		for (size_t i = 0; i < vCommand->command.length(); i++) {
			currCommand += tolower( vCommand->command.at(i) );
		}

		if ( com.length() > 0 && strstr( currCommand.c_str(), com.c_str() ) )
		{
			command c;
			c.text = currCommand + " ";
			switch (vCommand->type) {
				case TYPES::T_FLOAT: {
					float	value		= *(float*)vCommand->variable;

					stringstream stream;
					stream << fixed << setprecision(2) << value;
					string str = stream.str();

					c.text += str;
					break;
				}
				case TYPES::T_INT:
				{
					int value = *(int*)vCommand->variable;
					c.text += to_string(value);
					break;
				}
				case TYPES::T_STRING:
				{
					string value = *(string*)vCommand->variable;

					c.text += value;
					break;
				}
				default:
					break;
			}

			c.color		= D3DCOLOR_ARGB(255,255,251,0);
			commandName.push_back( c );
		}
	}

	for each(callback* c in callbacks)
	{
		string currCommand;
		for (size_t i = 0; i < c->command.length(); i++) {
			currCommand += tolower(c->command.at(i));
		}

		if ( com.length() > 0 && strstr( currCommand.c_str(), com.c_str() ) )
		{
			command c;
			c.text = currCommand;
			c.color = D3DCOLOR_ARGB(255, 12, 189, 247);

			commandName.push_back(c);
		}
	}

	for each(string str in officialCommands)
	{
		if (com.length() > 0 && strstr(str.c_str(), com.c_str()))
		{
			command c;
			c.text = str;
			c.color = D3DCOLOR_ARGB(255, 173, 12, 12);

			commandName.push_back(c);
		}
	}


	this->commandHints = commandName;
}

bool d3Console::checkOfficialCommands(string com, vector<string> args)
{
	command c1;
	if ( com == "help" ) 
	{
		c1.color = D3DCOLOR_ARGB(255, 0, 200, 100);

		c1.text = "Variables: ";
		for(auto c : varCommands)
		{
			string varCommand;
			for (size_t i = 0; i < c->command.length(); i++) {
				varCommand += tolower(c->command[i]);
			}

			c1.text += varCommand + ", ";
		}
		this->prevCommands.push_back(c1);

		c1.text = "Functions: ";
		for (auto c : callbacks)
		{
			string varCommand;
			for (size_t i = 0; i < c->command.length(); i++) {
				varCommand += tolower(c->command[i]);
			}

			c1.text += varCommand + ", ";
		}
		this->prevCommands.push_back(c1);

		c1.text = "Console commands: ";
		for (auto c : officialCommands)
		{
			c1.text += c + ", ";
		}
		this->prevCommands.push_back( c1 );


		return true;
	} else if (com == "quit") {
		exit(0);
	} else if (com == "clear") {
		this->prevCommands.clear();
		return true;
	}

	return false;
}
