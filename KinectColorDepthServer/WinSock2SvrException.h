//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "stdafx.h"
#include <sstream>

using namespace std;

struct Winsock2SvrException : exception
{
	Winsock2SvrException(wstring msg, int errorCode) : message(msg), errorCode(errorCode) { }
	wstring ErrorMessage() 
	{
		wostringstream wost;
		wost << message << L" " << errorCode << endl;
		return wost.str();
	}

private:
	wstring message;
	int errorCode;
};