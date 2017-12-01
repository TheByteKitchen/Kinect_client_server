//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) The Byte Kitchen. All rights reserved

//
// KinectColorDepthTestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IKinectColorDepthServer.h"

typedef IKinectColorDepthServer* (*IKinectColorDepthServer_Factory)(std::wostream&);

int _tmain(int argc, wchar_t* argv[])
{
	HINSTANCE dll_handle;

	// Get a handle to the DLL module.
	dll_handle = LoadLibrary(TEXT("KinectColorDepthServer.dll"));

	if (!dll_handle)
	{
		std::cerr << "Unable to initialize the dll\n";
		return 1;
	}

	std::cout << "Loaded the KinectColorDepthServer library\n";

	IKinectColorDepthServer_Factory factory =
		reinterpret_cast<IKinectColorDepthServer_Factory>(GetProcAddress(dll_handle, "CreateKinectColorDepthServer"));

	if (!factory) {
		std::cerr << "Unable to load create function from DLL!\n";
		::FreeLibrary(dll_handle);
		return 1;
	}

	try
	{
		while (true)
		{
			IKinectColorDepthServer* m_IkinectColorDepthServer = factory(std::wcout);

			std::cout << "Created KinectColorDepthServer\nCalling \"Run\" on the server, waiting for connections\n";

			// Synchronous call
			m_IkinectColorDepthServer->Run();

			std::cout << "KinectColorDepthServer \"Run\" ended\ncalling \"Destroy\" on the server\n";

			m_IkinectColorDepthServer->Destroy(m_IkinectColorDepthServer);

			std::cout << "Destroyed KinectColorDepthServer\n";
		}
	}
	catch (...)
	{
		std::cout << "Loop left because of top level exception\n";
	}

	FreeLibrary(dll_handle);

	std::cout << "Unloaded the library\n";

	std::cout << "\nHit return to close the application\n";

	std::getline(std::cin, std::string());

	return 0;
}

