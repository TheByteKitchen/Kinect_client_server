//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

#pragma once

#define LOCAL

#include "TypeDefs.h"

using namespace concurrency;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Platform;
using namespace std;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
#ifdef LOCAL
		static String^ HOSTNAME = L"localhost";
#else
		static String^ HOSTNAME = L"192.168.0.20";
#endif
		static String^ COLORPORT = L"27015";
		static String^ DEPTHPORT = L"27016";

		class KinectColorDepthClient 		
		{
			int PixelSize = 640 * 480;
			int ColorByteSize;
			int DepthByteSize;

			vector<unsigned short> depthdata;		
			vector<unsigned char> colordata;

		public:			
			KinectColorDepthClient(LoggingEventFunc lef, FrameReceivedCallback frc);

			// Receives Kinect data from server and loads it into textures already in the GPU
			void StartReceiveAsync(StreamSocket^ colorSocket, StreamSocket^ depthSocket);
			void StopReceiveAsync();
			ReceiveState GetState() { return CurrentState; }

		private:
			LoggingEventFunc		LogMessage;
			FrameReceivedCallback	FrameReceived;
			ReceiveState			CurrentState;

			unsigned int	dataLoadCnt;
			double cnt = 0;

			KinectColorDepthClient(void);
		};
	}
}