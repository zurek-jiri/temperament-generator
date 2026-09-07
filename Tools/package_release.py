# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
# See LICENSE and COPYRIGHT for terms and the no-warranty notice.
"""Create reviewed Windows and corresponding-source ZIPs; never uploads files."""
import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--juce-path", required=True, help="Exact JUCE source used by the compiler")
    parser.add_argument("--git", default=shutil.which("git"))
    parser.add_argument("--output", default="dist")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    if not args.git:
        parser.error("Git is required; install it or pass --git /path/to/git.")

    def git(*command):
        return subprocess.check_output([args.git, "-C", str(root), *command]).decode("utf-8")

    if git("status", "--porcelain", "--untracked-files=no").strip():
        parser.error("Commit the reviewed source before packaging; tracked files have changes.")
    paths = [Path(p) for p in git("ls-files", "-z").split("\0") if p]
    blocked = {".git", ".local-tools", "Preview", "Builds", "_site", "dist", "codex_session.txt", "TunFor.png"}
    for path in paths:
        if any(part in blocked or part.startswith(".env") for part in path.parts) or path.suffix.lower() == ".exe":
            parser.error("Private or generated file is tracked: " + str(path))
    version = re.search(r"project\(TemperamentGenerator VERSION (\d+\.\d+\.\d+)", (root / "CMakeLists.txt").read_text()).group(1)
    commit = git("rev-parse", "HEAD").strip()
    build = (root / args.build_dir).resolve()
    binary = build / "TemperamentGenerator_artefacts" / "Release" / "Temperament Generator.exe"
    if not binary.is_file():
        parser.error("Built Windows application was not found.")
    juce = Path(args.juce_path).resolve()
    for required in ("CMakeLists.txt", "LICENSE.md", "modules/juce_core/juce_core.h"):
        if not (juce / required).is_file():
            parser.error("Incomplete JUCE source: " + required)
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    configured = re.search(r"^JUCE_PATH:PATH=(.+)$", cache, flags=re.MULTILINE)
    if not configured or Path(configured.group(1).strip()).resolve() != juce:
        parser.error("--juce-path must match the JUCE_PATH recorded in the build's CMakeCache.txt.")
    destination = (root / args.output).resolve()
    if root not in destination.parents:
        parser.error("Output must stay inside this project.")
    destination.mkdir(parents=True, exist_ok=True)
    windows_zip = destination / ("TemperamentGenerator-" + version + "-windows-x64.zip")
    source_zip = destination / ("TemperamentGenerator-" + version + "-source.zip")
    sums = destination / "SHA256SUMS.txt"
    if any(path.exists() for path in (windows_zip, source_zip, sums)):
        parser.error("Output already exists; choose a new output folder rather than replacing release files.")
    source_note = ("Temperament Generator " + version + "\nSource commit: " + commit + "\n\n"
                   "Complete corresponding source (including JUCE) is available beside this binary at:\n"
                   "https://github.com/zurek-jiri/temperament-generator/releases/tag/v" + version + "\n"
                   "Source asset: " + source_zip.name + "\n\n"
                   "License: AGPLv3, version 3 only. No warranty. See LICENSE and COPYRIGHT.\n")
    prefix = "TemperamentGenerator-" + version
    with zipfile.ZipFile(windows_zip, "w", zipfile.ZIP_DEFLATED) as package:
        package.write(binary, prefix + "/Temperament Generator.exe")
        for path in paths:
            if path.parts[0] in {"Licenses", "docs"} or str(path) in {"README.md", "LICENSE", "COPYRIGHT", "THIRD_PARTY_NOTICES.md", "CHANGELOG.md", "CONTRIBUTING.md"}:
                package.write(root / path, prefix + "/" + path.as_posix())
        package.writestr(prefix + "/SOURCE.txt", source_note)
    with zipfile.ZipFile(source_zip, "w", zipfile.ZIP_DEFLATED) as package:
        for path in paths:
            package.write(root / path, prefix + "-source/" + path.as_posix())
        # Keep the actual dependency source, including original per-file notices.
        # Exclude version-control metadata and JUCE's separately prebuilt tools.
        for path in sorted(juce.rglob("*")):
            relative = path.relative_to(juce)
            if not path.is_file() or ".git" in relative.parts or relative.as_posix() in {"DemoRunner.exe", "Projucer.exe"}:
                continue
            package.write(path, prefix + "-source/JUCE/" + relative.as_posix())
        package.writestr(prefix + "-source/SOURCE_BUILD.txt", source_note + "\n"
                         "From this source folder, with CMake and a C++ compiler installed:\n"
                         "cmake -S . -B Builds/release -DJUCE_PATH=JUCE -DCMAKE_BUILD_TYPE=Release\n"
                         "cmake --build Builds/release --config Release\n"
                         "ctest --test-dir Builds/release -C Release --output-on-failure\n"
                         "See docs/BUILDING.md for platform development dependencies.\n")
    sums.write_text("".join(hashlib.sha256(path.read_bytes()).hexdigest() + "  " + path.name + "\n"
                            for path in (windows_zip, source_zip)), encoding="utf-8")
    for path in (windows_zip, source_zip):
        with zipfile.ZipFile(path) as package:
            if package.testzip() is not None:
                raise SystemExit("Archive integrity check failed: " + str(path))
        print(path.name + ": " + str(path.stat().st_size) + " bytes; archive verified")
    print("Created SHA256SUMS.txt. Nothing has been uploaded.")


if __name__ == "__main__":
    main()
