//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "Scene1.h"

using namespace Platform::Collections;
using namespace std;

using namespace TheByteKitchen;

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		/// <summary>
		/// An empty page that can be used on its own or navigated to within a Frame.
		/// </summary>
		public ref class MainPage sealed
		{
		public:
			MainPage();

			void OnSuspending();

		private:
			bool				IsStarted;
			StreamSocket^		m_depthSocket;
			StreamSocket^		m_colorSocket;
			Platform::Guid		m_adapterID;
			Windows::Foundation::EventRegistrationToken cookie;

			// We cannot use a shared_ptr here.
			// Reason: framework uses DirectXMath: a.o. XMMatrix and XMVector.
			// Using shared_ptr disrupts memory alignment.
			Scene1*	scene1;

			Windows::Networking::Connectivity::NetworkStatusChangedEventHandler^ networkStatusCallback;

			void StartStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
			void Stop();
			void Update(Object^ sender, Object^ value);
			void KinectClientGrid_KeyDown_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);

			void OnNetworkStatusChange(Object^ sender);
		};
	}
}
