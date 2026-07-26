# Dialog Overlay and Footer Hint Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give the unsaved-changes dialog the same dark modal dimmer as Omawrite's other dialogs and keep the centered keybindings hint visible at the 720-pixel minimum width.

**Architecture:** Reuse the shared `keybindingsOverlayColor` through an explicit property on `UnsavedChangesDialog`, matching `OverwriteDialog`. Remove unused footer space by sizing the status label to its rendered text while retaining the existing collision guard.

**Tech Stack:** Qt 6, QML/Qt Quick Controls, C++ QtTest

## Global Constraints

- The modal overlay color is exactly `#99000000`.
- `F1: Keybindings` remains horizontally centered.
- The hint may hide only when footer content genuinely collides.
- No GTK or portal theme integration is included.

---

### Task 1: Unsaved-changes modal overlay

**Files:**
- Modify: `src/UnsavedChangesDialog.qml`
- Modify: `src/Main.qml`
- Test: `tests/tst_omawrite.cpp`

**Interfaces:**
- Consumes: `ApplicationWindow.keybindingsOverlayColor`
- Produces: `UnsavedChangesDialog.overlayColor: color` and an `Overlay.modal` rectangle named `unsavedChangesDimmer`

- [ ] **Step 1: Write the failing test**

In `keyboardShortcutsActivate()`, locate the unsaved dialog and assert that it receives the shared overlay color:

```cpp
QObject *unsavedDialog =
    window->findChild<QObject *>(QStringLiteral("unsavedChangesDialog"));
QVERIFY(unsavedDialog);
QCOMPARE(unsavedDialog->property("overlayColor").value<QColor>(),
         QColor(QStringLiteral("#99000000")));
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `./bin/test`

Expected: FAIL because the dialog has no `objectName` or `overlayColor` property.

- [ ] **Step 3: Implement the overlay property and delegate**

Add to `UnsavedChangesDialog.qml`:

```qml
property color overlayColor: "#99000000"

Overlay.modal: Rectangle {
    objectName: "unsavedChangesDimmer"
    color: root.overlayColor
}
```

In the `UnsavedChangesDialog` instance in `Main.qml`, add:

```qml
objectName: "unsavedChangesDialog"
overlayColor: win.keybindingsOverlayColor
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `./bin/test`

Expected: all tests pass, including the shared overlay-color assertion.

- [ ] **Step 5: Commit**

```bash
git add src/UnsavedChangesDialog.qml src/Main.qml tests/tst_omawrite.cpp
git commit -m "fix: darken unsaved changes overlay"
```

### Task 2: Minimum-width footer hint

**Files:**
- Modify: `src/Main.qml`
- Test: `tests/tst_omawrite.cpp`

**Interfaces:**
- Consumes: `footerDocumentStatus.implicitWidth` and the existing footer collision bounds
- Produces: a content-sized, capped document-status label

- [ ] **Step 1: Change the existing test expectation**

In `footerKeybindingsHintAdapts()`, replace the 720-pixel hidden assertion with:

```cpp
window->setWidth(720);
QTRY_VERIFY(hint->property("visible").toBool());
const qreal narrowHintCenter = hint->property("x").toReal()
    + hint->property("width").toReal() / 2.0;
QVERIFY(qAbs(narrowHintCenter - window->width() / 2.0) <= 1.0);
QVERIFY(status->property("width").toReal()
        <= status->property("implicitWidth").toReal() + 1.0);
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `./bin/test`

Expected: FAIL because the status label still reserves 240 pixels at a 720-pixel window width and hides the hint.

- [ ] **Step 3: Size the status label to its content**

Replace the status label width in `Main.qml` with:

```qml
width: Math.min(implicitWidth, Math.min(360, win.width / 3))
```

Keep the existing `footerKeybindingsHint.visible` collision expression unchanged.

- [ ] **Step 4: Run complete verification**

Run:

```bash
./bin/test
./bin/build
git diff --check
```

Expected: 16 tests pass, the application builds, and the diff check reports no whitespace errors.

- [ ] **Step 5: Commit**

```bash
git add src/Main.qml tests/tst_omawrite.cpp
git commit -m "fix: retain footer hint at minimum width"
```
