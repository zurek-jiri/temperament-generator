# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
# See LICENSE and COPYRIGHT for terms and the no-warranty notice.

"""Portable Windows/Linux build, mathematical tests, and GUI smoke checks."""
import argparse
import os
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", default="Builds/ci")
    parser.add_argument("--preview-dir", default="Preview/ci")
    parser.add_argument("--juce-path", help="Optional local JUCE checkout instead of CMake's download")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    build = (root / args.build_dir).resolve()
    preview = (root / args.preview_dir).resolve()
    environment = dict(os.environ)
    if sys.platform == "win32":
        environment = {key.upper(): value for key, value in environment.items()}
    elif not sys.platform.startswith("linux"):
        parser.error("This build helper currently targets Windows and Linux.")

    def run(command, timeout=None):
        print("Running: " + repr(command), flush=True)
        subprocess.run(command, cwd=str(root), env=environment, check=True, timeout=timeout)

    configure = ["cmake", "-S", str(root), "-B", str(build)]
    configure += ["-A", "x64"] if sys.platform == "win32" else ["-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"]
    if args.juce_path:
        configure += ["-DJUCE_PATH=" + str(Path(args.juce_path).resolve())]
    run(configure)
    run(["cmake", "--build", str(build), "--config", "Release", "--parallel", "2"])
    run(["ctest", "--test-dir", str(build), "-C", "Release", "--output-on-failure"])
    suffix = ".exe" if sys.platform == "win32" else ""
    app = build / "TemperamentGenerator_artefacts" / "Release" / ("Temperament Generator" + suffix)
    command = [str(app), "--render-preview", str(preview)]
    if sys.platform.startswith("linux"):
        command = ["xvfb-run", "-a"] + command
    run(command, timeout=90)


if __name__ == "__main__":
    main()
