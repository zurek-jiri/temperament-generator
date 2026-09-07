# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
# See LICENSE and COPYRIGHT for terms and the no-warranty notice.

"""Preserve dependency license texts from the exact JUCE source used to build."""
import argparse
from pathlib import Path
import shutil

FILES = {
    "JUCE-LICENSE.md": "LICENSE.md",
    "HarfBuzz-COPYING.txt": "modules/juce_graphics/fonts/harfbuzz/COPYING",
    "SheenBidi-LICENSE.txt": "modules/juce_graphics/unicode/sheenbidi/LICENSE",
    "Independent-JPEG-Group-README.txt": "modules/juce_graphics/image_formats/jpglib/README",
    "libpng-LICENSE.txt": "modules/juce_graphics/image_formats/pnglib/LICENSE",
    "zlib-LICENSE.txt": "modules/juce_core/zip/zlib/LICENSE",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--juce-path", required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    juce = Path(args.juce_path).resolve()
    output = root / "Licenses"
    output.mkdir(exist_ok=True)
    parts = ["THIRD-PARTY NOTICES — Temperament Generator\n\n"
             "JUCE 8.0.12 is used under AGPLv3. JUCE copyright notices remain in its source.\n"
             "SheenBidi: Copyright (C) 2014-2025 Muhammad Tayyab Akram.\n"
             "This software is based in part on the work of the Independent JPEG Group.\n\n"
             "The following license documents are reproduced from the JUCE distribution.\n"
             "The release source archive preserves the original source and notices in full.\n"]
    for name, relative in FILES.items():
        source = juce / relative
        shutil.copy2(source, output / name)
        parts.append("\n\n" + "=" * 72 + "\n" + name + "\nSource: JUCE/" + relative + "\n\n"
                     + source.read_text(encoding="utf-8"))
    (output / "ThirdPartyNotices.txt").write_text("".join(parts), encoding="utf-8")
    print("Preserved " + str(len(FILES)) + " dependency license documents and combined offline notices.")


if __name__ == "__main__":
    main()
