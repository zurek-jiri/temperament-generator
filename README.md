# Temperament Generator 1.6.0

Desktop tools for designing twelve-note temperaments, converting comma corrections
to tuning charts, and exploring fifths and thirds in a harmony lattice.

- [Build on Windows, macOS or Linux](docs/BUILDING.md)
- [User manual](docs/USER_MANUAL.md)
- [Project website](https://zurek-jiri.github.io/temperament-generator/)
- [Downloads and corresponding source](https://github.com/zurek-jiri/temperament-generator/releases)
- [Release history](CHANGELOG.md) · [Contributing](CONTRIBUTING.md)
- [First public release checklist](docs/PUBLISHING.md) · [Third-party notices](THIRD_PARTY_NOTICES.md)

Maintained by [zurek-jiri](https://github.com/zurek-jiri). Free and open source under
[AGPLv3](LICENSE). Copyright (C) 2026 zurek-jiri and contributors. Provided without
warranty; redistribution and modification are permitted under that license.

Windows x64, macOS (Apple Silicon and Intel), and Linux x64 downloads include the
illustrated manual. Native builds run calculation and offscreen GUI checks;
Windows is also tested locally. See the release notes for platform requirements.
The app's **About / licenses**
button provides the complete license and third-party notices offline.

This software is based in part on the work of the Independent JPEG Group.

## Application guide

Both editing tabs use twelve clockwise ascending fifths:

```text
C -> G -> D -> A -> E -> B -> F#/Gb -> Db/C# -> Ab/G# -> Eb/D# -> Bb -> F -> C
```

C is at the top, F#/Gb at the bottom. Counterclockwise, the left branch follows descending fifths
(ascending fourths modulo octaves). Correction signs always describe clockwise fifths.

## Input layouts and calculation

Both tabs edit the same temperament, with one formula field per fifth.
**Inputs around circle** places compact boxes directly outside the circle.
**Formula list** keeps wide fields on the right for longer expressions, linked by
numbered outside markers. Clicking a marker opens its field for editing.

Both layouts include a **Cents vs equal** bar chart, in chromatic order from C
to B. It sits on the right of Inputs around circle, and between the circle and
the formula fields in Formula list. **Blue bars extend left for flat notes;
pink bars extend right for sharp notes.** The centre line is zero; A stays there.
The symmetric scale adapts to the current tuning and is labelled in cents.
The wider chart includes signed numerical values; hover over any row for its
three-decimal deviation. Both charts follow the current imported or calculated
tuning, including rotations, and clear their bars when no valid chart is available.

The interface adjusts to laptop and desktop windows. Circle and lattice views,
the cent chart, CSV controls and closing actions fit a typical 1366×768 display;
fonts remain at least 22 pixels. Larger windows give the diagrams more room.
Unusually small windows retain scrolling rather than shrinking text.

- **Inputs around circle:** compact boxes for seeing the fractions at their fifths.
- **Formula list:** wide fields for more involved calculations.
- In either tab, name the unit explicitly: `-P/12`, `-S/4`, `schisma/2`, or `-S/4-schisma/4`.
  **schisma = P - S**, so `-S/4-schisma/4` equals `-P/4`. There is no second contribution column.
  For compatibility, bare fractions still default to P in the compact layout and
  S in the list layout; synchronisation preserves the physical correction in both.

Positive corrections widen a pure fifth; negative corrections narrow it; zero
leaves it pure. **Calculate chart** evaluates the formulas and any automatic fields.
A valid circle must have a total correction of **-P**. Invalid or edited inputs
clear the old chart and interval colours until a new calculation succeeds.

**Equal temperament** uses `-P/12` for every fifth (shown as `-1/12` in the compact
layout). **Pure fifths** zeros all corrections and leaves a closure
gap. Edits, presets and imports update both layouts together.

## Interval colours on the circle

The fifth arcs and all thirds use the same purity analysis and colour settings as
the harmony lattice. Major thirds make **four triangles**; minor thirds make
**three squares**. Green, orange and red show increasing distance from the pure
ratios 3:2, 5:4 and 6:5. Fifths have stricter limits than thirds.

Click a tone to thicken its six connected fifths and thirds and dim the remaining
connections. Click it again, or click outside the circle, to clear the selection. This changes only the visual
analysis. Hover over a tone for its outgoing interval errors. Enharmonic pairs
remain visible on circle nodes; the lattice offers chord-specific spelling.

## Closing options

**Close circle...** opens a preview window. Choose one of:

- **Adjust one fifth:** select any of the twelve fifths.
- **Share equally across selected fifths:** tick the fifths that may change.
- **Share equally across all twelve fifths.**
- **Prefer simple fractions (inference):** choose allowed fifths and the maximum
  change per fifth in cents. The calculation favours readable small-number expressions, preserves simple
  fifths and concentrates any more complex remainder in an exceptional fifth.
  A measured tuning can also use small schisma adjustments to a shared base.
  All proposed changes must satisfy the limit; inspect them before applying.
- **Resolve automatic fields:** share the remaining correction only among Auto fields.

**Preview changes** lists the before/after formulas, each fifth's change, the
largest change, and the closure error. **Apply and calculate** installs the
proposal; Cancel leaves the tuning intact. Fixed, unselected formulas are preserved.
Explicit adjustment methods replace any Auto fields with their calculated formulas.

After CSV import, inference fits the original imported fifths, not already rounded
menu fractions. It cannot identify the creator's original intent uniquely.
The exact equal-adjustment options act on the current formula values.
See [reconstruction and closure algorithms](docs/RECONSTRUCTION.md) for search limits.

## Automatic calculation

Choose **Automatic calculation** at the top of any comma dropdown, or type `Auto`.
Click **Calculate chart**, or preview **Resolve automatic fields** in the closing
window. Fixed formulas stay unchanged; all Auto fields receive the same physical
cent correction:

```text
(-P - sum_of_fixed_corrections) / number_of_automatic_fields
```

Four automatic fields with other corrections zero each contribute `-P/4`;
twelve contribute `-P/12`. Auto remains selected so later edits can be balanced again.

The status line shows the Auto field count, cent share,
and nearby fractions of S and P, ordered by closeness. Denominators are
at most 24; `=` means exact within 0.0000001 cents and `≈` means approximate.
Hover over an Auto field for both signed errors, with the closest estimate first. The chart
uses the full-precision Auto value, never the rounded display fraction.
Presets and CSV import replace Auto fields with their own values.

## Harmony lattice

Open the separate **Harmony lattice** tab to explore the current temperament as a triangular
Tonnetz. Horizontal arrows are ascending fifths (3:2), up-right arrows are major thirds (5:4),
and down-right arrows are minor thirds (6:5). Repeated note names represent the same pitch class
with the same tuning. Each triangle forms a major or minor triad.

Choose a root and Major/Minor, click one of the 24 chord buttons, or click a triangle. The selected
chord is outlined and centred in the lattice. Clicking a note selects that root. The detail card
shows the three interval errors in cents; hovering an edge shows the actual interval size, its
pure ratio and the signed error. Positive errors mean wider than pure, negative errors narrower.

Lattice labels and the selected chord's details follow the selected major or minor key: A major
shows A-C#-E, Bb minor shows Bb-Db-F, and F# major includes E#. Other chromatic notes follow the
key's sharp/flat preference. Chord buttons and their tooltips use each chord's own context.
Enharmonic naming changes only labels; all twelve tuned pitch classes stay the same.
Connections use thick strokes and larger arrowheads for easier reading.

Colour is a guide to **distance from pure tuning**, using the absolute cent deviation:

Fifths and thirds have **independent sensitivity controls**. Fifths use stricter limits:

| Intervals | Sensitivity | Green: good | Orange: tempered | Red: rough |
| --- | --- | --- | --- | --- |
| Fifths | Strict | Up to 2 ct | Above 2, up to 6 ct | Above 6 ct |
| Fifths | Standard | Up to 4 ct | Above 4, up to 9 ct | Above 9 ct |
| Both thirds | Strict | Up to 5 ct | Above 5, up to 15 ct | Above 15 ct |
| Both thirds | Standard | Up to 8 ct | Above 8, up to 19 ct | Above 19 ct |

These thresholds are adjustable visual conventions, not universal hearing thresholds. Timbre,
register and voicing also influence perceived roughness. The triangle tint and chord button use
the **worst interval rating under its own limits**: a red fifth keeps a chord red even if a larger
third deviation is only orange. Tooltips also report the largest absolute cent error.
For example, equal temperament has green fifths and orange thirds with Standard sensitivity.
Both `-schisma` (about -1.953721 ct) and `-P/12` (about -1.955001 ct) are green
with either fifth setting, including the inclusive 2-cent Strict limit.
Quarter-comma meantone shows pure major thirds in its favourable keys and red wolf intervals.

The view uses the calculated chart or the original imported CSV values, as labelled above the lattice.
Opening it after an edit attempts a calculation, including Auto fields. Invalid inputs show an empty
state instead of stale colours. An imported chart can be explored even if its rounded comma fractions
do not close. Returning to its editing tab preserves the imported chart and its inputs. Chord selection
and sensitivity change only the visual analysis.

The interval calculation is `100*semitones + deviation[to] - deviation[from]`, with the destination
pitch class wrapped modulo 12, compared to `1200*log2(pure_ratio)`.
Background: [Tonnetz triads](https://archive.bridgesmathart.org/2006/bridges2006-261.pdf) and
[temperament comparison by fifth/third detuning](https://www.huygens-fokker.org/microtonality/temperament.html).

## Hearing the chords

On **Harmony lattice**, enable **Play Chords into Default Audio Output**.
The selected triad plays immediately; selecting a chord, a lattice triangle or
a root plays another two-second root-position triad, ending with a 70 ms fade-out.
The 25 supplied Principal 8 samples (MIDI 48–72, A=440 equal temperament) are
embedded as lossless 24-bit FLAC and repitched by the current chart's cent values.
No external samples, codecs, microphone or audio input are required.

Set **Loudness** from 0 to 100% (default **50%**) to adjust the chord and its reverb
together. Set **Reverb** from 0 to 4 seconds (default **3 seconds**) for dry sound
or a longer church-like decay.
The reverb's wet signal is filtered below 200 Hz and above 2500 Hz; the direct
sound remains unfiltered. Reverb tails may continue after the chord ends.
Rapid selections fade the old notes while starting the new chord. Changed or
invalid charts and leaving the lattice stop the old audition. Playback starts
off and opens only the default output when enabled; disable/re-enable to pick
up a changed system default device. Small windows scroll the chord-button list
while retaining the audio controls at the lower right.

See the [manual](docs/USER_MANUAL.md) and [audio implementation notes](docs/AUDIO.md).

## Expressions and schisma

Comma fields accept decimals, fractions, `+`, `-`, `*`, `/`, unary signs and parentheses.
Entered expressions are retained: `1-1/11` is not rewritten as `10/11` when calculating.
Decimal commas and scientific notation work. Multiplication must be explicit.

| Input | Meaning |
| --- | --- |
| `1-1/11` | 10/11 of the field's comma |
| `-(1-1/11)/2` | -5/11 of the field's comma |
| `schisma` or `H` | One schisma, in either tab |
| `-schisma/2` | Narrow by half a schisma |
| `syntonic` or `S` | One syntonic comma, in either tab |
| `pythagorean` or `P` | One Pythagorean comma, in either tab |
| `-syntonic/4+pythagorean/12` | Combine the two named physical corrections |
| `ET` | The correction from a pure fifth to a 700-cent fifth |

Bare numbers use the tab's default unit. Named commas retain their physical cent values. In a syntonic
field, `-1-schisma` or `-1-H` therefore equals one negative Pythagorean comma. `H` remains a backwards-compatible alias for `schisma`; generated expressions spell out the name. The musical note called H in Czech/German is labelled B.

Menus include zero and positive and negative fractions of P, S and schisma with denominators
1, 2, 3, 4, 5, 6, 12 and **24**. For example, `-P/12` gives equal temperament and
`-P/4+schisma/4` is one negative quarter syntonic comma.

Expressions are limited to 512 characters and nesting depth 32. Invalid syntax, unknown names,
division by zero, nonfinite values and corrections exceeding 1000 commas in magnitude are rejected.
Long fields scroll while editing; each field's large tooltip shows the complete expression.

## CSV and reverse calculation

Table and CSV use chromatic order `C;C#;D;D#;E;F;F#;G;G#;A;A#;B`. Output values are deviations
from equal temperament, normalised to **A = 0.000 cents** and rounded to three decimals.
Use **Copy CSV** or select the output text. Positive note deviations are sharp; negative ones are flat.

To reverse a chart, select the comma tab, paste twelve semicolon-separated numeric cent values into
**CSV in**, and click **Import CSV** or press Enter. Import updates both circle layouts.
A nonzero input A is subtracted from all twelve notes.

```text
fraction = (700 + deviation[next] - deviation[current] - pure_fifth_cents) / selected_comma_cents
```

Select the reconstruction method before importing:

- **Nearest fractions:** independently choose the closest menu correction in
  cents, including P, S and schisma fractions and ±1/24, plus multiples by 2, 3 and 4. Ties favour smaller magnitude,
  then menu order. Native fractions appear directly as `-P/4` or `-S/4` in the
  list layout, using their native comma. `P*2/6` simplifies to `P/3`; `S*3/4` remains explicit.
  Generated formulas put the comma first, as in `P*3/5`; either input order is accepted.
- **Precise simple expressions:** prefer small fractions and combinations of two
  comma terms. Once a candidate is within CSV rounding uncertainty, simplicity
  takes priority over another meaningless decimal place. Ordinary tuning
  corrections use numerators up to 12 and denominators up to 24. The displayed
  formula keeps those small terms instead of expanding them into large numbers.

Three-decimal CSV has a ±0.0005-cent rounding interval per note. A fifth is the
difference of two note deviations, so its uncertainty can reach ±0.001 cents.
An adequate small expression is labelled **Within CSV precision**. It is an
interpretation of the data, not a proof of its original spelling.

For the catalogue's quarter-comma meantone, precise reconstruction gives eleven
`-S/4` fields and `S*7/4-schisma` for the Ab–Eb wolf. The wolf equals `S*11/4-P`,
so those formulas close exactly. Tiny rounding differences do not contaminate
the other eleven fifths with elaborate expressions.

The status reports the largest fifth error. Each field's tooltip gives its signed
error (formula minus CSV-derived correction): normal text means exact within
0.0000001 cents, **amber** means an error at most 0.001 cents, and **red** means more.
For example, P/24 and schisma/2 differ by only about 0.00064 cents. Rounded CSV may select
either; it does not reliably encode the original comma spelling.

The chart, CSV output and interval colours retain the **imported values**, normalised
to A, until forward calculation succeeds. Independently fitted expressions may not
close; the app reports their closure error without silently changing a fifth.
Use **Close circle...** to preview a closed tuning. Invalid CSV leaves the existing
circle and chart intact. Editing a formula invalidates stale output.

Both editing tabs show one working temperament. Switching tabs preserves the
chart and CSV; bare fractions are converted to retain their physical comma unit.
**CSV in always retains the user's original pasted text**, including its original
A reference and formatting. Calculations, failed closure, edits, presets and
rotation never clear or overwrite it. Only editing or pasting into that field
changes it. **CSV out** updates with the current A-normalised result, or clears
when the result is invalid, so the two rows can be compared directly.

Use **Rotate CW** or **Rotate CCW** beside the circle to move all formulas,
including Auto, by one fifth. Clockwise moves C–G's correction to G–D;
counter-clockwise reverses this. Imported charts rotate without re-quantisation,
and calculated charts update from the rotated formulas, always with A = 0.
**Close circle** and **Calculate chart** occupy the two lower circle corners.

## Known temperament suggestions

The supplied [temperaments.csv](Source/temperaments.csv) is embedded in the
executable at build time; no separate CSV file is needed to run the app.
After import or calculation, the app compares the A-normalised chart with all
31 catalogue entries and their twelve rotations. Nearby matches must be within 1 cent
at every note. The suggestion shows the name, rotation in fifths, and maximum difference.
Values within 0.001 cents are labelled a catalogue match; other qualifying
results are labelled near a known temperament.

Aliases are retained: equal temperament also matches the catalogue's Neidhardt
Hof entry. **Suggestions** opens a large-text, scrollable list. All close names
and distinct rotations come first, sorted by maximum error and then RMS error.
The best harmonic alternatives bring the list to at least five distinct names,
using each alternative's best rotation. Identical symmetric rotations appear
only once per name. Catalogue comments accompany every result.

Alternatives compare all twelve fifths, major thirds and minor thirds. The
harmonic distance is the square root of half the squared fifth RMS difference,
plus a quarter of each squared third RMS difference, in cents. Lower is closer.
This recognises related interval patterns despite a larger difference at one
note; it retains the size of the differences rather than matching shape alone.
The interval comparison is independent of tuning reference and treats rotated
patterns consistently. The list also reports maximum and RMS A-relative note
differences and separate RMS differences for each interval family.
**Compare** labels are alternatives, not claimed identities; the nearest
available examples can still be distant. A suggestion never changes the tuning.
The 1-cent near-match allowance keeps Trost recognisable after nearest-fraction
import and Auto closure (up to about 0.53 cents difference in the original position).
That is reported as a near match, with the actual difference, rather than exact identity.
The supplied Rousseau variant suggests Ordinaire alongside Schlick and a rotated
Rameau even though its maximum note difference from Ordinaire is 6.35 cents.

## Mathematics

```text
P = 1200 * log2(531441/524288)       approximately 23.460010385 cents
S = 1200 * log2(81/80)              approximately 21.506289597 cents
H = P - S                         approximately  1.953720788 cents
pure_fifth = 1200 * log2(3/2)
```

The schisma is Pythagorean minus syntonic and has frequency ratio 32805/32768.
References: [Huygens-Fokker interval ratios](https://www.huygens-fokker.org/docs/intervals.html),
[comma terminology and logarithmic measures](https://www.huygens-fokker.org/docs/measures.html).

A row's fifth is `pure_fifth + the formula's physical cent correction`.
Each clockwise step satisfies `next_deviation - current_deviation = fifth - 700`.
The circle's total physical correction must equal **-P**. Pythagorean fractions therefore sum to -1;
syntonic fractions sum to **-P/S**, represented by `-1-H` in syntonic units. Closure tolerance is
0.000001 cents. Calculations use double precision; chart and CSV alone are rounded to three decimals.
Exact closure uses symbolic comma coefficients when possible; nonlinear formulas can require a numerical correction.

The calculator works without an audio device; optional audition needs an output.
No external sample assets or network services are required. JUCE retains its own licensing terms.
