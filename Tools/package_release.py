# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
# See LICENSE and COPYRIGHT for terms and the no-warranty notice.
"""Package native desktop binaries and optional complete corresponding source; never uploads."""
import argparse
import hashlib
from pathlib import Path
import re
import platform
import struct
import sys
import tarfile
import shutil
import subprocess
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--juce-path", required=True, help="Exact JUCE source used by the compiler")
    parser.add_argument("--git", default=shutil.which("git"))
    parser.add_argument("--output", default="dist")
    parser.add_argument("--include-source", action="store_true", help="Include a ZIP with project and exact JUCE sources")
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
    artefacts = build / "TemperamentGenerator_artefacts" / "Release"
    if sys.platform == "win32":
        platform_name = "windows-x64"
        binary = artefacts / "Temperament Generator.exe"
        data = binary.read_bytes()
        pe_offset = struct.unpack_from("<I", data, 0x3c)[0]
        if data[pe_offset:pe_offset+4] != b"PE\0\0" or struct.unpack_from("<H", data, pe_offset+4)[0] != 0x8664:
            parser.error("Release requires an x64 Windows executable, never Win32.")
    elif sys.platform == "darwin":
        arch = platform.machine()
        if arch not in {"arm64", "x86_64"}:
            parser.error("Unsupported Mac architecture: " + arch)
        platform_name = "macos-" + ("arm64" if arch == "arm64" else "x64")
        binary = artefacts / "Temperament Generator.app"
        executable = binary / "Contents/MacOS/Temperament Generator"
        subprocess.run(["lipo", str(executable), "-verify_arch", arch], check=True)
        subprocess.run(["codesign", "--force", "--deep", "--sign", "-", str(binary)], check=True)
        subprocess.run(["codesign", "--verify", "--deep", "--strict", str(binary)], check=True)
    elif sys.platform.startswith("linux"):
        platform_name = "linux-x64"
        binary = artefacts / "Temperament Generator"
        data = binary.read_bytes()[:20]
        if data[:5] != b"\x7fELF\x02" or struct.unpack_from("<H", data, 18)[0] != 62:
            parser.error("Release requires an x64 Linux executable.")
        linked = subprocess.check_output(["ldd", str(binary)], text=True)
        print(linked)
        if "not found" in linked:
            parser.error("Linux executable has missing shared libraries.")
    else:
        parser.error("Unsupported release platform.")
    if not binary.exists():
        parser.error("Built application was not found.")
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
    archive = destination / ("TemperamentGenerator-" + version + "-" + platform_name + (".tar.gz" if sys.platform.startswith("linux") else ".zip"))
    source_zip = destination / ("TemperamentGenerator-" + version + "-source.zip")
    sums = destination / ("SHA256SUMS-" + platform_name + ".txt")
    outputs = [archive] + ([source_zip] if args.include_source else [])
    if any(path.exists() for path in (*outputs, sums)):
        parser.error("Output already exists; choose a new output folder rather than replacing release files.")
    source_note = ("Temperament Generator " + version + "\nSource commit: " + commit + "\n\n"
                   "Complete corresponding source (including JUCE) is available beside this binary at:\n"
                   "https://github.com/zurek-jiri/temperament-generator/releases/tag/v" + version + "\n"
                   "Source asset: " + source_zip.name + "\n\n"
                   "License: AGPLv3, version 3 only. No warranty. See LICENSE and COPYRIGHT.\n")
    prefix = "TemperamentGenerator-" + version
    staging = destination / ("package-" + platform_name) / prefix
    staging.mkdir(parents=True, exist_ok=False)
    if binary.is_dir():
        shutil.copytree(binary, staging / binary.name, symlinks=True)
    else:
        shutil.copy2(binary, staging / binary.name)
    for path in paths:
        if path.parts[0] in {"Licenses", "docs"} or path.as_posix() in {"README.md", "LICENSE", "COPYRIGHT", "THIRD_PARTY_NOTICES.md", "CHANGELOG.md", "CONTRIBUTING.md"}:
            target = staging / path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(root / path, target)
    (staging / "SOURCE.txt").write_text(source_note, encoding="utf-8")
    if sys.platform.startswith("linux"):
        with tarfile.open(archive, "w:gz") as package:
            package.add(staging, arcname=prefix)
    elif sys.platform == "darwin":
        subprocess.run(["ditto", "-c", "-k", "--sequesterRsrc", "--keepParent", str(staging), str(archive)], check=True)
    else:
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as package:
            for path in sorted(staging.rglob("*")):
                if path.is_file():
                    package.write(path, prefix + "/" + path.relative_to(staging).as_posix())
    if args.include_source:
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
                            for path in outputs), encoding="utf-8")
    for path in outputs:
        if path.suffix == ".zip":
            with zipfile.ZipFile(path) as package:
                if package.testzip() is not None:
                    raise SystemExit("Archive integrity check failed: " + str(path))
        else:
            with tarfile.open(path) as package:
                # Read all contents to exercise gzip checksum verification too.
                for member in package:
                    if member.isfile():
                        stream = package.extractfile(member)
                        while stream.read(1024 * 1024):
                            pass
        print(path.name + ": " + str(path.stat().st_size) + " bytes; archive verified")
    print("Created " + sums.name + ". Nothing has been uploaded.")


if __name__ == "__main__":
    main()
