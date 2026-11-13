# SplinkNote v3

Minimalist, zero-latency stylus note-taking and PDF annotation application built for the Omarchy desktop.

Guided strictly by the Omarchy design philosophy: **One window, one job, opinionated minimalism, and 100% tablet-first UI & keyboard-first acceleration.**

---

## Features

- **Clean Slate Startup**: Opens with no document loaded. Jump straight into a new blank note or open an existing PDF via desktop portal.
- **Dual-Input Parity**:
  - **100% Tablet UI**: Floating ergonomic control pill with generous touch targets designed for 2-in-1 laptops folded over into tablets.
  - **Keyboard-First Acceleration**: Single-key shortcuts for rapid desktop operation.
- **Inking Physics & Digitizer Precision**:
  - **Pen**: Dynamic stroke width scaling with hardware stylus pressure.
  - **Highlighter**: Translucent multiplication over background text and notes.
  - **Swept Eraser**: Instant stroke-aware eraser with swept path collision testing.
  - **Bézier Smoothing**: Quadratic midpoint curve interpolation for organic, fluid ink.
- **Persistent Vector Strokes in PDF**:
  - Saves to standard, compliant PDF files viewable anywhere.
  - Embeds non-destructive vector stroke metadata directly in the PDF. Re-opening in SplinkNote restores the full stroke history for ongoing editing, erasing, undoing, and redoing.
- **Provenance & Dynamic Pages**:
  - Automatically identifies whether a PDF originated from SplinkNote or external software.
  - SplinkNote notebooks support indefinite page appending.
- **Adaptive Tiling (Hyprland)**: Scales seamlessly from compact side-by-side splits and quarter tiles to full-screen tablet canvas.
- **Deep Omarchy Desktop Harmony**: Follows the Omarchy theme accent color and Hyprland window rounding live, and adapts to system language without application restarts.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| <kbd>P</kbd> | Pen tool |
| <kbd>H</kbd> | Highlighter tool |
| <kbd>E</kbd> | Eraser tool |
| <kbd>Ctrl</kbd> + <kbd>Z</kbd> | Undo |
| <kbd>Ctrl</kbd> + <kbd>Y</kbd> / <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Z</kbd> | Redo |
| <kbd>Ctrl</kbd> + <kbd>S</kbd> | Save PDF |
| <kbd>Ctrl</kbd> + <kbd>O</kbd> | Open document |
| <kbd>Ctrl</kbd> + <kbd>N</kbd> | New blank note |
| <kbd>N</kbd> / <kbd>PageDown</kbd> | Next page |
| <kbd>B</kbd> / <kbd>PageUp</kbd> | Previous page |
| <kbd>+</kbd> / <kbd>=</kbd> | Add new page |
| <kbd>Esc</kbd> | Close current document (return to start screen) |
| <kbd>?</kbd> | Toggle keyboard shortcuts overlay |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd> | Quit application |

---

## Building and Testing

```sh
# Build binary
bin/build

# Run test suite
bin/test

# Build package and install into desktop launcher
bin/install
```

---

## License

MIT License. Portions copyright (c) 2024-2026 David Heinemeier Hansson, Omarchy contributors.
