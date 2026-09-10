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

The Linux release is built on Ubuntu 24.04 x64. The workflow runs native
mathematical tests and GUI rendering checks under Xvfb.

To run the downloaded binary on Ubuntu 24.04, extract the `.tar.gz` archive
(which preserves executable permissions), install the runtime libraries if
needed, and launch from the extracted folder:

```sh
sudo apt install libfreetype6 libfontconfig1 libx11-6 libxcomposite1 libxcursor1 \
  libxext6 libxinerama1 libxrandr2 libxrender1 libgl1 libasound2t64 fonts-dejavu-core
./'Temperament Generator'
```

The binary needs glibc 2.39 or later and an X11 display (or XWayland). It is not
an AppImage; older distributions should compile from source instead.

Install a compiler, CMake, Git and the development packages used by JUCE's GUI:

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build git pkg-config \
  libasound2-dev libfontconfig1-dev libfreetype-dev libx11-dev libxcomposite-dev \
  libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libgl1-mesa-dev fonts-dejavu-core xvfb xauth
cmake -S . -B Builds/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build Builds/linux --parallel 2
ctest --test-dir Builds/linux --output-on-failure
'./Builds/linux/TemperamentGenerator_artefacts/Release/Temperament Generator'
```

Audio output uses JUCE's native device support (ALSA on Linux). Input channels
are never opened. FLAC decoding is compiled into the application; MP3, Ogg/Vorbis,
ASIO and the embedded browser are disabled. See the [JUCE 8.0.12 Linux dependency list](https://github.com/juce-framework/JUCE/blob/8.0.12/docs/Linux%20Dependencies.md).

For a machine without a display, run the GUI checks under Xvfb:

```sh
xvfb-run -a './Builds/linux/TemperamentGenerator_artefacts/Release/Temperament Generator' --render-preview Preview
```

For desktop validation, also test typing, menus, clipboard, window scaling and
scrolling in your chosen environment. Offscreen checks do not cover every desktop.
The `--render-preview` checks also decode every embedded sample and render dry
and reverberant audio offline, without requiring speakers or a sound device.
CTest checks pitch conversion, fade timing, filtering, decay and rapid retriggering.

The 25 checked-in FLACs are embedded during a normal build. FFmpeg and the
maintainer's original WAV directory are unnecessary for compilation or playback.
`Tools/prepare_samples.py` is only for regenerating FLAC assets from those originals.

## macOS: Apple Silicon and Intel

Install Xcode's command line tools (`xcode-select --install`), CMake and Git.
The release workflow builds separately on native Apple Silicon and Intel
runners, with macOS 13 as the deployment target:

```sh
cmake -S . -B Builds/mac -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0
cmake --build Builds/mac --parallel 2
ctest --test-dir Builds/mac --output-on-failure
open 'Builds/mac/TemperamentGenerator_artefacts/Release/Temperament Generator.app'
```

For GUI smoke checks, run the executable inside the app bundle with
`--render-preview Preview`. Release packaging verifies the architecture and
applies an ad-hoc signature; these builds are not Apple-notarised. Source builds
and downloads use system frameworks, with no separate JUCE runtime to install.

## Release automation

The **Prepare desktop release** workflow builds Windows x64, Linux x64 and
both Mac architectures from the same commit. Each job runs tests, renders GUI
checks, verifies its executable architecture, and uploads a native archive to
a draft release. Windows also provides the complete corresponding source,
including the JUCE checkout used by all four jobs.

The final job downloads all five archives, verifies their SHA-256 hashes and
uploads one `SHA256SUMS.txt`. The release stays a draft until reviewed and
published. Ordinary **Build and test** checks also cover all four runners.

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
