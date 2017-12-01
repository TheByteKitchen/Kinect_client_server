//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "pch.h"
#include "Scene.h"
// if you include a component derived from this one, remove this include
// #include "SceneComponent.h" 
#include "TypeDefs.h"
#include "KinectColorDepthClient.h"
#include "KinectImage.h"
#include "camera.h"
#include <concurrent_queue.h>

using namespace std;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		enum SCENE_STATE { EXCEPTION, INACTIVE, INITIALIZING, RENDERING };

		///<summary>
		/// Manages and renders the overall scene. Rendering is calling the Update and Draw methods on components
		/// Managment includes lifetime management of components representing elements within the scene
		///</summary>
		class Scene1 : public Scene
		{
		public:
			// ctors, dtors
			Scene1(void);
			~Scene1(void);

			// Scene overrides. We don't use Scene::Run here since
			// The arrival of data at KinectColorDepthClient ticks the clock
			void Initialize();
			void LoadContent();
			void Update(SceneTime time);
			void Render(SceneTime time);

			void Run(StreamSocket^ colorSocket, StreamSocket^ depthSocket);
			void Stop();
			wstring GetLogMessage();
			wstring GetErrorMessage();
			SCENE_STATE GetState() { return CurrentState; }
			void KeyBoardInput(VirtualKey key);

		private:
			SCENE_STATE							CurrentState;
			concurrent_queue<wstring>			LogMessages;
			concurrent_queue<wstring>			ErrorMessages;

			shared_ptr<KinectColorDepthClient>	m_kinectColorDepthClient;

			// Note: not a (shared) pointer, because of DirectXMath
			KinectImage							m_kinectImage;
			CCamera								m_cCamera;

			void LogMessage(wstring wstr);
			void RenderFrame();
		};
	}
}