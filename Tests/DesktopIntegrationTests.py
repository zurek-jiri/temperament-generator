# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
"""Desktop registration and icon checks; writes only to temporary test folders."""
import importlib.util
from pathlib import Path
import plistlib
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "Tools"))
from desktop_assets import check_mac_icon, check_png

spec = importlib.util.spec_from_file_location("desktop_installer", ROOT / "Packaging/Linux/install-desktop.py")
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


class DesktopIntegrationTests(unittest.TestCase):
    def setUp(self):
        parent = ROOT / "Preview/desktop-tests"
        parent.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=parent)
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name).resolve()
        assert parent.resolve() in self.folder.parents
        self.package = self.folder / "Organ $stops; 50%"
        self.package.mkdir()
        self.binary = self.package / "Temperament Generator"
        self.binary.write_text("test executable", encoding="utf-8")
        (self.package / "temperament-generator.png").write_bytes(b"test icon")
        self.data = self.folder / "user data"

    def test_install_upgrade_remove(self):
        launcher = installer.install(self.package, self.data)
        entry = launcher.read_text(encoding="utf-8")
        self.assertIn("StartupWMClass=Temperament Generator\n", entry)
        self.assertIn("Icon=" + installer.APP_ID + "\n", entry)
        self.assertIn("Exec=/usr/bin/env -- " + installer.exec_argument(self.binary) + "\n", entry)
        icon = installer.locations(self.data)[1]
        self.assertEqual(icon.read_bytes(), b"test icon")
        moved = self.folder / "new version"
        self.package.rename(moved)
        installer.install(moved, self.data)
        self.assertIn(installer.exec_argument(moved / self.binary.name), launcher.read_text())
        installer.remove(self.data)
        self.assertFalse(launcher.exists())
        self.assertFalse(icon.exists())
        self.assertTrue((moved / self.binary.name).exists())
        installer.remove(self.data)  # Already removed is harmless.

    def test_refuses_unrelated_launcher(self):
        launcher, _ = installer.locations(self.data)
        launcher.parent.mkdir(parents=True)
        launcher.write_text("[Desktop Entry]\nName=Someone else's application\n")
        before = launcher.read_bytes()
        with self.assertRaises(ValueError):
            installer.install(self.package, self.data)
        with self.assertRaises(ValueError):
            installer.remove(self.data)
        self.assertEqual(launcher.read_bytes(), before)

    def test_reserved_characters_round_trip(self):
        value = '/home/Music $HOME/50%/quotes"and`backtick\\/Temperament Generator'
        encoded = installer.exec_argument(value)
        # Undo desktop string escaping, then command quoting, then %% field escaping.
        command = encoded.replace('\\\\', '\\')
        decoded = []
        escaped = False
        for c in command[1:-1]:
            if escaped:
                decoded.append(c); escaped = False
            elif c == '\\':
                escaped = True
            else:
                decoded.append(c)
        self.assertEqual(''.join(decoded).replace('%%', '%'), value)

    @unittest.skipUnless(sys.platform.startswith("linux"), "Native Linux desktop integration")
    def test_native_desktop_launcher(self):
        validator = shutil.which("desktop-file-validate")
        gio = shutil.which("gio")
        self.assertIsNotNone(validator, "Install desktop-file-utils for Linux checks")
        self.assertIsNotNone(gio, "GLib gio is required for the Linux launcher check")
        marker = self.folder / "launched.txt"
        self.binary.write_text("#!/bin/sh\nprintf 'launched' > " + shlex.quote(str(marker)) + "\n")
        self.binary.chmod(0o755)
        launcher = installer.install(self.package, self.data)
        subprocess.run([validator, str(launcher)], check=True)
        subprocess.run([gio, "launch", str(launcher)], check=True, timeout=10)
        deadline = time.monotonic() + 5
        while not marker.exists() and time.monotonic() < deadline:
            time.sleep(.05)
        self.assertEqual(marker.read_text(), "launched")

    def test_icon_resource_checks(self):
        png = self.folder / "icon.png"
        png.write_bytes(b"\x89PNG\r\n\x1a\n" + struct.pack(">I4sII", 13, b"IHDR", 512, 512))
        check_png(png)
        png.write_bytes(b"not an icon")
        with self.assertRaises(ValueError):
            check_png(png)
        bundle = self.folder / "Test.app"
        resources = bundle / "Contents/Resources"
        resources.mkdir(parents=True)
        info = bundle / "Contents/Info.plist"
        info.write_bytes(plistlib.dumps({"CFBundleIconFile": "Icon.icns"}))
        icon = resources / "Icon.icns"
        icon.write_bytes(b"icns" + struct.pack(">I", 20) + b"ic08" + struct.pack(">I", 12) + b"data")
        self.assertEqual(check_mac_icon(bundle), icon)
        icon.write_bytes(b"icns" + struct.pack(">I", 100))
        with self.assertRaises(ValueError):
            check_mac_icon(bundle)
        info.write_bytes(plistlib.dumps({}))
        with self.assertRaises(ValueError):
            check_mac_icon(bundle)


if __name__ == "__main__":
    unittest.main()
