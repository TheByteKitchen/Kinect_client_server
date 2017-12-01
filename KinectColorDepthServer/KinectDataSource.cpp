//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// This code is an adaptation of the Color Depth sample
//// in the Kinect for Windows SDK. Hence,
////
//// Original: Copyright (c) Microsoft Corporation. All rights reserved.
////
//// Current: Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "stdafx.h"
#include "KinectDataSource.h"
#include <ppltasks.h>
#include <limits>

using namespace concurrency;

#undef max
#define MEDIANFILTER
#define RLECOMPRESSION

KinectDataSource::KinectDataSource(ColorWinSockSvr* colorSvr, DepthWinSockSvr* depthSvr, std::wostream& log) :
m_colorWinSockSvr(colorSvr),
m_depthWinSockSvr(depthSvr),
m_logstream(log),
IsRunning(false)
{
	// get resolution as DWORDS, but store as LONGs to avoid casts later
	DWORD width = 0;
	DWORD height = 0;

	NuiImageResolutionToSize(cDepthResolution, width, height);
	m_depthWidth = static_cast<LONG>(width);
	m_depthHeight = static_cast<LONG>(height);

	m_depthD16 = std::vector<unsigned short>(m_depthWidth*m_depthHeight);
	m_depthD16Filt = std::vector<unsigned short>(m_depthWidth*m_depthHeight);

	m_medianFilter = make_shared<dmfilter::DiscreteMedianFilter<unsigned short, 307200>>();

#ifdef RLECOMPRESSION
	m_runLengthCodecDepth = make_shared<runlengthcodec<unsigned short>>(m_depthWidth*m_depthHeight);
#endif

	NuiImageResolutionToSize(cColorResolution, width, height);
	m_colorWidth = static_cast<LONG>(width);
	m_colorHeight = static_cast<LONG>(height);

	m_colorCoordinates = new LONG[m_depthWidth*m_depthHeight * 2];

	unsigned int colsz = m_colorWidth*m_colorHeight*cBytesPerPixel;
	m_colorRGBXFilt = new BYTE[colsz];
	m_colorRGBXMapd = vector<unsigned char>(colsz);

	m_medianFilterRgb = make_shared<dmfilter::DiscreteMedianFilter<unsigned char, 1228800>>();

#ifdef RLECOMPRESSION
	m_runLengthCodecRgb = make_shared<runlengthcodec<unsigned char>>(colsz);
#endif

	m_colorToDepthDivisor = m_colorWidth / m_depthWidth;

	m_xyScale = 0.0f;

	m_bDepthReceived = false;
	m_bColorReceived = false;

	m_hNextDepthFrameEvent = INVALID_HANDLE_VALUE;
	m_pDepthStreamHandle = INVALID_HANDLE_VALUE;
	m_hNextColorFrameEvent = INVALID_HANDLE_VALUE;
	m_pColorStreamHandle = INVALID_HANDLE_VALUE;

	m_bPaused = false;

	// Calculate correct XY scaling factor so that our vertices are correctly placed in the world
	// This helps us to unproject from the Kinect's depth camera back to a 3d world
	// Since the Horizontal and Vertical FOVs are proportional with the sensor's resolution along those axes
	// We only need to do this for horizontal
	// I.e. tan(horizontalFOV)/depthWidth == tan(verticalFOV)/depthHeight
	// Essentially we're computing the vector that light comes in on for a given pixel on the depth camera
	// We can then scale our x&y depth position by this and the depth to get how far along that vector we are
	const float DegreesToRadians = 3.14159265359f / 180.0f;
	m_xyScale = tanf(NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * DegreesToRadians * 0.5f) / (m_depthWidth * 0.5f);
}

KinectDataSource::KinectDataSource(void) : m_logstream(std::wcerr) { }

KinectDataSource::~KinectDataSource(void)
{
	if (NULL != m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor->Release();
	}

	CloseHandle(m_hNextDepthFrameEvent);
	CloseHandle(m_hNextColorFrameEvent);

	// done with pixel data
	delete[] m_colorCoordinates;
	delete[] m_colorRGBXFilt;
}

void KinectDataSource::Init()
{
	if (FAILED(CreateFirstConnected()))
		throw std::runtime_error("Initialization error(s), ending run.\n");
}

void KinectDataSource::Run()
{
	IsRunning = true;

	// loop
	while (IsRunning)
	{
		ProcessFrame();
	}
}

