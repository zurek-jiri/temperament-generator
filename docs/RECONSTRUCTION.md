# Reconstruction, recognition and closure

A fifth correction from CSV is
`700 + deviation[next] - deviation[current] - 1200*log2(3/2)` cents.
Twelve corrections must sum to `-P`. The identity `schisma = P - S` leaves
two independent comma coefficients. H remains an input alias for schisma.

## Nearest fractions

Nearest mode compares physical cent values against zero and positive/negative
P, S and schisma fractions with denominators 1, 2, 3, 4, 5, 6, 12 and 24.
It also tries multiples by 2, 3 and 4, reducing the fraction: `P*2/6` becomes
`P/3`, while `S*3/4` remains a small multiple. Generated formulas place the comma
name before the numerator; the parser accepts both orders. The input menu stays short.
Equal-error choices prefer smaller physical magnitude, then candidate order.

## Precise simple expressions

Three-decimal CSV has a rounding half-width of 0.0005 cents per note. A fifth
subtracts two note deviations, so up to 0.001 cents of interval error is consistent
with that precision. The algorithm treats a candidate within that interval as
sufficiently accurate, then chooses the simpler expression rather than chasing
unjustified decimal places.

Candidates include single fractions and the independent pairs P/S, P/schisma,
and S/schisma. Numerators are limited to 12 in magnitude and denominators to 24
for ordinary tuning corrections. Each first coefficient is searched around
zero and around the target divided by its comma; the second is rounded to the
nearest allowed fraction of the remainder. Unusually large corrections above
12 Pythagorean commas may require larger single-term numerators.

Complexity per reduced term is `30 + 4*abs(numerator) + 2*ceil(log2(denominator))`,
with 12 extra for a schisma term. This favours fewer terms, small numerators and
familiar P/S fractions. Within the CSV error interval, complexity wins and error
breaks ties. If no candidate is within it, the best numerical candidate is
reported honestly as an approximation. The original small summands are kept in
the output instead of expanding them into enormous combined numerators.

The supplied meantone chart recovers eleven `-S/4` fifths and
`S*7/4-schisma` for Ab–Eb. That wolf is exactly `S*11/4-P`, so the formulas
close without adding tiny compensating terms to the eleven ordinary fifths.

Both import modes preserve the imported A-normalised chart until the user
calculates or applies a closure proposal. Formula errors and closure residuals
are shown separately from those imported note values.

## Exact and automatic closing

The parser carries symbolic P/S coefficients alongside numerical values. Linear
expressions such as `-S/4-schisma/4` reduce to `-P/4`. One/selected/all closing
subtracts the sum of those coefficients from `-P` and shares the residual in
cents across the eligible fifths. Fixed, unselected formulas keep their spelling.

Nonlinear arithmetic remains valid numerical input, but can require numerical
closure rather than symbolic simplification. Simple fitting requires linear
comma expressions. Auto fields are first resolved in cents. The Auto method
keeps them persistent; explicit adjustment methods materialise them so they
cannot silently move an unselected fifth after a neighbouring edit.

## Prefer simple fractions

The inference target is the imported CSV when available, otherwise the current
evaluated formulas. The change limit applies to each eligible fifth, not to
accumulated note displacement around the circle. It must be positive and at
most 100 cents. The search proceeds in this order:

1. Choose readable expressions within CSV precision where possible. If they
   already close, keep them. Otherwise, try putting the exact remaining correction
   into one eligible fifth, protecting the simplest ordinary fifths first.
   Prefer a short closing expression; repeated contributions can be grouped.
2. For a fully adjustable circle, try a shared `-P/12` base with small multiples
   of schisma fractions. A dynamic programme requires their multiples to sum to
   zero and keeps every fifth inside the limit. This provides readable closed
   approximations for measured, nearly equal tunings without huge cancelling
   numerators.
3. Search combinations of small rational P/S coefficients on the exact integer
   grid `lcm(1,...,24) = 5354228880`. Retain 48 locally scored candidates plus
   eligible native menu fractions. Deduplicate equivalent coefficient sums,
   prune using remaining coefficient and cent bounds, and keep at most 4,000
   states per step. The score balances squared cent error and complexity; its
   ranking includes a lower bound on the error still needed for closure.
4. If necessary, allow a longer grouped expression in just the exceptional
   closing field, subject to the same change limit and the parser's 512-character
   maximum. If no proposal meets the constraints, report that rather than apply
   an unclosed or out-of-limit result.

These are bounded heuristics, not historical identification or a claim of global
optimality. Every proposal is reparsed and checked by the ordinary forward
calculation. It must close within 0.000001 cents, respect the change limit, and
stay within the application's 1000-comma correction bound. The preview reports
all before/after formulas and actual fifth changes; only Apply changes the tuning.

