# Responsive Footer and Keybindings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> superpowers:subagent-driven-development (recommended) or
> superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a collision-aware centered `F1: Keybindings` footer hint, color
all footer text `#e0af68`, and make the documented shortcuts reliable with one
binary under Wayland and X11.

**Architecture:** Keep layout and shortcut behavior in `src/Main.qml`, where
the existing footer, dialogs, and actions already live. Extend the Qt Test QML
integration coverage to reproduce real key activation and responsive resizing
before changing behavior. Use Qt's runtime platform abstraction; do not create
display-server-specific builds.

**Tech Stack:** Qt 6.11, Qt Quick/QML, Qt Quick Controls Material, C++17,
Qt Test, qmake.

## Global Constraints

- Ship one binary for native Wayland, X11/XCB, and XWayland.
- Do not add build-time display-server detection.
- Preserve every existing documented shortcut and add F1.
- Reuse the existing in-window modal Keyboard Shortcuts dialog.
- Set only footer text to `#e0af68`; retain current icon colors.
- Hide the center hint before it can overlap either footer side.
- Keep unrelated editor and document behavior unchanged.

---

### Task 1: Reproduce shortcut activation and establish the root cause

**Files:**
- Modify: `tests/tst_omawrite.cpp`
- Test: `tests/tst_omawrite.cpp`

**Interfaces:**
- Consumes: `Main.qml` root window, `Backend::openDialogRequested`, existing
  search state, and the existing shortcuts dialog.
- Produces: `keyboardShortcutsActivate()` regression coverage using actual Qt
  key events.

- [ ] **Step 1: Add a QML-window helper and failing shortcut test**

Add a helper that constructs `Main.qml`, exposes `Backend`, shows the resulting
`QQuickWindow`, and returns both objects for the test lifetime. Add stable
`objectName` values to the dialog only when the test demonstrates they are
needed for observation.

Exercise representative categories with real key events:

```cpp
QTest::keyClick(window, Qt::Key_F1);
QVERIFY(shortcutsDialog->property("opened").toBool());

shortcutsDialog->setProperty("visible", false);
QSignalSpy openSpy(&backend, &Backend::openDialogRequested);
QTest::keyClick(window, Qt::Key_O, Qt::ControlModifier);
QCOMPARE(openSpy.count(), 1);

QTest::keyClick(window, Qt::Key_F, Qt::ControlModifier);
QVERIFY(window->property("searchOpen").toBool());
```

- [ ] **Step 2: Run the focused test and record the failure mode**

Run:

```bash
./bin/test keyboardShortcutsActivate
```

Expected before implementation: F1 fails because it is not declared; at least
one existing non-save shortcut reproduces the manual failure. Capture QML/Qt
warnings, especially `activatedAmbiguously` evidence, before selecting the
minimal correction.

- [ ] **Step 3: Compare working and failing shortcut paths**

Inspect the `Ctrl+S` and `Ctrl+Shift+S` objects against failing application and
editor shortcuts. Verify sequence parsing, active-window state, shortcut
context, focused-item handling, and ambiguity signals. State one root-cause
hypothesis supported by the focused test output.

- [ ] **Step 4: Commit the regression test**

```bash
git add tests/tst_omawrite.cpp
git commit -m "test: reproduce keyboard shortcut activation"
```

---

### Task 2: Repair shortcuts and add F1

**Files:**
- Modify: `src/Main.qml`
- Modify: `tests/tst_omawrite.cpp`
- Modify: `README.md`
- Test: `tests/tst_omawrite.cpp`

**Interfaces:**
- Consumes: the root-cause evidence and `keyboardShortcutsActivate()` from
  Task 1.
- Produces: reliable existing shortcut actions, an application-level F1
  binding, and `shortcutsDialog` as the shared modal interface.

- [ ] **Step 1: Add the smallest shared shortcut correction**

Keep shortcut actions centralized in root-window functions so normal and
ambiguous activation cannot diverge:

```qml
function showKeybindings() {
    shortcutsDialog.open()
}

Shortcut {
    objectName: "keybindingsShortcut"
    sequences: ["F1", "Ctrl+?"]
    context: Qt.ApplicationShortcut
    onActivated: win.showKeybindings()
    onActivatedAmbiguously: win.showKeybindings()
}
```

Apply the confirmed shared correction to the existing bindings. If ambiguity
is the demonstrated cause, route `onActivated` and `onActivatedAmbiguously` to
the same existing action. If focused-item interception is demonstrated
instead, correct that interception at its source and retain `Shortcut` as the
platform-neutral action layer.

- [ ] **Step 2: Expand the focused regression test**

Cover F1 plus representative file, search, formatting, undo/redo, and
fullscreen bindings. Check observable state or backend signals after each key
event; do not treat successful QML construction as proof of activation.

- [ ] **Step 3: Update discoverability copy**

