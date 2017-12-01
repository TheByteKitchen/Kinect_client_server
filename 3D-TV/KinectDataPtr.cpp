#include "pch.h"
#include "KinectDataPtr.h"

using namespace TheByteKitchen::_3D_TV;
using namespace TheByteKitchen::DX;

ComPtr<ID3D11DeviceContext2> KinectDataPtr::m_d3dContext = nullptr;

KinectDataPtr::KinectDataPtr(void) { }

KinectDataPtr::KinectDataPtr(ComPtr<ID3D11Texture2D> texture2D) : m_texture2D(texture2D),  m_mapdSubRes()
{
	ThrowIfFailed(
		m_d3dContext->Map(m_texture2D.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &	m_mapdSubRes)
		);
}

KinectDataPtr::~KinectDataPtr(void)
{
	m_d3dContext->Unmap(m_texture2D.Get(), NULL);
}

KinectDataPtr::operator unsigned char*() const
{ 
	return  static_cast<unsigned char*>(m_mapdSubRes.pData); 
}

void KinectDataPtr::SetContext3D()
{
	m_d3dContext = GraphicsDeviceManager::Current()->D3DContext();
}
