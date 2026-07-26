# Dialog Overlay and Footer Hint Design

## Goal

Keep Omawrite's unsaved-changes confirmation visually consistent with its
other in-window dialogs, and keep the centered `F1: Keybindings` footer hint
visible at the application's supported minimum width whenever the footer
content does not genuinely collide.

## Unsaved-changes dialog

`UnsavedChangesDialog` will expose an `overlayColor` property and supply its
own `Overlay.modal` rectangle, following the existing `OverwriteDialog`
pattern. `Main.qml` will pass the shared `keybindingsOverlayColor`
(`#99000000`) into the dialog. This replaces Qt Quick Controls' bright default
modal dimmer without changing the dialog, buttons, or close behavior.

## Responsive footer

The document-status label currently reserves one-third of the window even
when its text is short. Its width will instead follow its rendered content,
capped at the existing maximum. The existing collision-based visibility rule
for the centered hint will remain as a final safeguard.

At the 720-pixel minimum window width, ordinary statuses such as `Unsaved`
will no longer consume unused space, so `F1: Keybindings` remains centered and
visible. If unusually long status text actually reaches the center region,
the hint may still hide rather than overlap.

## Verification

Automated QML tests will verify:

- the unsaved-changes dialog uses the shared dark overlay color;
- the footer hint remains visible and centered at 720 pixels with a normal
  unsaved status;
- the full existing test suite and application build still succeed.

No GTK or portal theme integration is included.
