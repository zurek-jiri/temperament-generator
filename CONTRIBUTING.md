# Contributing

Contributions are welcome through GitHub issues and pull requests. Contributions
to this project must be offered under its AGPL-3.0-only license. Preserve existing
copyright notices and add change notes where appropriate.

For a bug report, include your OS, app version, steps to reproduce, the expected
result and, when relevant, the twelve CSV values or comma expressions. A small
reproducible tuning is more useful than a large screenshot alone.

For changes, open a focused pull request with a short description and validation.
Run the mathematical tests described in [BUILDING](docs/BUILDING.md). Changes to
the interface should also run `--render-preview` and be inspected at normal and
high display scaling. Preserve large type, keyboard access and the scrolling layout.

Keep mathematical calculations separate from the GUI. Do not silently round values
used for circle closure; approximations are display information. Preserve imported
CSV values until the user explicitly calculates a replacement chart.
