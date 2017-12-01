The Kinect Client Server system consists of a desktop application and a WinRT application. The desktop is a server that sends Kinect for Windows data over a WinSocket2 socket. The WinRT application is a WinRT client that connects to the server to receive and show the Kinect data.

You can download the server binary from here, and run it. You can also download a compiled version of the client (named 3D-TV) from here directly. You will need a developer license to install it on your Windows 8 pc. Only the x86 platform is currently supported. An entry in the Windows store for the client will come in time (removes the requirement of a developer license). 

You can also download the source code and build the system yourself. The client application requires the WinRT DirectX Bus to build. So you will have to download that one too and use it as described. The Project file assumes that the TheByteKitchenLibs directory (either the source code or the compiled version) is a sibling of the KinectClientServer directory.