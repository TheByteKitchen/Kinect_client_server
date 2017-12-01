//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#include "stdafx.h"
#include "ColorWinSockSvr.h"
#include "DepthWinSockSvr.h"
#include "DiscreteMedianFilter.h"
#include "runlengthcodec.h"

using rlecodec::runlengthcodec;
using dmfilter::DiscreteMedianFilter;

///<summary>
/// This class represents a kinect camera: color and depth data
/// The interface is such that DirectX compatible data can be retreived;
/// no 'knowledge' of a kinect is required to further process the data
///</summary>

class KinectDataSource
{
	static const int                    cBytesPerPixel   = 4;
	static const NUI_IMAGE_RESOLUTION   cDepthResolution = NUI_IMAGE_RESOLUTION_640x480;
	static const NUI_IMAGE_RESOLUTION   cColorResolution = NUI_IMAGE_RESOLUTION_640x480;

	const string ColorWinsockPort;
	const string DepthWinsockPort;
public:
	KinectDataSource(ColorWinSockSvr* colorSvr, DepthWinSockSvr* depthSvr, std::wostream& log);
	~KinectDataSource(void);

	void Init();
	void Run();
	void Stop();

private:
	KinectDataSource(void);

	bool								IsRunning;
	std::wostream&						m_logstream;
	float                               m_xyScale;

	// Kinect
	INuiSensor*                         m_pNuiSensor;
	HANDLE                              m_hNextDepthFrameEvent;
	HANDLE                              m_pDepthStreamHandle;
	HANDLE                              m_hNextColorFrameEvent;
	HANDLE                              m_pColorStreamHandle;
	HANDLE								m_frameEvents[2];

	LONG                                m_depthWidth;
	LONG                                m_depthHeight;
	LONG                                m_colorWidth;
	LONG                                m_colorHeight;
	LONG                                m_colorToDepthDivisor;

	// For Discrete Median Filter
	shared_ptr<DiscreteMedianFilter<unsigned short, 307200>> m_medianFilter;
	shared_ptr<DiscreteMedianFilter<unsigned char, 1228800>> m_medianFilterRgb;

	// For Compression
	shared_ptr<runlengthcodec<unsigned short>>	m_runLengthCodecDepth;
	shared_ptr<runlengthcodec<unsigned char>>	m_runLengthCodecRgb;

	// For color
	unsigned char*                      m_colorRGBXFilt;
	vector<unsigned char>               m_colorRGBXMapd;
	vector<unsigned char>               m_colorRGBXMapdDummy;

	LONG*                               m_colorCoordinates;

	// For depth
	vector<USHORT>                      m_depthD16;
	vector<USHORT>                      m_depthD16Filt;
	vector<USHORT>                      m_depthD16FiltDummy;

	// for passing color data on to elsewhere
	ColorWinSockSvr*					m_colorWinSockSvr;
	DepthWinSockSvr*					m_depthWinSockSvr;

	// to prevent drawing until we have data for both streams
	bool                                m_bDepthReceived;
	bool                                m_bColorReceived;

	//bool                                m_bNearMode;
	// if the application is paused, for example in the minimized case
	bool                                m_bPaused;

	HRESULT CreateFirstConnected();
	//HRESULT ToggleNearMode();
	HRESULT ProcessFrame();
	HRESULT ProcessDepth();
	HRESULT ProcessColor();
	HRESULT MapColorToDepth();
	HRESULT SendToClient();
};


