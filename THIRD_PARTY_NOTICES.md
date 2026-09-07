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

**This software is based in part on the work of the Independent JPEG Group.**

JUCE contains other libraries for optional modules. This application links its
GUI, graphics, events, core and data-structure modules; audio and plugin-hosting
modules are not linked and the embedded browser is disabled. Preserve original
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
