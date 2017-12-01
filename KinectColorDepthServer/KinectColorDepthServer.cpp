//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// IKinectColorDepthServer.cpp : Defines the exported functions for the DLL application.
//

#pragma once

#include "stdafx.h"
#include "KinectColorDepthServer.h"
#include "ColorWinsockSvr.h"
#include "DepthWinsockSvr.h"
#include "WinSock2SvrException.h"
#include <iostream>

using namespace concurrency;
using namespace std;


// Factory
extern "C" KINECTCOLORDEPTHSERVER_API IKinectColorDepthServer* CreateKinectColorDepthServer(std::wostream& log)
{
	return new KinectColorDepthServer(log);
}

KinectColorDepthServer::KinectColorDepthServer(std::wostream& log) : m_logstream(log), m_kinectDataSource(nullptr) { }

KinectColorDepthServer::~KinectColorDepthServer() { }

void KinectColorDepthServer::Destroy(IKinectColorDepthServer* svr)
{
	if (this == svr)
	{
		this->Stop();

		delete svr;
	}
}

inline void observe_exception(task<void>& t)
{
	t.then([](task<void> pt)
	{
		try
		{
			pt.get();
		}
		catch (...)
		{
			// observe
		}

	});
}

inline void observe_2_exceptions(task<void>& t1, task<void>& t2)
{
	observe_exception(t1);
	observe_exception(t2);
}

void KinectColorDepthServer::Run()
{
	bool IsRunning = true;

	// Create shared_ptrs to hold auto ref counting pointers
	auto ColorServer = make_shared<ColorWinSockSvr>();
	auto DepthServer = make_shared<DepthWinSockSvr>();

	// Init async, print out exception messages if any
	auto InitColorServer = create_task([ColorServer]
	{
		ColorServer->Initialize();
	});

	auto InitDepthServer = create_task([DepthServer]
	{
		DepthServer->Initialize();
	});

	// both task should end before continuing
	auto joinInits = InitColorServer && InitDepthServer;

	try
	{
		joinInits.wait();
	}
	catch (Winsock2SvrException& ex)
	{
		// read out msg
		m_logstream << ex.ErrorMessage();

		observe_2_exceptions(InitColorServer, InitDepthServer);

		IsRunning = false;
	}
	catch (exception& ex)
	{
		// read out msg
		m_logstream << ex.what();

		observe_2_exceptions(InitColorServer, InitDepthServer);

		IsRunning = false;
	}

	if (IsRunning)
	{
		// Connect the servers to client (One and the same client!!!)
		auto ConnectColorServer = create_task([ColorServer]
		{
			ColorServer->ListenAndConnect();
		});

		auto ConnectDepthServer = create_task([DepthServer]
		{
			DepthServer->ListenAndConnect();
		});

		auto joinConnects = ConnectColorServer && ConnectDepthServer;

		try
		{
			joinConnects.wait();

			m_logstream << "Made two connections" << endl;

			// Now check if we have the same client for both servers
			// If not throw exception; there should be only one client requesting.
			// But How???
		}
		catch (Winsock2SvrException& ex)
		{
			// read out msg
			m_logstream << ex.ErrorMessage();

			observe_2_exceptions(ConnectColorServer, ConnectDepthServer);

			IsRunning = false;
		}
		catch (exception& ex)
		{
			// read out msg
			m_logstream << ex.what();

			observe_2_exceptions(ConnectColorServer, ConnectDepthServer);

			IsRunning = false;
		}
	}

	if (IsRunning)
	{
		// Start receiving.
		auto colorRcvBuf = make_shared<vector<char>>(512);
		auto colorMsgLen = make_shared<int>();
		auto ColorSvrReceive = create_task([ColorServer, colorMsgLen, colorRcvBuf]
		{
			ColorServer->Receive(*colorMsgLen, *colorRcvBuf);
		});

		auto depthRcvBuf = make_shared<vector<char>>(512);
		auto depthMsgLen = make_shared<int>();
		auto DepthSvrReceive = create_task([DepthServer, depthMsgLen, depthRcvBuf]
		{
			DepthServer->Receive(*depthMsgLen, *depthRcvBuf);
		});

		auto joinReceives = ColorSvrReceive && DepthSvrReceive;

		try
		{
			joinReceives.wait();
		}
		catch (Winsock2SvrException ex)
		{
			// read out msg
			m_logstream << ex.ErrorMessage();

			observe_2_exceptions(ColorSvrReceive, DepthSvrReceive);

			IsRunning = false;
		}
		catch (exception ex)
		{
			// read out msg
			m_logstream << ex.what();

			observe_2_exceptions(ColorSvrReceive, DepthSvrReceive);

			IsRunning = false;
		}
	}

	if (IsRunning)
	{
		// Start Sending!
		m_kinectDataSource = make_shared<KinectDataSource>(ColorServer.get(), DepthServer.get(), m_logstream);

		auto RunKinect = create_task([this]
		{
			m_kinectDataSource->Init();

			m_kinectDataSource->Run();
		});

		try
		{
			RunKinect.wait();
		}
		catch (Winsock2SvrException ex)
		{
			this->Stop();

			// read out msg 
			m_logstream << ex.ErrorMessage();
		}
		catch (exception ex)
		{
			// read out msg
			m_logstream << ex.what();
		}
	}
}

void KinectColorDepthServer::Stop()
{
	if (m_kinectDataSource)
	{
		m_kinectDataSource->Stop();

		// generates a call on KinectDataSource's dtor
		m_kinectDataSource = nullptr;
	}
}



