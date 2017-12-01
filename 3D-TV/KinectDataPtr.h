//// This class is not meant for general use. But only for use by the KinectData class
//// In case of other uses, remember to only create objects only on the stack (or face severe problems).
////

#pragma once

//#include "pch.h"
#include "GraphicsDeviceManager.h"

using namespace Microsoft::WRL;
using namespace TheByteKitchen::DX;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		class KinectDataPtr
		{
		public:
			KinectDataPtr(ComPtr<ID3D11Texture2D>);
			~KinectDataPtr(void);

			static void SetContext3D();

			operator unsigned char*() const;

		private:
			static ComPtr<ID3D11DeviceContext2> m_d3dContext;
			ComPtr<ID3D11Texture2D>				m_texture2D;
			D3D11_MAPPED_SUBRESOURCE			m_mapdSubRes; 

			KinectDataPtr(void);
		};
	}
}