# Release validation

## 1.4.2

Locally verified on Windows:

- Release build and 3,509 mathematical checks.
- GUI regression checks for comma expressions, automatic fields, CSV import,
  interval sensitivities, contextual spellings and harmony selection.
- Rendered and reviewed the offline About, AGPLv3 and third-party notice panels.
- PE import inspection confirms that the Windows C++ runtime is linked into
  the executable; imported DLLs are Windows system components.
- Website screenshot controls, navigation and public links checked in Edge at
  desktop and 390 CSS-pixel mobile width.

The JUCE source bundled with the Windows release is the actual source used by
the compiler. It matches official JUCE 8.0.12 after line-ending normalization,
apart from an added UTF-8 byte-order mark in `juce_LeakedObjectDetector.h`.
This is an encoding difference, with no change to its C++ implementation.

Linux compilation and offscreen GUI checks run in GitHub Actions. Interactive
Linux desktop testing and clean-machine Windows testing remain separate release
validation tasks; CI alone does not establish support for every desktop or OS.
