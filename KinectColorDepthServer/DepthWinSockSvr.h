//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "winsock2svr.h"

// Wrapper for WinSock2Svr that specifies a port number specific for Depth data
class DepthWinSockSvr : public WinSock2Svr
{
	static const string PORTNUMBER;

public:
	DepthWinSockSvr(void);
	virtual ~DepthWinSockSvr(void);
};

