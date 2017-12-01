//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// pch.h
// Header for standard system include files.
//

#define _ITERATOR_DEBUG_LEVEL 0

#pragma once

// Windows 8
#include <wrl.h>
#include <collection.h>
#include "windows.ui.xaml.media.dxinterop.h"
#include <agile.h>
#include <wrl\implements.h>

// Concurrency
#include <ppltasks.h>

// DirectX
#include <d3d11_2.h>
#include <DirectXMath.h>

// STL
#include <vector>
#include <math.h>
#include <memory>
#include <string>

//Application
#include "App.xaml.h"
#include "Inlines.inl"

#pragma comment (lib, "d3d11.lib")