void KinectDataSource::Stop()
{
	IsRunning = false;

	// Signal the client the server is shutting down
	const unsigned int SHUTDOWN_SIGNAL = std::numeric_limits<unsigned int>::max();
	vector<int> results(2);
	results[0] = m_depthWinSockSvr->Send(SHUTDOWN_SIGNAL);
	results[1] = m_depthWinSockSvr->Send(SHUTDOWN_SIGNAL);

	if (std::count(begin(results), end(results), -1) != 0)
	{
		m_logstream << "Sending Shutdown message failed." << endl;
	}

	// Cleanup other stuff
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectDataSource::CreateFirstConnected()
{
	INuiSensor * pNuiSensor = NULL;
	m_pNuiSensor = nullptr;
	HRESULT hr;

	int iSensorCount = 0;
	hr = NuiGetSensorCount(&iSensorCount);
	if (FAILED(hr))
	{
		m_logstream << L"Counting Kinect sensors failed" << endl;
		return hr;
	}

	// Look at each Kinect sensor
	for (int i = 0; i < iSensorCount; ++i)
	{
		// Create the sensor so we can check status, if we can't create it, move on to the next
		hr = NuiCreateSensorByIndex(i, &pNuiSensor);
		if (FAILED(hr))
		{
			continue;
		}

		// Get the status of the sensor, and if connected, then we can initialize it
		hr = pNuiSensor->NuiStatus();
		if (S_OK == hr)
		{
			m_pNuiSensor = pNuiSensor;
			break;
		}

		// This sensor wasn't OK, so release it since we're not using it
		pNuiSensor->Release();
	}

	if (NULL == m_pNuiSensor)
	{
		m_logstream << L"Finding an operational Kinect sensor failed" << endl;
		return E_FAIL;
	}

	// Initialize the Kinect and specify that we'll be using depth
	hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);
	if (FAILED(hr))
	{
		m_logstream << L"Initializing the Kinect sensor failed" << endl;
		return hr;
	}

	// Create an event that will be signaled when depth data is available
	m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Open a depth image stream to receive depth frames
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		cDepthResolution,
		NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES,
		//NULL,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle);
	if (FAILED(hr))
	{
		m_logstream << L"Initializing the Kinect depth stream failed" << endl;
		return hr;
	}

	// Create an event that will be signaled when color data is available
	m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Open a color image stream to receive color frames
	hr = m_pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		cColorResolution,
		0,
		2,
		m_hNextColorFrameEvent,
		&m_pColorStreamHandle);
	if (FAILED(hr))
	{
		m_logstream << L"Initializing the Kinect color stream failed" << endl;
		return hr;
	}

	m_frameEvents;
	m_frameEvents[0] = m_hNextDepthFrameEvent;
	m_frameEvents[1] = m_hNextColorFrameEvent;

	return hr;
}

/// <summary>
/// Process depth data received from Kinect
/// </summary>
/// <returns>S_OK for success, or failure code</returns>
HRESULT KinectDataSource::ProcessDepth()
{
	NUI_IMAGE_FRAME imageFrame;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
	if (FAILED(hr)) { return hr; }

	NUI_LOCKED_RECT LockedRect;
	hr = imageFrame.pFrameTexture->LockRect(0, &LockedRect, NULL, 0);
	if (FAILED(hr)) { return hr; }

#ifdef MEDIANFILTER
	m_medianFilter->Filter((unsigned short*)LockedRect.pBits, m_depthD16Filt.data());
#else
	memcpy((byte*)m_depthD16Filt.data(), LockedRect.pBits, LockedRect.size);
#endif

	m_bDepthReceived = true;

	hr = imageFrame.pFrameTexture->UnlockRect(0);
	if (FAILED(hr)) { return hr; };

	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

	return hr;
}

/// <summary>
/// Process color data received from Kinect
/// </summary>
/// <returns>S_OK for success, or failure code</returns>
HRESULT KinectDataSource::ProcessColor()
{
	NUI_IMAGE_FRAME imageFrame;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
	if (FAILED(hr)) { return hr; }

	NUI_LOCKED_RECT LockedRect;
	hr = imageFrame.pFrameTexture->LockRect(0, &LockedRect, NULL, 0);
	if (FAILED(hr)) { return hr; }

#ifdef MEDIANFILTER
	m_medianFilterRgb->Filter(LockedRect.pBits, m_colorRGBXFilt);
#else
	memcpy((byte*)m_colorRGBXFilt, LockedRect.pBits, LockedRect.size);
#endif

	m_bColorReceived = true;

	hr = imageFrame.pFrameTexture->UnlockRect(0);
	if (FAILED(hr)) { return hr; };

	hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

	return hr;
}

