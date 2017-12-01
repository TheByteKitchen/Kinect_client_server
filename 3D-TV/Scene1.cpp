//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "pch.h"
#include "Scene1.h"
#include "KinectData.h"

#include <sstream>

using namespace std;
using namespace std::placeholders;
using namespace TheByteKitchen::_3D_TV;

Scene1::Scene1(void) : CurrentState(INACTIVE) {}

Scene1::~Scene1(void) {}

void Scene1::Initialize()
{
	KinectData::CreateKinectData();

	// see http://bec-systems.com/site/760/c-callbacks-to-member-functions for an explanation on (member) callbacks in C++
	LoggingEventFunc lef = std::tr1::bind(&Scene1::LogMessage, this, _1);

	FrameReceivedCallback frc = std::tr1::bind(&Scene1::RenderFrame, this);

	m_kinectColorDepthClient = make_shared<KinectColorDepthClient>(lef, frc);

	//
	// create other objects here
	//

	RegisterComponent(&m_cCamera);

	RegisterComponent(&m_kinectImage);

	// Register the components before callling initialise (on them);
	Scene::Initialize();
}

void Scene1::LoadContent()
{
	Scene::LoadContent();

	// ...
}

void Scene1::Update(SceneTime time)
{
	Scene::Update(time);

	// ...

	m_kinectImage.View = m_cCamera.View;
}

void Scene1::Render(SceneTime time)
{
	// ...

	Scene::Render(time);
}

void Scene1::Run(StreamSocket^ colorSocket, StreamSocket^ depthSocket)
{
	m_kinectColorDepthClient->StartReceiveAsync(colorSocket, depthSocket);
}

void Scene1::Stop()
{
	if (m_kinectColorDepthClient.get() != nullptr)
	{
		// Signals the client to come out of the data receive loop
		// Effectively makes the client return from the Run method
		m_kinectColorDepthClient->StopReceiveAsync();
	}
}

unsigned int cnt = 0;
void Scene1::RenderFrame()
{
	if (CurrentState == INITIALIZING)
	{
		CurrentState = RENDERING;
	}

	RunOneFrame();
}

// The LogMessage method is a callback, so
// it runs on the KinectColorDepthClient ReceiveAsync thread
void Scene1::LogMessage(wstring wstr)
{
	auto state = m_kinectColorDepthClient->GetState();

	if (state == ReceiveState::EXCEPTION)
	{
		// stop the run!
		CurrentState = EXCEPTION;

		ErrorMessages.push(wstr);
	}
	else if (state == ReceiveState::INACTIVE)
	{
		if (CurrentState != SCENE_STATE::INACTIVE)
			CurrentState = SCENE_STATE::INACTIVE;

		LogMessages.push(wstr);
	}
	else if (state == ReceiveState::CONNECTING)
	{
		if (CurrentState != SCENE_STATE::INITIALIZING)
			CurrentState = SCENE_STATE::INITIALIZING;

		LogMessages.push(wstr);
	}
	else //state == ReceiveState::RECEIVING 
	{
		LogMessages.push(wstr);
	}
}

wstring Scene1::GetLogMessage()
{
	wstring ws;

	LogMessages.try_pop(ws);

	return std::move(ws);
}

wstring Scene1::GetErrorMessage()
{
	wstring ws;

	ErrorMessages.try_pop(ws);

	return std::move(ws);
}

void Scene1::KeyBoardInput(VirtualKey key)
{
	m_cCamera.KeyInput(key);
}
