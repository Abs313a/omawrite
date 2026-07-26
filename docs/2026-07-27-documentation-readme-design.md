# Documentation and README Design

## Goal

Flatten Omawrite's project documentation and replace the detailed fork README
with a concise presentation modeled on the upstream project.

## Documentation layout

All Markdown documents currently under `docs/superpowers/specs/` and
`docs/superpowers/plans/` will move directly into `docs/` without renaming.
The empty `superpowers/` hierarchy will be removed. Git-aware moves will
preserve file history.

The supplied screenshots will be copied into the same flat directory:

- `docs/Dark_theme.png`
- `docs/Light_theme.png`

## README structure

The README will contain:

1. `# Omawrite`
2. `My fork of [Omawrite](https://github.com/omacom-io/omawrite)`
3. The dark-theme screenshot
4. The light-theme screenshot
5. A short description and general enhancement summary
6. Installation through the Omarchy package and from source with
   `./bin/install`
7. The current shortcut list
8. Supported Wayland, X11/XCB, and XWayland environments
9. Concise requirements and bundled-font attribution retained from upstream

Implementation details such as exact colors, opacity values, dimensions,
dialog internals, and footer layout algorithms will not appear in the README.

## Verification

- Every README image and documentation link resolves inside the repository.
- No files remain below `docs/superpowers/`.
- The documented shortcuts match `src/Main.qml`.
- Markdown formatting and `git diff --check` pass.
