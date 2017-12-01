//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "stdafx.h"
#include "ColorWinSockSvr.h"

const string ColorWinSockSvr::PORTNUMBER = "27015";

ColorWinSockSvr::ColorWinSockSvr(void) : WinSock2Svr(PORTNUMBER)
{
}


ColorWinSockSvr::~ColorWinSockSvr(void)
{
}
