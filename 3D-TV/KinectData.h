#pragma once

#include "pch.h"
#include "KinectDataPtr.h"
#include "GraphicsDeviceManager.h"


using namespace TheByteKitchen::DX;
using std::shared_ptr;
using std::vector;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		/*
			This class holds data structures that are used by several componentes throughout the process chain:
			- input data structures
			- intermediate data structures
			- output data structures, if not only visual output, i.e. output to the screen 
		*/

		class KinectData
		{
			static const int m_depthWidth = 640;
			static const int m_depthHeight = 480;
			static const int m_colorWidth = 640;
			static const int m_colorHeight = 480;

		public:
			//
			// Ctors, Dtors
			//

			// Create copies of the description structures
			static void CreateKinectData();

			static KinectData* Current()
			{
				if (instance)
					return instance;
				else 
					throw std::exception("KinectData object not created, use the 'CreateKinectData' factory method first");
			}

			/*
			Nodig:

			- properties die de implementatie met een vector voor opslag verbergen
			- methoden om textures (zeg) toe te voegen. De data wordt niet door deze class gemaakt
			deze class is enkel bedoeld voor houden en ontsluiten van de data
			*/

			//
			// Properties
			//

			// Width in elements
			unsigned int ColorWidth();
			// Height in elements
			unsigned int ColorHeight();
			// Size in bytes
			unsigned int ColorByteSize();
			// Size in elements
			unsigned int ColorCnt();
			// Width in elements
			unsigned int DepthWidth();
			// Height in elements
			unsigned int DepthHeight();
			// Size in bytes
			unsigned int DepthByteSize();
			// Size in elements
			unsigned int DepthCnt();

			// Smart pointer to the texture data, that maps the texture when constructed, and unmaps it when destructed
			// Implicitly converts to unsigned char*
			unsigned char* ColorDataPtr();
			unsigned char* DepthDataPtr();
			unsigned char* DepthValuesPtr();

			//
			// Properties
			//
			ComPtr<ID3D11Device2> getDevice() { return m_d3dDevice; }
			ComPtr<ID3D11Texture2D> getDepthTexture() { return m_depthTexture2D; }
			shared_ptr<vector<unsigned int>> getDepthValues() { return m_depthValues; }

			//
			// Methods
			//

			void CreateColorSRV(ID3D11ShaderResourceView** ppSRView);
			void CreateDepthSRV(ID3D11ShaderResourceView** ppSRView);
			void CreateDepthFloatSRV(ID3D11ShaderResourceView** ppSRView);

		private:
			// State
			static KinectData* instance;
			D3D11_TEXTURE2D_DESC				m_colorTexDesc;
			D3D11_TEXTURE2D_DESC				m_depthTexDesc;

			shared_ptr<vector<unsigned int>>	m_depthValues;

			ComPtr<ID3D11Texture2D>             m_colorTexture2D;
			ComPtr<ID3D11Texture2D>             m_depthTexture2D;
			// for passing depth data as a texture
			ComPtr<ID3D11ShaderResourceView>    m_depthTextureRV;
			// for passing color data as a texture
			ComPtr<ID3D11ShaderResourceView>    m_colorTextureRV;
			ComPtr<ID3D11Device2>				m_d3dDevice;

			// Ctors, Dtors
			KinectData(void);
			KinectData(KinectData&);
			KinectData(D3D11_TEXTURE2D_DESC colorTexDesc, D3D11_TEXTURE2D_DESC depthTexDesc);
			~KinectData(void);
		};
	}
}