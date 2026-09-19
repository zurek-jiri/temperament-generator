# Temperament Generator 1.7.0

Choose a known temperament directly in the application. **Load temperament**
replaces the Equal temperament button with a searchable catalogue of 31 entries.
Select a name, read its note, and load its fifth formulas and CSV output.

## What's new since 1.6.1

- Search catalogue names and notes, then load a temperament with **Load selected**,
  Enter or a double-click. Equal temperament remains the first catalogue entry.
- Fill both circle layouts with precise comma expressions and immediately show
  the catalogue's A-normalised cent chart, CSV output and chord playback tuning.
  Your pasted CSV input stays unchanged for comparison.
- Keep the catalogue values until you calculate the formulas. The status line
  indicates when rounded reconstructed expressions need **Close circle...**.
- Include a Linux application-menu setup helper and the tuning-fork icon, using
  the same embedded artwork as the application. No administrator rights needed.
- Verify Mac bundle icon resources and add native desktop integration checks.
- Update the illustrated user manual and website screenshots.

Playback remains enabled by default, with 50% loudness and three-second reverb.
Earlier releases, including 1.6.1 and 1.6.0, remain available in the release history.

## Download and run

| Download suffix | Computer | Instructions |
| --- | --- | --- |
| `windows-x64.zip` | Windows 10/11, Intel/AMD 64-bit | Extract the complete ZIP and run `Temperament Generator.exe`. No Win32 build is supplied. |
| `macos-arm64.zip` | Apple Silicon Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `macos-x64.zip` | Intel Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `linux-x64.tar.gz` | Intel/AMD 64-bit Linux, built on Ubuntu 24.04 | Extract to a permanent folder. Run `python3 install-desktop.py` without sudo for the menu launcher and icon, or run `./'Temperament Generator'` directly. |
| `source.zip` | Developers, all platforms | Complete project source plus the exact JUCE source. See `SOURCE_BUILD.txt`. |

No account or separate JUCE installation is needed to run the application.
The Windows executable contains its C++ runtime. Linux uses system audio and GUI libraries;
Ubuntu runtime setup and compilation instructions are in [BUILDING.md](https://github.com/zurek-jiri/temperament-generator/blob/v1.7.0/docs/BUILDING.md).

The Windows download is unsigned; the Mac applications use a local ad-hoc
signature and are not Apple-notarised. If macOS blocks opening a trusted download,
use its **System Settings → Privacy & Security → Open Anyway** control after
attempting to open the app. The builds run mathematical and offscreen GUI checks
on their native platforms; these do not replace testing every desktop or monitor.

Each binary archive includes the manual, licence notices and corresponding-source
information. `SHA256SUMS.txt` verifies all five archives. This is free and open
source under AGPLv3; the full licence and notices remain available inside the app.
