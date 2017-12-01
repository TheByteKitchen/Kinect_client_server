//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "stdafx.h"
#include "DepthWinSockSvr.h"

const string DepthWinSockSvr::PORTNUMBER = "27016";

DepthWinSockSvr::DepthWinSockSvr(void) : WinSock2Svr(PORTNUMBER)
{
}


DepthWinSockSvr::~DepthWinSockSvr(void)
{
}
