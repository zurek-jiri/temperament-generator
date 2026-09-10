# Temperament Generator 1.6.0

Hear the chords you see in Harmony lattice. This release adds a sampled Principal
8 organ and adjustable church-style reverb, following your current temperament.
Earlier releases, including 1.5.5 and 1.4.2, remain available in the release history.

## What's new since 1.5.5

- Updated purity colours: fifths **Standard 4/9 ct**, **Strict 2/6 ct**;
  thirds **Standard 8/19 ct**, **Strict 5/15 ct**. Gentle is removed from both
  menus. Thirds 21 cents sharp or flat appear red in the lattice and circles.
- Enable **Play Chords into Default Audio Output** at the lower right of Harmony
  lattice. Select a chord, triangle, or root to hear a root-position triad for
  two seconds, with a final 70 ms fade-out.
- All 25 Principal 8 recordings are embedded as lossless 24-bit, 48 kHz FLAC.
  Notes are repitched from their original A=440 equal temperament to the current
  chart. No external sample folder or codec installation is needed.
- **Loudness** adjusts the chord and reverb together from 0 to 100%, starting at
  **50%**, with smooth changes during playback.
- **Reverb** adjusts the approximate decay from 0 to 4 seconds, starting at
  **3 seconds**. Its wet signal
  is filtered below 200 Hz and above 2500 Hz; direct sound remains unfiltered.
- Previous notes fade when changing chords quickly. Leaving the lattice or
  changing/invalidating the chart stops the previous audition. The checkbox
  starts off and only audio output is opened, never microphone input.
- A scrollable chord list keeps the playback controls visible on small screens.
- Updated illustrated manual, website screenshots, FLAC notices, and automated
  pitch, fade, reverb-filtering, sample-decoding and native GUI checks.

Use your system settings for output destination and volume. If the default device
changes, uncheck/recheck playback. The visual calculator remains usable without
a sound device. The dry setting is useful for hearing interval beating clearly.

## Download and run

| Download suffix | Computer | Instructions |
| --- | --- | --- |
| `windows-x64.zip` | Windows 10/11, Intel/AMD 64-bit | Extract the complete ZIP and run `Temperament Generator.exe`. No Win32 build is supplied. |
| `macos-arm64.zip` | Apple Silicon Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `macos-x64.zip` | Intel Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `linux-x64.tar.gz` | Intel/AMD 64-bit Linux, built on Ubuntu 24.04 | Extract the archive and run `./'Temperament Generator'` from its folder (quote the name in a terminal). |
| `source.zip` | Developers, all platforms | Complete project source plus the exact JUCE source. See `SOURCE_BUILD.txt`. |

No account or separate JUCE installation is needed to run the application.
The Windows executable contains its C++ runtime. Linux uses system audio and GUI libraries;
Ubuntu runtime setup and compilation instructions are in [BUILDING.md](https://github.com/zurek-jiri/temperament-generator/blob/v1.6.0/docs/BUILDING.md).

The Windows download is unsigned; the Mac applications use a local ad-hoc
signature and are not Apple-notarised. If macOS blocks opening a trusted download,
use its **System Settings → Privacy & Security → Open Anyway** control after
attempting to open the app. The builds run mathematical and offscreen GUI checks
on their native platforms; these do not replace testing every desktop or monitor.

Each binary archive includes the manual, licence notices and corresponding-source
information. `SHA256SUMS.txt` verifies all five archives. This is free and open
source under AGPLv3; the full licence and notices remain available inside the app.
