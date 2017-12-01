//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

/*
This library is a WinSock2 server:
- ...

*/

class WinSock2Svr
{
public:
	WinSock2Svr(string portNr);
	~WinSock2Svr(void);

	void Initialize();
	void ListenAndConnect();
	void Receive(int& msgLen, vector<char>& buf);
	int Send(const int msgLen, const vector<char>*);
	int Send(const int msgLen, const unsigned char* buf);
	int Send(const unsigned int msg);
	
protected:
	string				portNumber;
	string				ErrorString;
	SOCKET				ListenSocket;
	SOCKET				ClientSocket;
	struct addrinfo*	addressInfo;
	bool				IsSending;

	WinSock2Svr(void);
};