/// <summary>
/// Process color data received from Kinect
/// </summary>
/// <returns>S_OK for success, or failure code</returns>
HRESULT KinectDataSource::MapColorToDepth()
{
	long *longColors = (LONG *)m_colorRGBXFilt;
	long *ColorsMapped = (LONG *)m_colorRGBXMapd.data();
	HRESULT hr = S_OK;

	// Get of x, y coordinates for color in depth space
	// This will allow us to later compensate for the differences in location, angle, etc between the depth and color cameras
	m_pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
		cColorResolution,
		cDepthResolution,
		m_depthWidth*m_depthHeight,
		m_depthD16Filt.data(),
		m_depthWidth*m_depthHeight * 2,
		m_colorCoordinates
		);

	unsigned long index = 0;
	int depthIndex = 0;
	LONG colorInDepthX;
	LONG colorInDepthY;

	// loop over each row and column of the color
	for (LONG y = 0; y < m_colorHeight; ++y)
	{
		for (LONG x = 0; x < m_colorWidth; ++x)
		{
			// retrieve the depth to color mapping for the current depth pixel
			colorInDepthX = m_colorCoordinates[depthIndex++];
			colorInDepthY = m_colorCoordinates[depthIndex++];

			// make sure the depth pixel maps to a valid point in color space
			if (colorInDepthX >= 0 && colorInDepthX < m_colorWidth && colorInDepthY >= 0 && colorInDepthY < m_colorHeight)
			{
				// set source for copy to the color pixel
				long ind = colorInDepthX + colorInDepthY * m_colorWidth;
				long *pSrc = longColors + ind;

				ColorsMapped[index++] = *pSrc;
			}
			else
			{
				ColorsMapped[index++] = 0;
			}
		}
	}

	return hr;
}

vector<int> results(4);
HRESULT KinectDataSource::SendToClient()
{
#ifdef RLECOMPRESSION
	auto compressDepth = create_task([this]()
	{
		m_runLengthCodecDepth->encode(m_depthD16Filt);
	});

	auto compressColor = create_task([this]()
	{
		m_runLengthCodecRgb->encode(m_colorRGBXMapd);

	});

	auto joinCompression = compressDepth && compressColor;

	try
	{
		joinCompression.get();
	}
	catch (exception& ex)
	{
		m_logstream << ex.what();

		return E_FAIL;
	}

#if defined(MEDIANFILTER)
	if (m_runLengthCodecDepth->size() > (unsigned int)(m_depthWidth * m_depthHeight * sizeof(unsigned short)))
	{
		m_logstream << "Frame dropped. Depth frame size exceeded 614400: " << m_runLengthCodecDepth->size() << endl;

		return E_FAIL;
	}

	if (m_runLengthCodecRgb->size() > (unsigned int)(m_colorWidth * m_colorHeight * cBytesPerPixel))
	{
		auto &l = m_runLengthCodecRgb->channelsize;

		m_logstream << "Frame dropped. Color frame size exceeded 1228800: " << m_runLengthCodecRgb->size() << endl
			<< "RGB lengths:\tR: " << l[0] << "\tG: " << l[1] << " \tB: " << l[2] << endl;

		return E_FAIL;
	}
#endif //Medianfilter

	if (IsRunning)
	{
		results[0] = m_depthWinSockSvr->Send(sizeof(m_runLengthCodecDepth->channelsize), reinterpret_cast<unsigned char*>(&(m_runLengthCodecDepth->channelsize)));
		results[1] = m_depthWinSockSvr->Send(m_runLengthCodecDepth->size(), m_runLengthCodecDepth->data().get());
		results[2] = m_colorWinSockSvr->Send(sizeof(m_runLengthCodecRgb->channelsize), reinterpret_cast<unsigned char*>(&(m_runLengthCodecRgb->channelsize)));
		results[3] = m_colorWinSockSvr->Send(m_runLengthCodecRgb->size(), m_runLengthCodecRgb->data().get());

		if (std::count(begin(results), end(results), -1) != 0)
		{
			OutputDebugString(L"Send failed\n");

			this->Stop();
		}
	}

#else
	if (IsRunning)
	{
		unsigned int depsz = m_depthWidth*m_depthHeight*sizeof(USHORT);
		unsigned int colsz = m_colorWidth*m_colorHeight*cBytesPerPixel;

		results[0] = m_depthWinSockSvr->Send(depsz);
		results[1] = m_depthWinSockSvr->Send(depsz, (unsigned char*)m_depthD16Filt.data());

		results[2] = m_colorWinSockSvr->Send(colsz);
		results[3] = m_colorWinSockSvr->Send(colsz, m_colorRGBXMapd.data());

		if (std::count(begin(results), end(results), -1) != 0)
		{
			OutputDebugString(L"Send failed\n");

			this->Stop();
		}
	}
#endif

	return S_OK;
}

/// <summary>
/// Renders a frame
/// </summary>
/// <returns>S_OK for success, or failure code</returns>
HRESULT KinectDataSource::ProcessFrame()
{
	if (m_bPaused)
	{
		return S_OK;
	}

	if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, m_frameEvents, true, INFINITE))
	{
		if (SUCCEEDED(ProcessColor()) && SUCCEEDED(ProcessDepth()))
		{
			MapColorToDepth();

			SendToClient();
		}
	}

	return S_OK;
}