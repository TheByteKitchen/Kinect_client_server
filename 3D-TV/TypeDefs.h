#include "pch.h"
#include <functional>

#pragma once

using namespace std;

/*
	INACTIVE:		Initial state. Entered from either FRAMERECEIVED or EXCEPTION.
					If the KinectColorDepthClient returns to this state the object should be destroyed.
	CONNECTING:		Entered from STOPPED when Start button is clicked. Ends when first image is rendered.
	RECEIVING:		
	EXCEPTION:		Entered from either CONNECTING or RECEIVING. Signifies to the user / owner an exceptional
					situation has occurred, and the program cannot continue. After sending an error message, the
					Client moves to state STOPPED, so the client object can be destroyed.

*/
enum ReceiveState { EXCEPTION, INACTIVE, CONNECTING, RECEIVING};

typedef std::tr1::function<void (wstring message)> LoggingEventFunc;
typedef std::tr1::function<void ()> FrameReceivedCallback;
