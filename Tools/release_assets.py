# Copyright (C) 2026 zurek-jiri and contributors.
# SPDX-License-Identifier: AGPL-3.0-only
"""Create/verify draft release assets. Publishing remains a separate final step."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("prepare", "upload", "verify"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    version = re.search(r"project\(TemperamentGenerator VERSION (\d+\.\d+\.\d+)", (root / "CMakeLists.txt").read_text()).group(1)
    tag = "v" + version
    sha = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()
    repository = os.environ["GITHUB_REPOSITORY"]

    def gh(*command):
        return subprocess.check_output(["gh", *command, "--repo", repository], cwd=root, text=True)

    if args.action == "prepare":
        releases = json.loads(gh("release", "list", "--limit", "100", "--json", "tagName"))
        if not any(item["tagName"] == tag for item in releases):
            gh("release", "create", tag, "--draft", "--target", sha,
               "--title", "Temperament Generator " + version,
               "--notes-file", "docs/RELEASE_NOTES.md")
    release = json.loads(gh("release", "view", tag, "--json", "isDraft,targetCommitish,assets,url"))
    if not release["isDraft"] or release["targetCommitish"] != sha:
        raise SystemExit("Refusing to modify a published release or a draft for a different source commit.")
    if args.action == "upload":
        files = sorted(path for path in (root / "dist").iterdir() if path.is_file())
        if not files:
            raise SystemExit("No packaged assets found.")
        # Reruns can repair this exact draft only; published assets are immutable here.
        gh("release", "upload", tag, *map(str, files), "--clobber")
    elif args.action == "verify":
        destination = root / "dist" / "verify"
        destination.mkdir(parents=True, exist_ok=False)
        gh("release", "download", tag, "--dir", str(destination))
        prefix = "TemperamentGenerator-" + version
        expected = {prefix + "-" + suffix for suffix in (
            "windows-x64.zip", "linux-x64.tar.gz", "macos-x64.zip", "macos-arm64.zip", "source.zip")}
        hashes = {}
        for manifest in destination.glob("SHA256SUMS-*.txt"):
            for line in manifest.read_text().splitlines():
                digest, filename = line.split("  ", 1)
                if filename not in expected or filename in hashes or not re.fullmatch(r"[0-9a-f]{64}", digest):
                    raise SystemExit("Unexpected or duplicate checksum entry: " + filename)
                hashes[filename] = digest
        if set(hashes) != expected:
            raise SystemExit("Missing release archives or checksums.")
        for filename, digest in hashes.items():
            with (destination / filename).open("rb") as stream:
                actual = hashlib.file_digest(stream, "sha256").hexdigest()
            if actual != digest:
                raise SystemExit("Download checksum failed: " + filename)
            print("Verified downloaded asset: " + filename, flush=True)
        combined = destination / "SHA256SUMS.txt"
        combined.write_text("".join(hashes[name] + "  " + name + "\n" for name in sorted(hashes)), encoding="utf-8")
        gh("release", "upload", tag, str(combined), "--clobber")
        for asset in release["assets"]:
            if asset["name"].startswith("SHA256SUMS-") and asset["name"].endswith(".txt"):
                gh("release", "delete-asset", tag, asset["name"], "--yes")
        print("All five archives verified. Release remains a draft for final review.")
    print(release["url"])


if __name__ == "__main__":
    main()
