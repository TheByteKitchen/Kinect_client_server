//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "GraphicsDeviceManager.h"
#include <winerror.h>
#include <string>

using namespace TheByteKitchen::_3D_TV;
using namespace TheByteKitchen;

//using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage() : IsStarted(false), scene1(nullptr), m_adapterID()
{
	InitializeComponent();

	CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &MainPage::Update));

	//create handler for network status changed event
	networkStatusCallback = ref new Windows::Networking::Connectivity::NetworkStatusChangedEventHandler(this, &MainPage::OnNetworkStatusChange, CallbackContext::Same);

	cookie = Windows::Networking::Connectivity::NetworkInformation::NetworkStatusChanged += networkStatusCallback;
}

void MainPage::Update(Object^ sender, Object^ value)
{
	if (scene1)
	{
		auto state = scene1->GetState();

		if (state == SCENE_STATE::EXCEPTION)
		{
			// Button
			if (StartStop->IsEnabled)
				StartStop->IsEnabled = false;

			// Progress ring
			if (progress->IsActive)
			{
				progress->IsActive = false;
				LogText2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
				LogText2->Text = L"";
			}

			// Notifications
			auto str = scene1->GetErrorMessage();
			if (str != L"")
			{
				ErrorMessage->Text += ref new String(str.c_str());

				Stop();
			}
			else
			{
				// If there are no more error messages, 
				// destroy the scene and the KinectColorDpethClient
				if (scene1)
				{
					delete scene1;
					scene1 = nullptr;
				}

				if (m_colorSocket)
				{
					delete m_colorSocket;
					m_colorSocket = nullptr;
				}

				if (m_depthSocket)
				{
					delete m_depthSocket;
					m_depthSocket = nullptr;
				}

				// just to set the local flag correctly
				Stop();
			}
		}
		else if (state == SCENE_STATE::INACTIVE)
		{
			// Button
			if (!StartStop->IsEnabled && !IsStarted) // Shutting down activity
			{
				StartStop->Content = L"Start";
				StartStop->IsEnabled = true;

				if (scene1)
				{
					delete scene1;
					scene1 = nullptr;
				}

				if (m_colorSocket)
				{
					delete m_colorSocket;
					m_colorSocket = nullptr;
				}

				if (m_depthSocket)
				{
					delete m_depthSocket;
					m_depthSocket = nullptr;
				}
			}

			// Progress ring
			if (progress->IsActive)
			{
				progress->IsActive = false;
				LogText2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
				LogText2->Text = L"";
			}

			// Notifications
			// ...
		}
		else if (state == SCENE_STATE::INITIALIZING)
		{
			// Progress ring
			if (!progress->IsActive)
			{
				progress->IsActive = true;
				LogText2->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}

			// Notifications
			auto str = scene1->GetLogMessage();
			if (str != L"")
				LogText2->Text += ref new String(str.c_str());
		}
		else if (state == SCENE_STATE::RENDERING)
		{
			// Button
			if (!StartStop->IsEnabled && IsStarted)
			{
				StartStop->Content = L"Stop";
				StartStop->IsEnabled = true;
			}

			// Progress ring
			if (progress->IsActive)
			{
				progress->IsActive = false;
				LogText2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
				LogText2->Text = L"";

				// Get the ID of the local adapter involved in the conection. Required for handling a network disconnection error
				m_adapterID = m_colorSocket->Information->LocalAddress->IPInformation->NetworkAdapter->NetworkAdapterId;
			}

			// Notifications
			// ...
		}
	}
}

void _3D_TV::MainPage::StartStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StartStop->IsEnabled = false;

	if (!IsStarted)
	{
		try
		{
			ErrorMessage->Text = L"";

			GraphicsDeviceManager::CreateGraphicsDeviceManager(Window::Current->CoreWindow, this);

			if (GraphicsDeviceManager::Current()->FeatureLevel() < D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0)
			{
				throw ref new Platform::Exception(DXGI_ERROR_UNSUPPORTED, L"That is: 3D-TV requires DirectX 11(+) and this PC doesn't have it.\n");
			}

			scene1 = new Scene1();

			scene1->Initialize();

			scene1->LoadContent();

			m_colorSocket = ref new StreamSocket();
			m_depthSocket = ref new StreamSocket();

			scene1->Run(m_colorSocket, m_depthSocket);

			IsStarted = true;
		}
		catch (exception ex)
		{
			progress->IsActive = false;
			LogText2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

			wstringstream wsstr;
			wsstr << ex.what() << L"\n";

			ErrorMessage->Text += ref new String(wsstr.str().c_str());

			Stop();
		}
		catch (Platform::Exception^ ex)
		{
			progress->IsActive = false;
			LogText2->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

			auto hr = ex->HResult;
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				ErrorMessage->Text += L"Your graphics adapter is suspended, so 3D-TV cannot run.\n\nPlease restart the application.\n\nIf the problem persists, try a restart of your PC.\n";
			else
				ErrorMessage->Text += ex->Message;

			Stop();
		}
	}
	else
	{
		Stop();
	}
}

void MainPage::Stop()
{
	//OnNetworkStatusChange(this);

	if (scene1)
	{
		// we cannot delete scene1 or the StreamSockets here, the KinectColorDepthClient and KinectImage might still be cleaning up.

		scene1->Stop();
	}

	IsStarted = false;
	StartStop->Content = L"Start";
	StartStop->IsEnabled = true;
}

void _3D_TV::MainPage::KinectClientGrid_KeyDown_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	if (IsStarted)
		scene1->KeyBoardInput(e->Key);
}

void MainPage::OnSuspending()
{
	Stop();

}

void MainPage::OnNetworkStatusChange(Object^ sender)
{
	try
	{
		auto profiles = Windows::Networking::Connectivity::NetworkInformation::GetConnectionProfiles();
		for (auto prof : profiles)
		{
			if (m_adapterID == prof->NetworkAdapter->NetworkAdapterId)
			{
				if (prof->GetNetworkConnectivityLevel() == Windows::Networking::Connectivity::NetworkConnectivityLevel::None)
				{
					ErrorMessage->Text += L"Network Error.\nThe connection was lost.\n";

					Stop();
				}

				return;
			}
		}
	}
	catch (Exception^ ex)
	{
		ErrorMessage->Text += L"An unexpected exception occurred\nwhile checking for network errors.\n";

		Stop();
	}
}