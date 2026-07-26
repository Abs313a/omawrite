# Omawrite

A dead-simple Markdown writing app built with Qt Quick and C++ that automatically follows system dark/light mode.

## Features

- Focused plain-text Markdown editing with inline styling for headings, lists,
  quotes, code, bold, italic, and links.
- Atomic saves, crash recovery, and warnings for files changed outside
  Omawrite.
- An Omawrite-owned overwrite confirmation that keeps the editor darkened
  without relying on the portal's confirmation styling.
- Find and replace, word count, printing, fullscreen, and multiple windows.
- A responsive footer with Save As, Save, and Open controls, document status,
  an `F1: Keybindings` hint, and word count. The center hint hides before it
  can overlap the side content in narrow tiled windows.
- One Qt build for Wayland, X11/XCB, and XWayland.

## Install

Install via the Omarchy Package Repository via the `omawrite` package. It's installed by default in new installations of Omarchy (from Quattro forward).

## Footer controls

The bottom-left icons are ordered:

1. Save As
2. Save
3. Open

Footer text uses `#e0af68` at 75% opacity. The editor uses the bundled iA
Writer Mono S font at 18px with 120% line height; footer text uses 12px.

## Shortcuts

- `Ctrl+S` saves. New documents use the XDG desktop portal file picker.
- `Ctrl+Shift+S` saves as.
- `Ctrl+O` opens a Markdown file through the portal picker.
- `Ctrl+P` opens the system print dialog.
- `Ctrl+N` opens a new Omawrite window.
- `Ctrl+Z` and `Ctrl+Shift+Z` handle undo and redo.
- `F11` toggles fullscreen.
- `Ctrl+F` searches the document. Use `Enter` or `Ctrl+G` for the next match and `Shift+Enter` for the previous match.
- `Ctrl+H` opens find and replace.
- `Ctrl+B`, `Ctrl+I`, and `Ctrl+K` insert bold, italic, and link Markdown.
- `F1` shows the keyboard shortcut reference.

Unsaved drafts are recovered after an abnormal exit. Omawrite also watches open files
and warns before an external change can replace local work. Selecting an existing
save target opens Omawrite's own overwrite confirmation before any data is replaced.
In that confirmation, Cancel abandons Save As, No returns to the file picker,
and Overwrite replaces the selected file.

## Requirements

- Qt 6: `qt6-base`, `qt6-declarative`, `qt6-quickcontrols2`
- `xdg-desktop-portal` and a portal backend

The iA Writer Mono font is bundled under the SIL Open Font License 1.1; see
`fonts/OFL.txt`. The font is copyright Information Architects Inc. and based on
IBM Plex, copyright IBM Corp.
