# Temperament Generator 1.4.2

Desktop tools for designing twelve-note temperaments, converting comma corrections
to tuning charts, and exploring fifths and thirds in a harmony lattice.

- [Build on Windows or Linux](docs/BUILDING.md)
- [Project website](https://zurek-jiri.github.io/temperament-generator/)
- [Downloads and corresponding source](https://github.com/zurek-jiri/temperament-generator/releases)
- [Release history](CHANGELOG.md) · [Contributing](CONTRIBUTING.md)
- [First public release checklist](docs/PUBLISHING.md) · [Third-party notices](THIRD_PARTY_NOTICES.md)

Maintained by [zurek-jiri](https://github.com/zurek-jiri). Free and open source under
[AGPLv3](LICENSE). Copyright (C) 2026 zurek-jiri and contributors. Provided without
warranty; redistribution and modification are permitted under that license.

Windows builds are tested locally. Linux builds are checked by CI; a Linux desktop
release still requires interactive validation. The app's **About / licenses**
button provides the complete license and third-party notices offline.

This software is based in part on the work of the Independent JPEG Group.

## Application guide

Both editing tabs use twelve clockwise ascending fifths:

```text
C -> G -> D -> A -> E -> B -> F#/Gb -> Db/C# -> Ab/G# -> Eb/D# -> Bb -> F -> C
```

C is at the top, F#/Gb at the bottom. Counterclockwise, the left branch follows descending fifths
(ascending fourths modulo octaves). Correction signs always describe clockwise fifths.

## Modes and calculation

- **Pythagorean / fifths:** enter Pythagorean-comma corrections in the circle's boxes.
- **Syntonic / fifths:** the numbered edges correspond to the adjacent table. Enter syntonic
  and Pythagorean comma contributions side by side. **The two contributions add.** The resulting
  cent-deviation chart appears below the circle and input table.

Positive corrections widen a pure fifth; negative corrections narrow it; zero leaves it pure.
**Calculate chart** uses the entered intervals. **Close circles** keeps eleven fifths and adjusts
F#/Gb-to-Db/C#. In paired mode a recognised menu fraction goes in its own comma column, with
zero in the other field; other closing corrections use a syntonic expression. Inconsistent circles
report a closure error.

**Equal temperament** resets all notes to zero deviation, using `-1/12` in the Pythagorean column
and zero in the syntonic column. **Pure intervals** zeros all contributions;
the resulting circle requires a closing wolf fifth. Switching tabs preserves each tab's inputs.

## Automatic calculation

Choose **Automatic calculation** at the top of any comma dropdown, or type `Auto`.
The field displays **Auto** in green at the normal large font size. Click **Calculate chart**
or **Close circles** to share the remaining correction equally in cents among all automatic fields.
Fixed expressions stay unchanged, including the usual closing fifth. With no automatic fields,
the existing calculation and closing behaviour applies.

The split is over **fields** in both columns. Each selected field receives
`(-P - sum_of_fixed_corrections_in_cents) / number_of_automatic_fields` cents, expressed using
that field's comma. If both fields in one fifth are automatic, that fifth receives two shares.
The share can be negative, positive or zero. Four automatic Pythagorean fields with all other
corrections zero each contribute `-1/4`; twelve contribute `-1/12` and give equal temperament.

Auto remains selected after calculation, so later edits can be balanced again. The circle centre
shows the number of automatic fields, the cent share, and its nearest fractions of both syntonic
(diatonic) and Pythagorean (ditonic) comma. The closest ratio is highlighted green. Fractions are
reduced, with denominators up to 24; the comparison uses their absolute cent error. Equal errors
prefer a smaller denominator, then the active tab's comma. An `=` marks an exact match within
0.0000001 cents; an approximation uses `≈`. For example, `-5.865002596...` cents appears as
`= -1/4 P` and `≈ -3/11 S`, with the Pythagorean ratio highlighted.

Hover over an Auto field to see both ratios, the closest comma and signed approximation errors
in the large tooltip. Calculation retains full precision so the displayed approximations do not
alter the chart or circle closure. Selecting a number or expression replaces that field's Auto mode.
Presets and CSV import replace Auto selections with their own values.

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
| Fifths | Strict | Up to 1 ct | Above 1, up to 4 ct | Above 4 ct |
| Fifths | Standard | Up to 2 ct | Above 2, up to 6 ct | Above 6 ct |
| Fifths | Gentle | Up to 4 ct | Above 4, up to 10 ct | Above 10 ct |
| Both thirds | Strict | Up to 5 ct | Above 5, up to 15 ct | Above 15 ct |
| Both thirds | Standard | Up to 8 ct | Above 8, up to 22 ct | Above 22 ct |
| Both thirds | Gentle | Up to 12 ct | Above 12, up to 30 ct | Above 30 ct |

These thresholds are adjustable visual conventions, not universal hearing thresholds. Timbre,
register and voicing also influence perceived roughness. The triangle tint and chord button use
the **worst interval rating under its own limits**: a red fifth keeps a chord red even if a larger
third deviation is only orange. Tooltips also report the largest absolute cent error.
For example, equal temperament has green fifths and orange thirds with Standard sensitivity;
quarter-comma meantone shows pure major thirds in its favourable keys and red wolf intervals.

The view uses the calculated chart or the original imported CSV values, as labelled above the lattice.
Opening it after an edit attempts a calculation, including Auto fields. Invalid inputs show an empty
state instead of stale colours. An imported chart can be explored even if its rounded comma fractions
do not close. Returning to its editing tab preserves the imported chart and its inputs. Chord selection
and sensitivity change only the visual analysis.

The interval calculation is `100*semitones + deviation[to] - deviation[from]`, with the destination
pitch class wrapped modulo 12, compared to `1200*log2(pure_ratio)`.
Background: [Tonnetz triads](https://archive.bridgesmathart.org/2006/bridges2006-261.pdf) and
[temperament comparison by fifth/third detuning](https://www.huygens-fokker.org/microtonality/temperament.html).

## Expressions and schisma

Comma fields accept decimals, fractions, `+`, `-`, `*`, `/`, unary signs and parentheses.
Entered expressions are retained: `1-1/11` is not rewritten as `10/11` when calculating.
Decimal commas and scientific notation work. Multiplication must be explicit.

| Input | Meaning |
| --- | --- |
| `1-1/11` | 10/11 of the field's comma |
| `-(1-1/11)/2` | -5/11 of the field's comma |
| `schisma` or `H` | One schisma, in either column |
| `-H/2` | Narrow by half a schisma |
| `syntonic` or `S` | One syntonic comma, in either column |
| `pythagorean` or `P` | One Pythagorean comma, in either column |
| `-syntonic/4+pythagorean/12` | Combine the two named physical corrections |
| `ET` | The correction from a pure fifth to a 700-cent fifth |

Bare numbers use the column's unit. Named commas retain their physical cent values. In a syntonic
field, `-1-schisma` or `-1-H` therefore equals one negative Pythagorean comma. `H` is an expression
variable; the musical note called H in Czech/German is labelled B.

Menus include zero, positive and negative 1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/12, schisma fractions,
and exact expressions for fractions of the other comma. For example, `-1/12-H/12` in syntonic units
gives equal temperament; `-1/4+H/4` in Pythagorean units is one negative quarter syntonic comma.

Expressions are limited to 512 characters and nesting depth 32. Invalid syntax, unknown names,
division by zero, nonfinite values and corrections exceeding 1000 commas in magnitude are rejected.
Long fields scroll while editing; each field's large tooltip shows the complete expression.

## CSV and reverse calculation

Table and CSV use chromatic order `C;C#;D;D#;E;F;F#;G;G#;A;A#;B`. Output values are deviations
from equal temperament, normalised to **A = 0.000 cents** and rounded to three decimals.
Use **Copy CSV** or select the output text. Positive note deviations are sharp; negative ones are flat.

To reverse a chart, select the comma tab, paste twelve semicolon-separated numeric cent values into
**CSV input**, and click **CSV to fifths** or press Enter. Import stays in the selected mode.
A nonzero input A is subtracted from all twelve notes.

```text
fraction = (700 + deviation[next] - deviation[current] - pure_fifth_cents) / selected_comma_cents
```

Each result is independently snapped to the nearest menu expression, including schisma expressions.
The boxes show readable fractions or expressions rather than raw decimal ratios. Ties favour smaller
magnitude, then menu order.

- Normal text: exact within numerical tolerance.
- **Amber:** interval rounding no greater than 0.001 cents, consistent with three-decimal CSV.
- **Red:** the nearest menu expression differs by more than 0.001 cents.

In paired mode, import uses the **native comma column** for each recognised simple fraction.
Because `P = S + H`, a correction formerly displayed as `-1/4-H/4` in the syntonic column now
appears as **Syntonic: `0`, Pythagorean: `-1/4`**. A quarter syntonic comma appears as
**Syntonic: `-1/4`, Pythagorean: `0`**. Schisma-only corrections retain an explicit `H` expression.
Rounding colour and the error tooltip appear on the field containing the recovered correction.
Import selects the nearest menu correction by its physical cent value; it does not recover the
original spelling or an arbitrary split between contributions.

The chart and output remain labelled **imported values** until forward calculation succeeds.
Independent rounding may prevent closure: the app reports this without changing another interval.
Edit expressions or use **Close circles** to calculate a new chart. Invalid CSV preserves the existing
circle and chart. Editing circle inputs invalidates stale output.

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

A paired row's fifth is `pure_fifth + syntonic_fraction*S + pythagorean_fraction*P`.
Each clockwise step satisfies `next_deviation - current_deviation = fifth - 700`.
The circle's total physical correction must equal **-P**. Pythagorean fractions therefore sum to -1;
syntonic fractions sum to **-P/S**, represented by `-1-H` in syntonic units. Closure tolerance is
0.000001 cents. Calculations use double precision; chart and CSV alone are rounded to three decimals.
Automatic closure prefers rational or rational-plus-schisma expressions where possible.

No audio devices, external assets or network services are required. JUCE retains its own licensing terms.
