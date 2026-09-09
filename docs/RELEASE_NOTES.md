# Temperament Generator 1.5.5

This release brings the recent circle, formula, chart and catalogue improvements
to Windows x64, macOS and Linux. Version 1.4.2 remains available in the release
history; the intermediate 1.5.0–1.5.4 builds were local development versions.

## What's new since 1.4.2

- Two synchronised layouts: compact inputs around the circle and a wide formula
  list. Both include a cent-deviation bar chart showing notes below/above equal
  temperament. Larger text and laptop layouts keep the controls readable.
- Coloured fifth arcs and interconnected major/minor thirds on the circles.
  Select a note to highlight its intervals; click outside to deselect.
- Rotate the temperament by one fifth in either direction. Rotation, calculation
  and closure controls sit beside the circle.
- One field can combine Pythagorean and syntonic commas and schisma. Generated
  expressions use readable forms such as `P*3/5` and spell out `schisma`.
- CSV import offers nearest fractions, including 1/24 and small multiples, or
  precise simple expressions. Closure can adjust one fifth, selected fifths,
  every fifth, automatic fields, or prefer simple expressions within a limit.
- CSV in always preserves the original pasted values for comparison with CSV out.
- An embedded catalogue of 31 temperaments recognises names and rotations.
  Suggestions now include related tunings beyond the old 1-cent cutoff, ranked
  by fifth and third differences. Exact/near matches remain first; alternatives
  are labelled Compare with their actual differences, not claimed identities.
- A comprehensive illustrated [user manual](https://github.com/zurek-jiri/temperament-generator/blob/v1.5.5/docs/USER_MANUAL.md), refreshed website
  screenshots and automated builds for all four desktop downloads.

## Download and run

| Download suffix | Computer | Instructions |
| --- | --- | --- |
| `windows-x64.zip` | Windows 10/11, Intel/AMD 64-bit | Extract the complete ZIP and run `Temperament Generator.exe`. No Win32 build is supplied. |
| `macos-arm64.zip` | Apple Silicon Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `macos-x64.zip` | Intel Mac, macOS 13 or later | Extract and move `Temperament Generator.app` to Applications. |
| `linux-x64.tar.gz` | Intel/AMD 64-bit Linux, built on Ubuntu 24.04 | Extract the archive and run `./Temperament Generator` from its folder (quote the name in a terminal). |
| `source.zip` | Developers, all platforms | Complete project source plus the exact JUCE source. See `SOURCE_BUILD.txt`. |

No account or separate JUCE installation is needed to run the application.
The Windows executable contains its C++ runtime. Linux uses system GUI libraries;
Ubuntu runtime setup and compilation instructions are in [BUILDING.md](https://github.com/zurek-jiri/temperament-generator/blob/v1.5.5/docs/BUILDING.md).

The Windows download is unsigned; the Mac applications use a local ad-hoc
signature and are not Apple-notarised. If macOS blocks opening a trusted download,
use its **System Settings → Privacy & Security → Open Anyway** control after
attempting to open the app. The builds run mathematical and offscreen GUI checks
on their native platforms; these do not replace testing every desktop or monitor.

Each binary archive includes the manual, licence notices and corresponding-source
information. `SHA256SUMS.txt` verifies all five archives. This is free and open
source under AGPLv3; the full licence and notices remain available inside the app.
