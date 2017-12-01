//------------------------------------------------------------------------------
// <copyright file="Camera.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//
//	   Additions and Adaptations: Copyright (c) The Byte Kitchen. All rights reserved.
//
// </copyright>
//------------------------------------------------------------------------------
#pragma once

#include "pch.h"
#include "Camera.h"

using namespace TheByteKitchen::_3D_TV;

/// <summary>
/// Constructor
/// </summary>
CCamera::CCamera() : View() 
{ }

CCamera::~CCamera() { }

/// <summary>
/// Reset the camera state to initial values
/// </summary>
void CCamera::Initialize()
{
	m_rotationSpeed = .0125f;
	m_movementSpeed = .03f;

	View       = XMMatrixIdentity();

	m_eye      = XMVectorSet(0.f, 0.f, -1.5f, 0.f);
	m_at       = XMVectorSet(0.f, 0.f,  10.f, 0.f);
	m_up       = XMVectorSet(0.f, 1.f,   0.f, 0.f);
	m_forward  = XMVectorSet(0.f, 0.f,   1.f, 0.f);
	m_right    = XMVectorSet(1.f, 0.f,   0.f, 0.f);

	m_atBasis  = XMVectorSet(0.f, 0.f,   1.f, 0.f);
	m_upBasis  = XMVectorSet(0.f, 1.f,   0.f, 0.f);

	m_yaw      = 0.f;
	m_pitch    = 0.f;
}

void CCamera::LoadContent() { }

/// <summary>
/// Update the view matrix
/// </summary>
void CCamera::Update(SceneTime time)
{
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

	m_at      = XMVector4Transform(m_atBasis, rotation);
	m_up      = XMVector4Transform(m_upBasis, rotation);
	m_forward = XMVector4Normalize(m_at);

	m_right   = XMVector3Cross(m_up, m_forward);
	m_right   = XMVector4Normalize(m_right);

	//take into account player position so they're always looking forward
	m_at += m_eye;

	View = XMMatrixLookAtLH(m_eye, m_at, m_up);
}

void CCamera::Render(SceneTime time) { }

/// <summary>
/// Handles window messages, used to process input
/// </summary>
void CCamera::KeyInput(VirtualKey key)
{
	using namespace Windows::System;

	switch(key)
	{

	case VirtualKey::Q:
	case VirtualKey::Left:
		{
			m_yaw -= m_rotationSpeed;
			break;
		}
	case VirtualKey::E:
	case VirtualKey::Right:
		{
			m_yaw += m_rotationSpeed;
			break;
		}
	case VirtualKey::R:
	case VirtualKey::Up:
		{
			m_pitch -= m_rotationSpeed;
			break;
		}
	case VirtualKey::F:
	case VirtualKey::Down:
		{
			m_pitch += m_rotationSpeed;
			break;
		}
	case VirtualKey::A:
		{
			m_eye -= m_right * m_movementSpeed;
			break;
		}
	case VirtualKey::D:
		{
			m_eye += m_right * m_movementSpeed;
			break;
		}
	case VirtualKey::S:
		{
			m_eye -= m_forward * m_movementSpeed;
			break;
		}
	case VirtualKey::W:
		{
			m_eye += m_forward * m_movementSpeed;
			break;
		}
	case VirtualKey::Space:
		{
			Initialize();
			break;
		}
	}
}

