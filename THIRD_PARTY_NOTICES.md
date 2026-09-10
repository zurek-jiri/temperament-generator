# Third-party components and artwork

Temperament Generator is maintained by `zurek-jiri` and distributed under
AGPL-3.0-only. See [LICENSE](LICENSE) and [COPYRIGHT](COPYRIGHT).

Full notices are included in `Licenses/`, embedded in the executable's
**About / licenses** panel, and included in release archives. The corresponding
source archive includes the exact JUCE source used to build the executable.

| Component | Version/source | Terms and preserved text |
| --- | --- | --- |
| JUCE | 8.0.12 | AGPLv3 route; [JUCE notice](Licenses/JUCE-LICENSE.md) and [AGPLv3](LICENSE) |
| HarfBuzz | Included in JUCE 8.0.12 | Old MIT; [copyright and permission](Licenses/HarfBuzz-COPYING.txt) |
| SheenBidi | Included in JUCE 8.0.12 | Apache 2.0; [license](Licenses/SheenBidi-LICENSE.txt); Copyright (C) 2014-2025 Muhammad Tayyab Akram |
| Independent JPEG Group | Included in JUCE 8.0.12 | [README and license](Licenses/Independent-JPEG-Group-README.txt) |
| libpng | Included in JUCE 8.0.12 | [license](Licenses/libpng-LICENSE.txt) |
| zlib | Included in JUCE 8.0.12 | [license](Licenses/zlib-LICENSE.txt) |
| libFLAC | Included in JUCE 8.0.12 | BSD-style [license and notices](Licenses/FLAC-LICENSE.txt) |

**This software is based in part on the work of the Independent JPEG Group.**

JUCE contains other libraries for optional modules. This application links its
GUI, graphics, events, core, data-structure, audio-basics, audio-device and
audio-format modules. Plugin hosting, Ogg/Vorbis, MP3 decoding and the embedded
browser are disabled. FLAC decoding uses JUCE's included library. Preserve original
notices throughout redistributed JUCE source. System fonts and operating system
libraries are used from the user's machine and are not bundled as assets.

The official JUCE terms are in the
[8.0.12 source distribution](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md).
This project uses the AGPLv3 option, not a commercial JUCE subscription.

## Project artwork and website

`Source/TuningFork.svg` is the application's original vector icon. Website images
are screenshots of this application. The local reference image `Source/TunFor.png`
is not used at runtime and is excluded from the public repository and releases.

The website uses local assets, system fonts and plain HTML/CSS/JavaScript.
Its original code, content and artwork use the project's AGPLv3 license.

## Principal 8 recordings

The project maintainer supplied the Principal8 recordings for inclusion and
distribution in this application. The 25 primary notes are stored losslessly in
`Source/PrincipalSamples/` and embedded in the executable. They follow the
project's AGPL-3.0-only licence. Their original filenames, PCM hashes and loop
metadata are preserved in the manifest. The alternate `069-A_a.wav` recording
is not used. FFmpeg is used only to prepare these assets; its executable and
libraries are not linked or distributed with this program.
