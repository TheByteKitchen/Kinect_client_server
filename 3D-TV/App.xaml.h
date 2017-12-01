//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"

namespace TheByteKitchen
{
	namespace _3D_TV
	{
		/// <summary>
		/// Provides application-specific behavior to supplement the default Application class.
		/// </summary>
		ref class App sealed
		{
		public:
			App();
			virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;

		private:
			void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);

		};
	}
}
