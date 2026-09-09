# Publishing and maintenance

Repository: https://github.com/zurek-jiri/temperament-generator

Website: https://zurek-jiri.github.io/temperament-generator/

Maintainer and public credit: `zurek-jiri`. License: **AGPL-3.0-only**.
`COPYRIGHT`, `LICENSE`, `THIRD_PARTY_NOTICES.md` and `Licenses/` form part of
source and binary releases. The application also displays the notices offline.

## Keeping hosting free

Use a public repository, GitHub Free, the supplied `github.io` address, and
standard GitHub-hosted runners. No custom domain, paid runner, Codespaces,
Copilot subscription or GitHub Packages registry is needed for this project.
Build checks skip private repositories and use no artifact or cache storage.
Pages uses a small, short-lived deployment artifact.

Keep paid usage disabled and do not add a payment method just to host this project.
GitHub states that Actions usage exceeding included quotas is blocked when there
is no payment method. If an account already has payment details, review its
product budgets and enable stopping usage at the budget limit. These project
scripts do not change billing settings or subscribe to paid services.

These are GitHub's current services, not a promise about future prices.
See [GitHub Free](https://docs.github.com/en/get-started/learning-about-github/githubs-plans),
[Actions billing](https://docs.github.com/en/billing/concepts/product-billing/github-actions),
and [Pages limits](https://docs.github.com/en/pages/getting-started-with-github-pages/github-pages-limits).

## What is published

Source, tests, build scripts, documentation, original artwork and website assets
belong in Git. Applications and corresponding source bundles belong in Releases.
The `.gitignore` excludes build folders, loose executables, conversations, sign-in
data, publishing tools and the unused reference image. Review staged files before
every commit.

## Releasing an update

1. Update CMake's project version and the changelog. Use `major.minor.patch`:
   patch for compatible fixes, minor for compatible features, major for
   incompatible public-interface or file-format changes.
2. Run the Windows build, calculation tests and GUI previews. Review CI and test
   the interface before advertising support on another platform.
3. Keep notices aligned with the JUCE source used for the build:
   `python Tools/collect_notices.py --juce-path /path/to/JUCE`.
4. Commit the reviewed source, then package the release:

   ```sh
   python Tools/package_release.py --build-dir Builds/VS2026 --juce-path /path/to/JUCE --include-source
   ```

5. Tag the commit `vX.Y.Z`. Create a draft release, attach both archives and
   checksums, review the notes, then publish. Keep corresponding source available
   alongside every binary download. Never replace published files with a
   different build; release a new version.
6. Update website availability and run the Project website workflow.

The Windows executable contains the C++ runtime and needs no JUCE installation.
Validate releases on a clean Windows machine as part of release testing. Signing
requires a signing identity/certificate; the first build is unsigned, and Windows
may display an unknown-publisher warning.

## Website

Open `site/index.html` locally, or prepare it for publication:

```sh
python Tools/prepare_site.py --repository zurek-jiri/temperament-generator --output _site
```

GitHub Pages uses GitHub Actions as its publishing source. Run the manual Project
website workflow after reviewing a change. Downloads link to Releases; the
desktop application is not executed inside the website.

## License maintenance

Keep copyright and permission notices intact. Include the full AGPLv3 text,
no-warranty notice, library notices and corresponding source with releases.
Record modifications in the changelog and commit history. Downstream users may
redistribute and modify the application, including commercially, under the
license's source-sharing conditions. They do not need to buy a JUCE subscription
to use this AGPL build.

## Cross-platform releases from 1.5.5

Use **Prepare desktop release** on the reviewed main commit. It creates a draft,
builds and tests Windows x64, Linux x64, Apple Silicon Mac and Intel Mac, and
uploads the native archives, complete corresponding source and combined SHA-256
checksums. All jobs use standard runners on this public repository; they use
release assets instead of Actions artifact storage or caches.

Review all jobs and the five verified archives before publishing the draft as
the latest release. Keep older releases and tags intact. Dispatch **Project
website** after publication. A rerun may replace assets only on a draft for the
exact same source commit; the helper refuses to modify published releases.
