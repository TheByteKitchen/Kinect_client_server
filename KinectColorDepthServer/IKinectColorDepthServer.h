//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KINECTCOLORDEPTHSERVER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KINECTCOLORDEPTHSERVER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KINECTCOLORDEPTHSERVER_EXPORTS
#define KINECTCOLORDEPTHSERVER_API __declspec(dllexport)
#else
#define KINECTCOLORDEPTHSERVER_API __declspec(dllimport)
#endif

class IKinectColorDepthServer
{
public:
	virtual void Run()=0;
	virtual void Stop()=0;
	virtual void Destroy(IKinectColorDepthServer* svr)=0;
};
