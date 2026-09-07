# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
# See LICENSE and COPYRIGHT for terms and the no-warranty notice.

"""Configure, compile and test with the installed Visual Studio and JUCE."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
version = re.search(r"project\(TemperamentGenerator VERSION (\d+\.\d+\.\d+)", (root / "CMakeLists.txt").read_text()).group(1)
cmake = Path(r"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")
if not cmake.is_file():
    cmake = shutil.which("cmake")
if not cmake:
    sys.exit("CMake was not found. Install CMake and Visual Studio with Desktop development with C++.")

# Some developer shells supply both Path and PATH. MSBuild rejects that environment.
environment = {name.upper(): value for name, value in os.environ.items()}
build = root / "Builds" / "VS2026"
commands = [
    [str(cmake), "-S", str(root), "-B", str(build), "-G", "Visual Studio 18 2026", "-A", "x64", "-DJUCE_PATH=C:/JUCE"],
    [str(cmake), "--build", str(build), "--config", "Release", "--parallel", "4"],
    [str(Path(cmake).with_name("ctest.exe")), "--test-dir", str(build), "-C", "Release", "--output-on-failure"],
]
if "--configure-only" in sys.argv:
    commands = commands[:1]
for command in commands:
    completed = subprocess.run(command, cwd=str(root), env=environment)
    if completed.returncode:
        sys.exit(completed.returncode)

if "--configure-only" not in sys.argv:
    binary = build / "TemperamentGenerator_artefacts" / "Release" / "Temperament Generator.exe"
    targets = [root / ("Temperament Generator " + version + ".exe"), root / "Temperament Generator.exe"]
    for name in ("Temperament Generator 1.4.1.exe", "Temperament Generator 1.4.exe", "Temperament Generator 1.3.1.exe", "Temperament Generator 1.3.exe", "Temperament Generator 1.2.1.exe", "Temperament Generator 1.2.exe", "Temperament Generator 1.1.exe"):
        legacy = root / name
        if legacy.exists():
            targets.append(legacy)
    copied = False
    for target in targets:
        try:
            shutil.copy2(binary, target)
            copied = True
        except PermissionError:
            print("Executable in use; left running: " + str(target), flush=True)
    if not copied:
        sys.exit("All desktop copies are in use. The built application is at: " + str(binary))

if "--preview" in sys.argv:
    app = build / "TemperamentGenerator_artefacts" / "Release" / "Temperament Generator.exe"
    try:
        completed = subprocess.run([str(app), "--render-preview", str(root / "Preview")],
                                   cwd=str(root), env=environment, timeout=30)
        sys.exit(completed.returncode)
    except subprocess.TimeoutExpired:
        sys.exit("The offscreen UI check exceeded 30 seconds.")
