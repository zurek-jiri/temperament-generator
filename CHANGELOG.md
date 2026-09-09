# Changelog

## 1.5.5 — 2026-09-09

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
- A comprehensive illustrated [user manual](docs/USER_MANUAL.md), refreshed website
  screenshots and automated builds for all four desktop downloads.


Versions 1.5.0–1.5.4 were local development builds, incorporated in this release.

## 1.4.2 — 2026-09-07

- Prepare GitHub documentation, Windows/Linux build checks and project website.
- Make JUCE dependency setup portable and use platform font defaults on Linux.
- Read the application version from CMake.
- Release under AGPLv3, maintained by zurek-jiri.
- Accessible About / licenses panel with full offline license and library notices.
- Bundle the Windows C++ runtime into the executable.
- Public project website and corresponding-source packaging.

## 1.4.1

- Independent, stricter fifth and more forgiving third colour sensitivities.
- Thicker harmony lattice connections and larger arrowheads.
- Note spelling follows the selected key, including C# in A major and Db in Bb minor.

## 1.4.0

- Separate harmony lattice tab with fifths and thirds in three directions.
- Selectable major/minor triads, interval details and colour sensitivity.

## 1.3.1

- Auto corrections show nearby syntonic and Pythagorean fractions with errors.

## 1.3.0

- Persistent automatic fields share the remaining correction equally in cents.
- New tuning-fork application icon.

## 1.2.1

- Reverse CSV conversion uses the native comma column for recognised fractions.

## Earlier development

- Fifth circles, arithmetic expressions, schisma, paired comma inputs,
  A-relative cent charts and semicolon-separated CSV import/export.
