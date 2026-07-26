# Documentation and README Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Flatten Omawrite's documentation, add the supplied theme screenshots, and replace the detailed README with a concise upstream-style fork README.

**Architecture:** Preserve document filenames and Git history with `git mv`, while storing all Markdown documents and screenshots directly under `docs/`. Rebuild `README.md` around the user's fork attribution and screenshots, documenting only general enhancements, installation, current bindings, environments, requirements, and attribution.

**Tech Stack:** Markdown, Git, shell validation

## Global Constraints

- Preserve the user's uncommitted README intent.
- Display the upstream URL as the linked text `Omawrite`.
- Show `Dark_theme.png` before `Light_theme.png`.
- Do not document exact colors, opacities, dimensions, or dialog implementation details.
- Keep all documentation and screenshot assets directly under `docs/`.

---

### Task 1: Flatten documentation and add screenshots

**Files:**
- Move: `docs/superpowers/specs/2026-07-26-responsive-footer-keybindings-design.md` → `docs/2026-07-26-responsive-footer-keybindings-design.md`
- Move: `docs/superpowers/plans/2026-07-26-responsive-footer-keybindings.md` → `docs/2026-07-26-responsive-footer-keybindings.md`
- Move: `docs/superpowers/specs/2026-07-27-dialog-overlay-footer-hint-design.md` → `docs/2026-07-27-dialog-overlay-footer-hint-design.md`
- Move: `docs/superpowers/plans/2026-07-27-dialog-overlay-footer-hint.md` → `docs/2026-07-27-dialog-overlay-footer-hint.md`
- Add: `docs/Dark_theme.png`
- Add: `docs/Light_theme.png`

**Interfaces:**
- Consumes: supplied PNG files under `/home/abs/Pictures/`
- Produces: flat, repository-relative documentation and image paths

- [ ] **Step 1: Move existing Markdown files with Git history**

Run:

```bash
git mv docs/superpowers/specs/2026-07-26-responsive-footer-keybindings-design.md docs/
git mv docs/superpowers/plans/2026-07-26-responsive-footer-keybindings.md docs/
git mv docs/superpowers/specs/2026-07-27-dialog-overlay-footer-hint-design.md docs/
git mv docs/superpowers/plans/2026-07-27-dialog-overlay-footer-hint.md docs/
```

- [ ] **Step 2: Copy the supplied screenshots**

Copy:

```text
/home/abs/Pictures/Dark_theme.png  -> docs/Dark_theme.png
/home/abs/Pictures/Light_theme.png -> docs/Light_theme.png
```

Use a byte-preserving copy and verify both destination files are PNG images
with dimensions `981x985`.

- [ ] **Step 3: Verify the flattened layout**

Run:

```bash
find docs -mindepth 2 -type f
find docs -maxdepth 1 -type f -printf '%f\n' | sort
```

Expected: the first command prints nothing; the second lists all Markdown
documents plus both screenshots.

- [ ] **Step 4: Commit**

```bash
git add docs
git commit -m "docs: flatten project documentation"
```

### Task 2: Rewrite the concise fork README

**Files:**
- Modify: `README.md`

**Interfaces:**
- Consumes: `docs/Dark_theme.png`, `docs/Light_theme.png`, `bin/install`, and shortcut sequences from `src/Main.qml`
- Produces: portable repository-relative image links and accurate user-facing instructions

- [ ] **Step 1: Replace the README with the approved structure**

Write these sections in order:

```markdown
# Omawrite

My fork of [Omawrite](https://github.com/omacom-io/omawrite).

![Omawrite dark theme](docs/Dark_theme.png)

![Omawrite light theme](docs/Light_theme.png)

A dead-simple Markdown writing app built with Qt Quick and C++ that
automatically follows system dark/light mode.

## Enhancements

[A concise list of the fork's general user-facing enhancements.]

## Install

[Omarchy package instructions, followed by `./bin/install` for an Arch-based
source checkout.]

## Shortcuts

[The current bindings from `src/Main.qml`.]

## Supported environments

[Wayland, X11/XCB, and XWayland.]

## Requirements

[The concise upstream dependency and bundled-font attribution.]
```

Do not retain the detailed footer styling or dialog-behavior paragraphs.

- [ ] **Step 2: Validate README references and shortcut accuracy**

Run:

```bash
test -f docs/Dark_theme.png
test -f docs/Light_theme.png
rg -n 'sequence: "' src/Main.qml
rg -n 'docs/(Dark_theme|Light_theme)\.png|Ctrl\+|F1|F11|Wayland|X11|XWayland' README.md
```

Expected: both images exist, and the README binding list agrees with the QML
shortcut declarations.

- [ ] **Step 3: Validate repository state**

Run:

```bash
git diff --check
find docs -mindepth 2 -type f
git status --short
```

Expected: no whitespace errors, no nested files below `docs/`, and only the
intended README change remains after Task 1's commit.

- [ ] **Step 4: Commit**

```bash
git add README.md
git commit -m "docs: refresh fork README"
```
