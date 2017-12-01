//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved.
////
//// Adaptations:
//// Copyright (c) The Byte Kitchen. All rights reserved

#include "pch.h"
#include "KinectImage.h"
#include "BasicLoader.h"
#include "KinectData.h"

using namespace TheByteKitchen::_3D_TV;

Platform::String^ GEOMETRY_SHADER = "GeometryShader.cso";
Platform::String^ VERTEX_SHADER	= "VertexShader.cso";
Platform::String^ PIXEL_SHADER	= "PixelShader.cso";

KinectImage::KinectImage() : m_colorWidth(640), m_colorHeight(480), m_depthWidth(640), m_depthHeight(480), m_xyScale(0.0f) { }

KinectImage::~KinectImage() { }

///<summary>
///
///</summary>
void KinectImage::Initialize( void )
{
	SceneComponent::Initialize();

	KinectData::Current()->CreateColorSRV(&m_colorTextureRV);

	KinectData::Current()->CreateDepthSRV(&m_depthTextureRV);

	// Create the vertex buffer
	D3D11_BUFFER_DESC bd = {0};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(short);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ThrowIfFailed(
		m_d3dDevice->CreateBuffer(&bd, NULL, m_vertexBuffer.GetAddressOf())
		);

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	ThrowIfFailed(
		m_d3dDevice->CreateBuffer(&bd, NULL, m_CBChangesEveryFrame.GetAddressOf())
		);

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ThrowIfFailed(
		m_d3dDevice->CreateSamplerState( &sampDesc, &m_colorSampler )
		);

	// Reasonable default
	View = XMMatrixIdentity();

	// Initialize the projection matrix
	m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_depthWidth / static_cast<FLOAT>(m_depthHeight), 0.1f, 100.f);

	// Calculate correct XY scaling factor so that our vertices are correctly placed in the world
	// This helps us to unproject from the Kinect's depth camera back to a 3d world
	// Since the Horizontal and Vertical FOVs are proportional with the sensor's resolution along those axes
	// We only need to do this for horizontal
	// I.e. tan(horizontalFOV)/depthWidth == tan(verticalFOV)/depthHeight
	// Essentially we're computing the vector that light comes in on for a given pixel on the depth camera
	// We can then scale our x&y depth position by this and the depth to get how far along that vector we are
	const float DegreesToRadians = 3.14159265359f / 180.0f;
	m_xyScale = tanf(NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * DegreesToRadians * 0.5f) / (m_depthWidth * 0.5f);    

	// Set rasterizer state to disable backface culling
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.DepthBias = false;
	rasterDesc.DepthBiasClamp = 0;
	rasterDesc.SlopeScaledDepthBias = 0;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	ComPtr<ID3D11RasterizerState> pState;

	ThrowIfFailed(
		m_d3dDevice->CreateRasterizerState(&rasterDesc, pState.GetAddressOf())
		);

	m_d3dContext->RSSetState(pState.Get());

}

//<summary>
/// Loads the quad mesh from file and initiates corresponding buffers and views
/// Loads the texture from file and initiates corresponding buffers and views
///</summary>
void KinectImage::LoadContent( void )
{
	SceneComponent::LoadContent();

	// Define the vertex input layout
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{ { "POSITION", 0, DXGI_FORMAT_R16_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	// Create a BasicLoader, and use it to asynchronously load all
	// application resources.  When an output value becomes non-null,
	// this indicates that the asynchronous operation has completed.
	BasicLoader^ loader = ref new BasicLoader(m_d3dDevice.Get(), m_wicFactory.Get());

	auto loadGeometryShaderTask	= loader->LoadShaderAsync( GEOMETRY_SHADER, &m_geometryShader);

	auto loadVertexShaderTask	= loader->LoadShaderAsync( VERTEX_SHADER, layout, ARRAYSIZE(layout), &m_vertexShader, &m_inputLayout );

	auto loadPixelShaderTask	= loader->LoadShaderAsync( PIXEL_SHADER, &m_pixelShader );

	(loadGeometryShaderTask && loadVertexShaderTask && loadPixelShaderTask).then([=]()
	{
		// set semaphore for internal use
		m_loadingComplete = true; 
	});
}

void KinectImage::Update(SceneTime time)
{
	// Update variables that change once per frame
	CBChangesEveryFrame cb;
	cb.View = XMMatrixTranspose(View);
	cb.Projection = XMMatrixTranspose(m_projection);
	cb.XYScale = XMFLOAT4(m_xyScale, -m_xyScale, 0.f, 0.f); 

	m_d3dContext->UpdateSubresource(m_CBChangesEveryFrame.Get(), 0, NULL, &cb, 0, 0);

	// Doesn't do anything (yet)
	//SceneComponent::Update(time);
}

void KinectImage::Render(SceneTime time)
{
	m_d3dContext->OMSetRenderTargets( 1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get() );

	// Clear the back buffer
	static float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);

	// Clear the depth buffer to 1.0 (max depth)
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set vertex buffer
	UINT stride = 0;
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set primitive topology
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Set the input vertex layout
	// In this case we don't actually use it for anything
	// All the work is done in the geometry shader, but we need something here
	// We only need to set this once since we have only one vertex format
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	// Set up shaders
	m_d3dContext->VSSetShader(m_vertexShader.Get(), NULL, 0);

	m_d3dContext->GSSetShader(m_geometryShader.Get(), NULL, 0);
	m_d3dContext->GSSetConstantBuffers(0, 1, m_CBChangesEveryFrame.GetAddressOf());

	m_d3dContext->GSSetShaderResources(0, 1, m_depthTextureRV.GetAddressOf());
	m_d3dContext->GSSetShaderResources(1, 1, m_colorTextureRV.GetAddressOf());
	
	m_d3dContext->GSSetSamplers(0, 1, m_colorSampler.GetAddressOf());

	m_d3dContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Draw the scene
	m_d3dContext->Draw(m_depthWidth * m_depthHeight, 0);

	ID3D11ShaderResourceView* SRViewNULL[2] = { NULL, NULL };
	m_d3dContext->GSSetShaderResources(0, 2, SRViewNULL);

	//SceneComponent::Render(time);
}