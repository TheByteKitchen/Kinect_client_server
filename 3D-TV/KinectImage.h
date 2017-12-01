//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "pch.h"
#include "SceneComponent.h"
#include "Scene.h"
#include "VertexFormats.h"
#include "NuiApi.h"

using namespace Microsoft::WRL;
using namespace TheByteKitchen::DX;
using namespace std;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		/// <summary>
		/// Constant buffer for shader
		/// </summary>
		struct CBChangesEveryFrame
		{
			XMMATRIX View;
			XMMATRIX Projection;
			XMFLOAT4 XYScale;
		};

		class KinectImage : public SceneComponent
		{
		public:
			KinectImage(void);
			~KinectImage();

			void Initialize();
			void LoadContent();
			void Update(SceneTime time);
			void Render(SceneTime time);

			XMMATRIX View;

		private:
			// fields needed for a model mesh, a texture, and shaders
			ComPtr<ID3D11InputLayout>			m_inputLayout;
			ComPtr<ID3D11Buffer>				m_vertexBuffer;
			ComPtr<ID3D11GeometryShader>		m_geometryShader;
			ComPtr<ID3D11VertexShader>			m_vertexShader;
			ComPtr<ID3D11PixelShader>			m_pixelShader;
			ComPtr<ID3D11Buffer>                m_CBChangesEveryFrame;
			// for passing depth data as a texture
			ComPtr<ID3D11ShaderResourceView>    m_depthTextureRV;
			// for passing color data as a texture
			ComPtr<ID3D11ShaderResourceView>    m_colorTextureRV;
			ComPtr<ID3D11SamplerState>          m_colorSampler;

			XMMATRIX                            m_projection;

			bool								m_loadingComplete;

			// Separate depth and width for color and depth: 
			// Later we might turn to high res color info
			// there is no high res depth info (yet)
			LONG                                m_depthWidth;
			LONG                                m_depthHeight;
			LONG                                m_colorWidth;
			LONG                                m_colorHeight;
			float                               m_xyScale;
		};
	}
}

