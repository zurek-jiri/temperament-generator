# Principal 8' sample bank

25 mono recordings supplied by the project maintainer, MIDI 48–72, in equal
temperament at A = 440 Hz. These assets are distributed as part of this project
under AGPL-3.0-only; see the root LICENSE and THIRD_PARTY_NOTICES.md.

Each FLAC preserves every sample of the original 24-bit, 48 kHz PCM recording.
`manifest.json` records original names, frame counts, PCM and FLAC SHA-256 hashes,
and original forward-loop endpoints (end exclusive). `SampleMetadata.h` provides
the same playback metadata in the executable. The primary `069-A.wav` is used;
the alternate `069-A_a.wav` is excluded.

Builds embed the checked-in FLACs directly and require no conversion tools.
To regenerate from the maintainer's original `Principal8/` directory, install
FFmpeg and run `python Tools/prepare_samples.py`. It decodes each generated FLAC
back to PCM and verifies byte-for-byte equality. Original WAV files and FFmpeg
are not needed by users of the application.
