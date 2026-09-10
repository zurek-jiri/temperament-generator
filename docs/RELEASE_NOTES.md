# Temperament Generator 1.6.1

Chord playback is now enabled by default in Harmony lattice. Select a chord to
hear it on the embedded Principal 8' organ, with 50% loudness and three-second
reverb. Launching the app remains silent; the audio output opens on the first
chord selection. Uncheck playback whenever you want silence.

## What's new since 1.6.0

- Enable **Play Chords into Default Audio Output** by default. Playback still
  lasts two seconds, including a final 70 ms fade-out, and uses output only.
- Correct the stop name to **Principal 8'**, including its foot-length mark.
- Refresh the embedded temperament catalogue with capitalized comment openings.
  Suggested-match comments now begin with **Note:**.
- Update the manual and website screenshots to reflect these refinements.

This release retains the 1.6.0 purity settings: fifths Standard 4/9 ct and Strict
2/6 ct; thirds Standard 8/19 ct and Strict 5/15 ct. Loudness and reverb remain
adjustable. No sample folder or codec installation is needed.

Earlier releases, including 1.6.0, remain available in the release history.

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
Ubuntu runtime setup and compilation instructions are in [BUILDING.md](https://github.com/zurek-jiri/temperament-generator/blob/v1.6.1/docs/BUILDING.md).

The Windows download is unsigned; the Mac applications use a local ad-hoc
signature and are not Apple-notarised. If macOS blocks opening a trusted download,
use its **System Settings → Privacy & Security → Open Anyway** control after
attempting to open the app. The builds run mathematical and offscreen GUI checks
on their native platforms; these do not replace testing every desktop or monitor.

Each binary archive includes the manual, licence notices and corresponding-source
information. `SHA256SUMS.txt` verifies all five archives. This is free and open
source under AGPLv3; the full licence and notices remain available inside the app.