## Embedded catalogue

CMake embeds `Source/temperaments.csv` in a generated C++ header and tracks the
CSV as a configure dependency. The executable does not read an external CSV.
Names, note deviations and supplied comments are preserved. Recognition checks
all twelve rotations of each entry after normalising both charts to A.

A close match must be within 1 cent at every note. The UI reports the maximum
difference, the rotation in fifths and alternatives, including aliases. Up to 0.001 cents
is labelled Match; larger differences up to 1 cent are labelled Near.
Suggestions are independent of expression reconstruction and never replace the
user's tuning or claim that similar data uniquely identifies its author.
The wider near-match tolerance accommodates accumulated nearest-fraction error:
Trost's one-decimal catalogue CSV can differ by 0.53 cents after Auto closure.
The Suggestions window first lists every close name and distinct rotation, ordered
by maximum note difference and then RMS difference. Identical rotated charts of
the same entry are deduplicated; different catalogue names are retained.

Beyond this limit, alternatives are ranked by the harmonic distance
`sqrt(0.5 * fifthRms^2 + 0.25 * majorThirdRms^2 + 0.25 * minorThirdRms^2)`.
Each RMS compares all twelve intervals of its family in cents between the
input and the rotated catalogue entry. This preserves absolute interval sizes,
is independent of a common pitch offset, and gives fifths twice the weight of
each third family. It is a transparent comparison heuristic, not a probability
of historical identity or a perceptual hearing threshold.

Add the best rotation of each remaining catalogue name until at least five
distinct names are represented. No distance cutoff hides these alternatives;
they are labelled Compare and may be distant. Their note maximum/RMS and three
interval RMS differences remain visible. All close names/rotations survive even
when they already exceed five entries. Ties use note maximum error, catalogue
order, then rotation order. Nonfinite input or invalid tolerances yield no results.

The supplied Rousseau variant suggests Ordinaire in every rotation: its fifth
RMS difference is about 1.869 cents and harmonic distance about 2.700 cents,
although the maximum A-relative note difference in the original position is
6.350 cents. Other close harmonic alternatives include Schlick and rotated Rameau.

## Shared editing state and rotation

The Inputs around circle and Formula list layouts share one chart. Changes synchronise formulas to the other tab
using their physical comma amounts, preserving Auto and imported rounding metadata.
Bare and mixed linear expressions are converted symbolically; unusual nonlinear
expressions use full-precision numerical conversion. Invalid expressions clear the
corresponding other field and invalidate both charts and CSV output. Merely changing
tabs does not re-import or re-quantise the tuning.

CSV in is user-owned reference text. No calculation, failed closure, expression
edit, preset, rotation or view change clears or overwrites it. Only the user
changes that editor. Import reads it without changing the original values,
reference or formatting; normalisation is applied only to the working chart and
CSV out. This keeps the original row available for direct comparison with output.

Clockwise rotation moves edge `i` to `(i+1)%12`, including Auto and its rounding
metadata. Chromatic note deviations move by seven semitones and are normalised
back to A = 0. Imported charts retain the rotated input deviations until Calculate;
other charts are recalculated. Twelve steps restore the original formulas.

## Circle geometry and layout

Clockwise circle indices follow `C,G,D,A,E,B,F#,C#,Ab,Eb,Bb,F`.
Fifths join index `i` to `i+1`, major thirds to `i+4`, minor thirds to `i+9`,
modulo 12: a ring, four triangles and three squares. All 36 intervals share
the lattice's purity analysis and independent fifth/third sensitivities.

Compact inputs sit outside the ring. The Formula list layout uses a long table
linked to outside markers. Markers are moved away from enharmonic note labels
when necessary. Clicking a tone highlights its incident connections; clicking
outside clears the selection without changing calculations. Layout checks cover
1280×660, 1320×660, 1320×680, 1320×800 and 1480×900 content areas, including all editing and lattice
controls. Smaller-than-supported windows retain scrolling rather than shrink text.

`CentDeviationView` receives the same note deviations and validity as the result
table and harmony analysis. Its twelve chromatic rows plot negative deviations
to the left in blue and positive deviations to the right in pink. A single
symmetric extent covers every row, using readable 1, 2, 2.5, 5 and 10 multiples
of powers of ten, with a minimum extent of one cent. Both layout instances use
the same scale. The wide chart also displays the three-decimal values; the
narrow chart keeps more space for bars and provides values in row tooltips.

The compact layout places this chart to the right of the circle. Formula list
places it between the circle and the fifth/formula columns, keeping formula
fields at least 300 pixels wide at the minimum supported window size. Invalid
or edited tuning data removes stale bars. GUI checks compare both charts to the
result table and verify left/right rendering using the supplied meantone tuning.
