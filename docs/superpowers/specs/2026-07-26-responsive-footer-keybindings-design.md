# Responsive Footer and Keybindings Design

## Goal

Add a centered `F1: Keybindings` hint to Omawrite's footer and make `F1`
open the existing Keyboard Shortcuts dialog. Preserve and repair the existing
documented shortcuts so the same application build behaves correctly under
both Wayland and X11.

## Current behavior

The footer has a left-aligned group containing Save, Open, and document status,
plus a right-aligned word count. The footer text currently uses the dynamic
muted theme color.

The QML declares shortcuts for file operations, editing, search, formatting,
fullscreen, and the shortcut dialog. In manual testing of the locally built
fork under Wayland, only `Ctrl+S` and `Ctrl+Shift+S` activated successfully.
The existing tests build and pass but do not exercise real keyboard activation.

## Footer design

Add a text label reading `F1: Keybindings` whose horizontal center follows the
window's horizontal center rather than the remaining space between the side
items. This keeps the hint visually centered in both tiled and floating
windows.

The label must:

- Use iA Writer Mono S at the same 11-pixel size as the current footer text.
- Use the fixed color `#e0af68`.
- Match the footer text's existing subdued opacity.
- Share the existing bottom margin and vertical alignment.
- Expose a stable QML `objectName` for tests.
- Open the Keyboard Shortcuts dialog when clicked if implemented as an
  interactive control.

The status text and word count must also use `#e0af68`. Save and Open icons are
not footer text and retain their current theme-derived color.

The center hint must never overlap the left footer group or right word count.
It remains visible only when its implicit width, plus deliberate clear space on
both sides, fits between those items. When a narrow tiled window provides
insufficient room, the hint hides cleanly; status and word count retain their
positions and behavior. Resizing back to a sufficient width restores it.

## Keybinding behavior

`F1` becomes an application-level shortcut that opens the existing Keyboard
Shortcuts dialog. The existing `Ctrl+?` shortcut remains supported, and the
dialog copy lists `F1` as the primary discoverable binding.

All currently documented shortcuts remain unchanged. The implementation must
first establish why most current QML `Shortcut` objects do not activate during
manual use. The correction should address that shared cause rather than
rewriting individual bindings speculatively.

The shortcut implementation must be platform-neutral Qt code. Omawrite should
ship one binary that selects its Qt platform backend at runtime. Build-time
detection of Wayland or X11 must not be introduced because it would tie the
binary to the environment in which it was compiled.

Runtime platform identification may be added only for diagnostics if the
investigation needs it. Platform-specific input branches are acceptable only
if isolated Wayland and X11 evidence proves Qt cannot provide equivalent
behavior through one shared path.

## Validation

Automated QML tests must verify:

- The new hint exists and has the exact text `F1: Keybindings`.
- Status, hint, and word-count text use `#e0af68`.
- The hint is centered in a sufficiently wide window.
- The hint hides when the available space would make it collide with either
  footer side and returns when space is restored.
- Activating the F1 shortcut opens the existing dialog.
- Representative existing application and editor shortcuts still invoke their
  intended actions.

The full `./bin/test` suite must pass. The built application must also receive
manual smoke testing with the native Wayland backend and the X11/XCB backend:

```bash
QT_QPA_PLATFORM=wayland ./build/omawrite
QT_QPA_PLATFORM=xcb ./build/omawrite
```

Manual checks cover F1, file shortcuts, search, formatting, undo/redo,
fullscreen, the centered wide-window layout, and the narrow tiled-window
fallback. XWayland may be tested as an additional deployment path, but it does
not replace native Wayland and XCB coverage.

## Scope boundaries

This change does not redesign the footer, alter icon colors, introduce
display-server-specific builds, change existing shortcut assignments, or add a
new keybindings interface. It reuses the current dialog and keeps unrelated
editor behavior unchanged.
