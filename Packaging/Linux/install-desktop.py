#!/usr/bin/env python3
# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
"""Register the extracted application and its icon for the current Linux user."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

APP_ID = "io.github.zurek-jiri.temperament-generator"
MARKER = "X-TemperamentGenerator-Launcher=true"


def desktop_value(value):
    return str(value).replace("\\", "\\\\").replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t")


def exec_argument(value):
    # Desktop Entry specification: command quoting, then string-value escaping.
    value = str(value).replace("%", "%%")
    quoted = '"' + ''.join("\\" + c if c in '\\"`$' else c for c in value) + '"'
    return desktop_value(quoted)


def locations(data_home):
    return (data_home / "applications" / (APP_ID + ".desktop"),
            data_home / "icons/hicolor/512x512/apps" / (APP_ID + ".png"))


def check_owned(launcher, icon):
    if launcher.is_symlink() or icon.is_symlink():
        raise ValueError("Refusing to replace a symbolic-link launcher or icon.")
    if launcher.exists() and MARKER not in launcher.read_text(encoding="utf-8").splitlines():
        raise ValueError("A different launcher already uses this application ID.")
    if icon.exists() and not launcher.exists():
        raise ValueError("An icon already uses this application ID without our launcher.")


def install(package, data_home):
    package = package.resolve()
    binary = package / "Temperament Generator"
    source_icon = package / "temperament-generator.png"
    if not binary.is_file() or not source_icon.is_file():
        raise ValueError("Run this script from the complete extracted Linux release folder.")
    if any(c in str(binary) for c in "=\n\r\t"):
        raise ValueError("Move the extracted folder to a path without '=', tabs or line breaks, then retry.")
    launcher, icon = locations(data_home)
    check_owned(launcher, icon)
    entry = "\n".join([
        "[Desktop Entry]", "Version=1.0", "Type=Application",
        "Name=Temperament Generator", "Comment=Design, compare and hear musical temperaments",
        # GIO checks the first executable before expanding %% in Exec. Using
        # env keeps that check independent of percent signs in the install path.
        "Exec=/usr/bin/env -- " + exec_argument(binary),
        "TryExec=" + desktop_value(binary), "Icon=" + APP_ID,
        "Terminal=false", "Categories=AudioVideo;Audio;",
        "Keywords=temperament;tuning;music;organ;", "StartupNotify=false",
        "StartupWMClass=Temperament Generator", MARKER, "",
    ])
    icon.parent.mkdir(parents=True, exist_ok=True)
    launcher.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source_icon, icon)
    icon.chmod(0o644)
    launcher.write_text(entry, encoding="utf-8")
    launcher.chmod(0o644)
    return launcher


def remove(data_home):
    launcher, icon = locations(data_home)
    check_owned(launcher, icon)
    for path in (launcher, icon):
        if path.exists():
            path.unlink()


def refresh(data_home):
    for command in (["update-desktop-database", str(data_home / "applications")],
                    ["gtk-update-icon-cache", "--force", "--ignore-theme-index", str(data_home / "icons/hicolor")]):
        if shutil.which(command[0]):
            try:
                subprocess.run(command, check=False, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, timeout=15)
            except (OSError, subprocess.TimeoutExpired):
                pass  # Desktops can also discover files without these optional caches.


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--remove", action="store_true", help="Remove the menu launcher and icon; keep the application")
    args = parser.parse_args()
    if not sys.platform.startswith("linux"):
        parser.error("This helper is for Linux. On macOS, open the .app bundle.")
    if os.geteuid() == 0:
        parser.error("Run as your ordinary desktop user, without sudo.")
    configured = os.environ.get("XDG_DATA_HOME", "")
    data_home = Path(configured) if configured and Path(configured).is_absolute() else Path.home() / ".local/share"
    try:
        if args.remove:
            remove(data_home)
            print("Removed the application-menu launcher and icon. The application files are unchanged.")
        else:
            launcher = install(Path(__file__).resolve().parent, data_home)
            print("Installed application-menu launcher:", launcher)
            print("Open Temperament Generator from your application menu and pin it there if desired.")
            print("Keep this extracted folder in place. After moving it or upgrading, run this script again.")
        refresh(data_home)
    except (OSError, ValueError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