Change the dialog and README shortcut lists so they describe:

```text
F1  Keybindings
Ctrl+?  Keybindings
```

Use “Keybindings” consistently for the new F1 label while retaining the
existing dialog title unless a test requires otherwise.

- [ ] **Step 4: Run the focused and full suites**

Run:

```bash
./bin/test keyboardShortcutsActivate
./bin/test
```

Expected: all shortcut assertions pass and the full suite reports zero
failures.

- [ ] **Step 5: Commit shortcut behavior**

```bash
git add src/Main.qml tests/tst_omawrite.cpp README.md
git commit -m "fix: make keyboard shortcuts reliable"
```

---

### Task 3: Add the responsive footer hint and color

**Files:**
- Modify: `src/Main.qml`
- Modify: `tests/tst_omawrite.cpp`
- Test: `tests/tst_omawrite.cpp`

**Interfaces:**
- Consumes: `win.showKeybindings()` and the existing `footerStatus` and word
  count label.
- Produces: `footerKeybindingsHint`, `footerWordCount`, and responsive
  collision-free footer behavior.

- [ ] **Step 1: Add failing footer presentation tests**

Assign stable object names and assert exact presentation:

```cpp
QCOMPARE(statusLabel->property("color").value<QColor>(),
         QColor(QStringLiteral("#e0af68")));
QCOMPARE(hint->property("text").toString(),
         QStringLiteral("F1: Keybindings"));
QCOMPARE(hint->property("color").value<QColor>(),
         QColor(QStringLiteral("#e0af68")));
QCOMPARE(wordCount->property("color").value<QColor>(),
         QColor(QStringLiteral("#e0af68")));
```

Resize the root window to a wide width and verify the hint is visible and its
center matches the window center within one pixel. Resize until the available
gap is smaller than the hint plus its safety margins and verify it hides.
Restore the wide size and verify it becomes visible again.

- [ ] **Step 2: Run the focused footer test and verify failure**

Run:

```bash
./bin/test footerKeybindingsHintAdapts
```

Expected before implementation: FAIL because the hint and fixed footer text
color do not yet exist.

- [ ] **Step 3: Implement the footer label and collision rule**

Keep the side items anchored as they are. Add a centered label with matching
font and opacity:

```qml
Label {
    id: footerKeybindingsHint
    objectName: "footerKeybindingsHint"
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    text: "F1: Keybindings"
    color: "#e0af68"
    opacity: 0.75
    font.family: "iA Writer Mono S"
    font.pixelSize: 11
    visible: x - 12 > footerStatus.x + footerStatus.width
             && x + width + 12 < footerWordCount.x
}
```

Give the word count label `id: footerWordCount` and
`objectName: "footerWordCount"`. Give the status label
`objectName: "footerDocumentStatus"`. Set those two text labels to
`color: "#e0af68"` without changing the Save/Open icon colors.

If the hint is clickable, use a `TapHandler` calling
`win.showKeybindings()` without changing its text geometry.

- [ ] **Step 4: Run focused and full automated verification**

Run:

```bash
./bin/test footerKeybindingsHintAdapts
./bin/test keyboardShortcutsActivate
./bin/test
git diff --check
```

Expected: responsive, shortcut, and existing tests all pass; whitespace check
is clean.

- [ ] **Step 5: Commit the footer**

```bash
git add src/Main.qml tests/tst_omawrite.cpp
git commit -m "feat: add responsive keybindings footer hint"
```

---

### Task 4: Verify runtime behavior on Wayland and X11

**Files:**
- Modify only if verification exposes a proven defect in the files already
  listed above.
- Test: built `build/omawrite` executable.

**Interfaces:**
- Consumes: completed shortcuts and responsive footer.
- Produces: one verified binary with Qt-selected Wayland and XCB backends.

- [ ] **Step 1: Rebuild from current source**

```bash
./bin/build
```

Expected: qmake and make exit successfully and produce `build/omawrite`.

- [ ] **Step 2: Run native Wayland smoke testing**

```bash
QT_QPA_PLATFORM=wayland ./build/omawrite
```

Manually verify F1, Ctrl+S, Ctrl+Shift+S, Ctrl+O, Ctrl+N, Ctrl+P, Ctrl+F,
Ctrl+H, Ctrl+B, Ctrl+I, Ctrl+K, undo/redo, fullscreen, wide footer centering,
and narrow-window hint hiding.

- [ ] **Step 3: Run X11/XCB smoke testing**

```bash
QT_QPA_PLATFORM=xcb ./build/omawrite
```

Repeat the same checklist. Confirm the application reports no QML shortcut or
layout warnings attributable to this change.

- [ ] **Step 4: Run final noninteractive verification**

```bash
./bin/test
git diff --check
git status --short
```

Expected: zero test failures, clean whitespace, and only intentional tracked
changes or commits.
