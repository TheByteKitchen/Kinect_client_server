The WinRT DirectX bus is a library that facilitates the creation and the use of DirectX components. It provides basic facilities such as a 3D Device, Context, SwapChain and the like. You just create components, such as a camera, a cube, an avatar, etc. and the code in the library will call its Load, Initialize, Update and Render methods. A component is plugged into the bus by calling the Register method on it.

How to use it.
* Download and build the source, or download the binary package.
* In a VC++ solution that you want to use this library with:
	* Add the directory containing the .h files to the Include directories
	* Add the path to the TheByteKitchen_DX directory name to the Library directories
	* Add "TheByteKitchen_DX.lib" (without quotes) to the Linker | Input | Additional Dependencies
* Refer to the .h files in your code.

The general usage pattern is to
* Derive a class from the Scene class in which you create and register your components
* Derive your components from the SceneComponent class.

See the 3D-TV application; the client in the Kinect Client Server application elsewhere on this project, for an example. A more extensive example, extensively described on The Byte Kitchen Blog, will be available in time.