//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// This code is an adaptation of the simple WinSock Server in the MSDN documentation. Hence,
////
//// Copyright (c) Microsoft Corporation. All rights reserved.
////
//// Adaptations:
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "stdafx.h"
#include "WinSock2Svr.h"
#include "WinSock2SvrException.h"

WinSock2Svr::WinSock2Svr(string portNr) : portNumber(portNr), IsSending(false)
{
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;

	addressInfo = NULL;
}

// No error handling
WinSock2Svr::~WinSock2Svr(void)
{
	// shutdown the connection since we're done
	shutdown(ClientSocket, SD_BOTH);

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
}

///<summary>
///Initializes Winsock and resolves server address and port
///</summary>
void WinSock2Svr::Initialize()
{
	WSADATA wsaData;
	struct addrinfo hints;

	// Initialize Winsock
	int res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (res != 0)
	{
		throw Winsock2SvrException(wstring(L"WSAStartup failed"), res);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	res = getaddrinfo(NULL, portNumber.c_str(), &hints, &addressInfo);
	if ( res != 0 ) 
	{
		WSACleanup();
		throw Winsock2SvrException(wstring(L"getaddrinfo failed"), res);
	}
}

///<summary>
///Creates a socket for listening to connection resquests, and accepts suitable requests
///Creates a (private) ClientSocket object - to use for reception and transmission. 
///</summary>
void WinSock2Svr::ListenAndConnect()
{
	// Create a SOCKET for connecting to server
	ListenSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(addressInfo);
		WSACleanup();
		throw Winsock2SvrException(wstring(L"socket failed"), WSAGetLastError());
	}

	// Setup the TCP listening socket
	int res = bind(ListenSocket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);
	if (res == SOCKET_ERROR)
	{
		freeaddrinfo(addressInfo);
		closesocket(ListenSocket);
		WSACleanup();
		throw Winsock2SvrException(wstring(L"bind failed"), WSAGetLastError());
	}

	freeaddrinfo(addressInfo);

	res = listen(ListenSocket, SOMAXCONN);
	if (res == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		WSACleanup();
		throw Winsock2SvrException(wstring(L"listen failed"), WSAGetLastError());
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		closesocket(ListenSocket);
		WSACleanup();
		throw Winsock2SvrException(wstring(L"accept failed"), WSAGetLastError());
	}

	// We are ready to send data
	IsSending = true;

	// No longer need server socket
	closesocket(ListenSocket);
}

///<summary>
/// Pumps data from buf into the socket asynchronously
///</summary>
int WinSock2Svr::Send(const int msgLen, const vector<char>* buf)
{
	int res=0;

	if (IsSending)
	{
		res = send( ClientSocket, buf->data(), msgLen, 0 );
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();

			IsSending = false;
			return res;
		}
	}
	else // CLose connection and socket
	{
		// shutdown the connection since we're done
		res = shutdown(ClientSocket, SD_SEND);
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();
			return res;
		}
	}

	return 0;
}

///<summary>
/// Pumps data from buf into the socket asynchronously
///</summary>
int WinSock2Svr::Send(const int msgLen, const unsigned char* buf)
{
	int res = 0;

	if (IsSending)
	{
		res = send(ClientSocket, (const char*)buf, msgLen, 0);
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();

			IsSending = false;
			return res;
		}
	}
	else // CLose connection and socket
	{
		// shutdown the connection since we're done
		res = shutdown(ClientSocket, SD_SEND);
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();
			return res;
		}
	}

	return 0;
}


///<summary>
/// Pumps data from buf into the socket asynchronously
///</summary>
int WinSock2Svr::Send(const unsigned int msg)
{
	int res = 0;

	if (IsSending)
	{
		res = send(ClientSocket, reinterpret_cast<const char*>(&msg), sizeof(int), 0);
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();

			IsSending = false;
			return res;
		}
	}
	else // CLose connection and socket
	{
		// shutdown the connection since we're done
		res = shutdown(ClientSocket, SD_SEND);
		if (res == SOCKET_ERROR)
		{
			closesocket(ClientSocket);
			WSACleanup();
			return res;
		}
	}

	return 0;
}

///<summary>
/// Reads res characters from the socket into buf. 
/// If a CLOSE_SOCKET message or an empty messagge is received
/// the socket is closed and we cleanup the runtime environment
///</summary>
void WinSock2Svr::Receive(int& msgLen, vector<char>& buf)
{
	int res = recv(ClientSocket, buf.data(), buf.capacity(), 0);
	if (res > 0) {

		msgLen = res;
	}
	else if (res == 0) // CLOSE_SOCKET message
		IsSending = false; // to be sure
	else 
	{
		closesocket(ClientSocket);
		WSACleanup();
		throw Winsock2SvrException(wstring(L"recv failed"), WSAGetLastError());
	}
}