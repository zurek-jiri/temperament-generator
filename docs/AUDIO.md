# Sampled chord playback

JUCE 8.0.12 supplies audio-device access and FLAC decoding. The renderer itself
is ordinary C++17 and is tested without a device or GUI. Device inputs are always
zero. Playback starts selected, and the default output opens lazily on the first
chord selection in the visible lattice. Startup and offscreen previews stay silent.

## Recordings and tuning

`Source/PrincipalSamples/` holds 25 lossless 24-bit, 48 kHz mono recordings,
provided in equal temperament at A=440. CMake embeds them using binary data.
On first audition, they are decoded into immutable floating-point sample buffers.
The original forward-loop metadata is preserved for large upward pitch shifts;
normal two-second auditions use the recordings' beginnings.

For each selected root-position triad, MIDI notes are `48+root`, plus 3 or 4,
and plus 7. The source-position increment is

```
sourceSampleRate / deviceSampleRate * 2^((deviation[note] - deviation[A]) / 1200)
```

Four-point cubic interpolation repitches the recordings during rendering. A
10 ms attack and a final 70 ms linear fade are included in each two-second note.
Old voices release over 70 ms when retriggered. A fixed 64-voice pool bounds
rapid-selection work, and final output is limited to +/-0.98 for headroom.

The master loudness defaults to 50% and ranges from 0 to 100%. It scales the
combined direct signal and reverb before the output limiter. Changes are
smoothed over approximately 20 ms to prevent clicks.

## Reverb

An eight-line feedback delay network uses unequal 31–72 ms delays and an
orthogonal Hadamard mixing matrix for stereo diffusion. Per-line gain is
`10^(-3 * delaySeconds / decaySeconds)`, setting approximate RT60 in the requested
0–4 second range. Feedback gains and the dry/wet switch are smoothed over about
20 ms when adjusted. The UI defaults to a three-second decay. The wet return level
is fixed relative to the direct signal; zero seconds disables its contribution.

Second-order high-pass (200 Hz) and low-pass (2500 Hz) filters precede the network
and filter its stereo returns. The direct samples bypass these filters. This is
a compact synthetic room, not a measured church impulse response. The direct
chord ends at two seconds; the tail can continue, with all state cleared by eight
seconds after the latest selection. Stopping a chart audition fades the entire
output over 70 ms, then clears voices and reverb.

## Device and thread behaviour

The message thread decodes samples and sends small chord requests through a
bounded single-producer/single-consumer queue. The audio callback consumes the
latest pending selection, avoiding a backlog of obsolete chords. It performs no
file reads, decoding, allocation or GUI access. Delay buffers are allocated in
device preparation. Reverb and loudness settings use atomic values. Device shutdown removes
the callback before destroying its state; errors are relayed to the UI asynchronously.

The UI retains the enabled preference when changing tabs, but stops the old
audition. Unchecking closes the device. Re-enabling asks for the current default
output again when auditioning a valid chart. A device failure leaves the visual application usable.

## Validation

`ChordRendererTests` covers all 24 triad voicings, pitch ratios and actual rendered
frequency at 44.1/48/96 kHz, A-reference invariance, two-second duration, the 70 ms
fade, stereo decay, the two frequency cutoffs, stop behaviour, rapid retriggering
and invalid values, plus loudness scaling and muting. The GUI preview route additionally decodes the complete
embedded bank and writes dry/reverberant quarter-comma meantone audition WAVs
into the ignored Preview directory. These checks run on all release platforms.

For an explicit hardware check, launch the application with
`--check-default-audio /path/to/report-folder`. This plays one two-second C major
chord, verifies that output callbacks carry nonzero samples and inputs remain
closed, writes `default-audio-check.txt`, then exits. It is deliberately separate
from automated CI, where a sound device need not exist.
