# CMRenderer

This project is a rendering framework / primitive game engine I am implementing in C++ to learn graphics programming in depth. I have plans for portability, but the project is currently only buildable on Windows, and with MSVC bias.

The project also currently relies heavily on D3D11, and will terminate at runtime if not supported. (Cross platform is yet to be implemented)

## Prerequisites:
- A C++ compiler supporting C++20.
- CMake with a version of 3.31 or higher.
- Graphics drivers that support DirectX 11. (See *[How to determine the version of DirectX by using the DirectX Diagnostic Tool.](https://support.microsoft.com/en-us/topic/how-to-determine-the-version-of-directx-by-using-the-directx-diagnostic-tool-0c21cbce-55dc-2f9c-d3b1-f966c8339906#:~:text=Click%20Start%2C%20and%20then%20click,information%20for%20each%20DirectX%20file.)*)
- A *Windows* operating system.

## Note:
This project may not fully support a wide variety of IDE's, and is not currently guaranteed to be portable across platforms, or compilers other than MSVC.
You may need to update the project's *CMakeLists.txt* to fit your desired needs.

## Installing:
This project relies on various submodules
which *must* be cloned alongside the base repository.

To clone the repository:
```
git clone --recursive-submodules [this-repository-url]
```

Or if the repository has already been cloned:
```
git submodule update --init --recursive
```

## Building (CMake)
For CMake, you can generate a project file by running:
```
cmake -S . -B build -G "[your-generator-here]"
```

at the root directory of the project (the directory the project was cloned into).

Note: if you wish to use a generator other than Visual Studio 17 2022 (which is used by default in this project)
with Visual Studio's CMake integration, I recommend you also update the *"generator"* fields for
each configuration in *CMakeSettings.json* from *"Visual Studio 17 2022 Win64"* to your desired generator.

From here you can build using your IDE of choice, or invoke CMake by running:
```
cmake --build build
```

Side note: Instead of using CMake to generate a .sln file for Visual Studio, I'd recommend using
Visual Studio's native CMake integration by right-clicking on the project's root folder and clicking *"Open With Visual Studio"*.

## Latest Screenshot
![Latest Screenshot](https://github.com/mb-07fw/CMRenderer/blob/Test/screenshots/2025-11-23-(fixed__weird__texturing).png)