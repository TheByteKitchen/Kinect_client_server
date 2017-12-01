//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved.
////
//// Adaptations:
//// Copyright (c) The Byte Kitchen. All rights reserved


// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _ITERATOR_DEBUG_LEVEL 0

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// STL
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>

// DirectX
#include <d3d11_2.h>
#include <xnamath.h>

// Kinect
#include "NuiApi.h"
#include "NuiImageCamera.h"





