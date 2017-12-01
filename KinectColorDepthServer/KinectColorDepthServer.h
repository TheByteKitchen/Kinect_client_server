//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

//#include "stdafx.h"
#include "IKinectColorDepthServer.h"
#include "KinectDataSource.h"
#include <iosfwd>
// Concurrency
#include <ppltasks.h>

class KinectColorDepthServer : public IKinectColorDepthServer
{
public:
	KinectColorDepthServer(std::wostream& log);
	~KinectColorDepthServer();

	void Run();
	void Stop();

	void Destroy(IKinectColorDepthServer* svr);
 
private:
	std::wostream&				 m_logstream;
	shared_ptr<KinectDataSource> m_kinectDataSource;
	
}; 