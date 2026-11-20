# ClubEngine
Native C++20 game prototype using Vulkan 1.3 and SDL3.

## Milestone 0.1
- CMake native Windows build.
- SDL3 Vulkan window and surface.
- Vulkan 1.3 GPU selection.
- Graphics and presentation queue setup.
- Initial camera and scene interfaces.
- GitHub Actions Windows x64 artifact workflow.

## Build
Requires CMake 3.24+, Visual Studio 2022 C++ workload, Vulkan 1.3 SDK.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Every push to main triggers a Windows x64 artifact build.
