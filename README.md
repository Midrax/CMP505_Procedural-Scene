## Setup

1. Clone the repository and open PowerShell in its root directory:

   ```powershell
   git clone https://github.com/Midrax/CMP505_Procedural-Scene.git
   cd CMP505_Procedural-Scene
   ```

2. Prepare the dependencies before opening the Visual Studio solution:

   ```powershell
   .\scripts\Setup-Dependencies.ps1
   ```

   This downloads the pinned vcpkg tooling and installs the x64 Debug and Release
   libraries for DirectXTK, DirectXMath, SFML, ImGui, and Assimp.

   Generated tools, dependencies, and build outputs stay in the ignored `build/`
   directory. The script is needed on a fresh checkout, rather than before every
   build. No global vcpkg installation or NuGet package restore is required.

## Build and run

1. Open `Game.sln` in Visual Studio 2026.
2. Select **Debug** and **x64**, then build the solution.
3. Press **F5** to run with the debugger, or **Ctrl+F5** to run without it.

The executable is written to:

```text
build/bin/x64/Debug/Game.exe
```

Select **Release** for an optimized build; its executable is under
`build/bin/x64/Release/`.

The build copies runtime DLLs, assets, shaders, and license notices beside the
executable. Keep those accompanying files when copying a build elsewhere. The
application resolves its content from the executable directory.

To build from a **Developer PowerShell for VS 2026** terminal:

```powershell
msbuild Game.sln /m /p:Configuration=Release /p:Platform=x64
```

## Optional x86 build

Install the matching dependencies:

```powershell
.\scripts\Setup-Dependencies.ps1 -Triplet x86-windows
```

Select **x86** in the solution configuration. Visual Studio calls this platform
**Win32** in the project settings, and its outputs are under
`build/bin/Win32/Debug/` or `build/bin/Win32/Release/`.

## Audio credits

The soundtrack is **“36 Ghosts IV” by Nine Inch Nails**. Its attribution,
source, and CC BY-NC-SA license details are in [Assets/audio-credits.md](Assets/audio-credits.md).
