//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "PerfTimer.h"

// Initialize the resolution of the timer
LARGE_INTEGER PerfTimer::m_freq = \
          (QueryPerformanceFrequency(&PerfTimer::m_freq), PerfTimer::m_freq);

// Calculate the overhead of the timer
LONGLONG PerfTimer::m_overhead = PerfTimer::GetOverhead();