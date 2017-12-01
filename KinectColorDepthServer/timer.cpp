//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "timer.h"

// Initialize the resolution of the timer
LARGE_INTEGER Timer::m_freq = \
          (QueryPerformanceFrequency(&Timer::m_freq), Timer::m_freq);

// Calculate the overhead of the timer
LONGLONG Timer::m_overhead = Timer::GetOverhead();