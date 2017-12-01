#include "pch.h"
#include "KinectData.h"

using namespace TheByteKitchen::_3D_TV;

KinectData* KinectData::instance = nullptr;

//
// Ctors, Dtors
//

KinectData::KinectData(void) { }

KinectData::KinectData(KinectData&) { }

KinectData::KinectData(D3D11_TEXTURE2D_DESC colorTexDesc, D3D11_TEXTURE2D_DESC depthTexDesc) : 
	m_colorTexDesc(colorTexDesc), m_depthTexDesc(depthTexDesc)
{ 
	m_d3dDevice = GraphicsDeviceManager::Current()->D3DDevice();

	KinectDataPtr::SetContext3D();

	ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(&colorTexDesc, NULL, &m_colorTexture2D )
		);

	//ThrowIfFailed(DXGI_ERROR_DEVICE_REMOVED);

	ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(&depthTexDesc, NULL, &m_depthTexture2D)
		);
}

KinectData::~KinectData(void) { }

//
// Factory
//

void KinectData::CreateKinectData()
{
	// Create depth texture
	// CPU will write it
	D3D11_TEXTURE2D_DESC depthTexDesc = {0};
	depthTexDesc.Width = m_depthWidth;
	depthTexDesc.Height = m_depthHeight;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_R16_SINT;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	depthTexDesc.MiscFlags = 0;

	// Create color texture
	D3D11_TEXTURE2D_DESC colorTexDesc = {0};
	colorTexDesc.Width = m_colorWidth;
	colorTexDesc.Height = m_colorHeight;
	colorTexDesc.MipLevels = 1;
	colorTexDesc.ArraySize = 1;
	colorTexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	colorTexDesc.SampleDesc.Count = 1;
	colorTexDesc.SampleDesc.Quality = 0;
	colorTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	colorTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	colorTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colorTexDesc.MiscFlags = 0;

	instance = new KinectData(colorTexDesc, depthTexDesc);

	/*
	Create the resource views here. Then you can release the pointers to the textures, if you like.
	*/

}

//
// Properties
//

unsigned char* KinectData::ColorDataPtr()
{
	return KinectDataPtr(m_colorTexture2D);
}

unsigned char* KinectData::DepthDataPtr()
{
	return KinectDataPtr(m_depthTexture2D);
}

unsigned char* KinectData::DepthValuesPtr()
{
	return (unsigned char*)m_depthValues->data();
}

// Width in elements
unsigned int KinectData::ColorWidth()
{
	return m_colorTexDesc.Width;
}

// Height in elements
unsigned int KinectData::ColorHeight()
{
	return m_colorTexDesc.Height;
}

// Size in bytes
unsigned int KinectData::ColorByteSize()
{
	return m_colorTexDesc.Width * m_colorTexDesc.Height * sizeof(unsigned int);
}

// Size in elements
unsigned int KinectData::ColorCnt()
{
	return m_colorTexDesc.Width * m_colorTexDesc.Height;
}

unsigned int KinectData::DepthWidth()
{
	return m_depthTexDesc.Width;
}

// Height in elements
unsigned int KinectData::DepthHeight()
{
	return m_depthTexDesc.Height;
}

// Size in bytes
unsigned int KinectData::DepthByteSize()
{
	return m_depthTexDesc.Width * m_depthTexDesc.Height * sizeof(unsigned short);
}

// Size in elements
unsigned int KinectData::DepthCnt()
{
	return m_depthTexDesc.Width * m_depthTexDesc.Height;
}


void KinectData::CreateColorSRV(ID3D11ShaderResourceView** ppSRView)
{
	ThrowIfFailed(
		m_d3dDevice->CreateShaderResourceView(m_colorTexture2D.Get(), NULL, ppSRView)
		);
}

void KinectData::CreateDepthSRV(ID3D11ShaderResourceView** ppSRView)
{
	ThrowIfFailed(
		m_d3dDevice->CreateShaderResourceView(m_depthTexture2D.Get(), NULL, ppSRView)
		);
}
