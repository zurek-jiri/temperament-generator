# Build Temperament Generator

The mathematical core uses C++17. The desktop interface uses JUCE 8.0.12. CMake
fetches that specific JUCE tag when `JUCE_PATH` is not supplied. Internet access
and Git are needed for the first dependency download. To build offline, obtain
the matching JUCE source beforehand and pass `-DJUCE_PATH=/path/to/JUCE`.

## Windows

Install Visual Studio with **Desktop development with C++**, CMake, and Git.
In a developer terminal at the project root:

```powershell
cmake -S . -B Builds/windows -A x64
cmake --build Builds/windows --config Release --parallel
ctest --test-dir Builds/windows -C Release --output-on-failure
& './Builds/windows/TemperamentGenerator_artefacts/Release/Temperament Generator.exe'
```

The original local shortcut, `python Tools/build.py --preview`, remains available
for the existing Visual Studio 2026 installation with JUCE in `C:/JUCE`.
The portable CMake commands above do not depend on that shortcut.

## Linux: Ubuntu 24.04

The Linux build workflow is prepared; a Linux release is not yet verified on
the maintainer's machine. Windows testing is not proof of Linux compatibility.

Install a compiler, CMake, Git and the development packages used by JUCE's GUI:

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build git pkg-config \
  libfontconfig1-dev libfreetype-dev libx11-dev libxcomposite-dev \
  libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libgl1-mesa-dev fonts-dejavu-core xvfb xauth
cmake -S . -B Builds/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build Builds/linux --parallel 2
ctest --test-dir Builds/linux --output-on-failure
'./Builds/linux/TemperamentGenerator_artefacts/Release/Temperament Generator'
```

The application uses no audio devices or embedded browser, so their optional
JUCE dependencies are unnecessary. See the [JUCE 8.0.12 Linux dependency list](https://github.com/juce-framework/JUCE/blob/8.0.12/docs/Linux%20Dependencies.md).

For a machine without a display, run the GUI checks under Xvfb:

```sh
xvfb-run -a './Builds/linux/TemperamentGenerator_artefacts/Release/Temperament Generator' --render-preview Preview
```

Before calling Linux supported, inspect the screenshots and test typing, menus,
clipboard, window scaling and scrolling on a real desktop. The initial JUCE 8
Linux target uses X11; Wayland desktops may run it through XWayland. Test the
chosen desktop environment rather than promising universal Linux support.
The compiled binary needs the corresponding system libraries; an AppImage or
distribution package can make installation easier after the basic build passes.

## Core-only build, without JUCE

```sh
cmake -S . -B Builds/core -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build Builds/core --config Release
ctest --test-dir Builds/core -C Release --output-on-failure
```

## Version source

`project(... VERSION ...)` in `CMakeLists.txt` is the application version source.
Use Git tags such as `v1.4.1` for releases. Keep the changelog and release notes
consistent with the tag; never replace an already published release with different files.
