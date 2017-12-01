//------------------------------------------------------------------------------
// <copyright file="Camera.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "pch.h"
#include "SceneComponent.h"
#include "SceneTime.h"

using namespace Windows::System;
using namespace DirectX;
using namespace Platform;
using namespace TheByteKitchen::DX;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		class CCamera : public SceneComponent
		{
		public:
			/// <summary>
			/// Constructor
			/// </summary>
			CCamera();
			~CCamera();

			void Initialize();
			void LoadContent();
			void Update(SceneTime time);
			void Render(SceneTime time);

			XMMATRIX View;

			/// <summary>
			/// Handles window messages, used to process input
			/// </summary>
			void KeyInput(VirtualKey key);

			/// <summary>
			/// Get the camera's up vector
			/// </summary>
			/// <returns>camera's up vector</returns>
			XMVECTOR  GetUp() { return m_up; }

			/// <summary>
			/// Get the camera's right vector
			/// </summary>
			/// <returns>camera's right vector</returns>
			XMVECTOR  GetRight() { return m_right; }

			/// <summary>
			/// Get the camera's position vector
			/// </summary>
			/// <returns>camera's position vector</returns>
			XMVECTOR  GetEye() { return m_eye; }

		private:
			float     m_rotationSpeed;
			float     m_movementSpeed;

			float     m_yaw;
			float     m_pitch;

			XMVECTOR  m_eye;
			XMVECTOR  m_at;
			XMVECTOR  m_up;
			XMVECTOR  m_forward;
			XMVECTOR  m_right;

			XMVECTOR  m_atBasis;
			XMVECTOR  m_upBasis;
		}; 
	}
}