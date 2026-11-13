# SplinkNote v3 — Technical Feedback & Improvement Notes

> **Document Purpose**: Task backlog and architectural plan recorded for immediate execution upon next session.  
> **Source**: User feedback & visual inspection of desktop tiling comparison (Omawrite, Hype, Omacalc).

---

## 1. Continuous Vertical Multi-Page Flow (Infinite / Natural Notebook Canvas)
* **Problem**: Discrete pagination (`1 / 1` with `<` `>` and manual `+` button) creates friction for natural note-taking.
* **Architecture Solution**:
  * Implement continuous vertical page stacking: Page 1, Page 2, Page 3 rendered sequentially with a standard 36pt vertical gap.
  * In blank notebook mode, automatically append new blank pages as the user scrolls or inks near the bottom of the last page.
  * Update `pageScreenRect(int index)` and viewport frustum culling so only pages currently intersecting the viewport are painted.
  * Map input coordinates dynamically via `pageIndexAtScreenPos(pos)` so strokes land on the correct page without modal navigation.

---

## 2. Touch Gestures & Status Bar Zoom Controls
* **Touch Pinch-to-Zoom**:
  * Track multi-touch points in `touchEvent(QTouchEvent *event)`. When two touch points are active, compute the ratio of current distance to initial distance, zooming smoothly around the gesture centroid.
* **Status Bar Controls**:
  * Add a compact, touch-friendly zoom cluster at the bottom-right corner of the window:
    * `[-]` Zoom Out
    * `[100%]` / `[Fit]` Toggle between 100% zoom and fit-to-width
    * `[+]` Zoom In
  * Hit targets sized to minimum 36×36px for finger/stylus precision.

---

## 3. Hardware Stylus Eraser Tip & Barrel Buttons
* **Hardware Eraser Tip**:
  * Detect `QPointingDevice::PointerType::Eraser` in `QTabletEvent` and `QPointerEvent`. Automatically switch to the swept eraser when the physical stylus is flipped upside down, restoring the active tool upon flipping back.
* **Barrel / Side Buttons**:
  * Detect `Qt::RightButton` or `Qt::TaskButton` during pointer move/press. While held, temporarily engage the swept stroke eraser.
* **Eraser Hover Footprint**:
  * Render a subtle circular dashed indicator at the hover position when the eraser tip or barrel button is active, showing the exact swept strike radius before making contact.

---

## 4. Tack-Sharp High-DPI Rendering (Poppler & Vector Strokes)
* **Poppler External PDF Rasterization**:
  * Replace fixed 150 DPI rendering with high-DPI supersampling:  
    `dpi = 72.0 * devicePixelRatio * 2.0` (300+ DPI target) with `QImage::Format_ARGB32_Premultiplied` and smooth bilinear scaling.
* **Vector Inking Pipeline**:
  * Synchronize texture resolution with device pixel ratio:  
    `setTextureSize(size().toSize() * devicePixelRatio())`.
  * Set `QPainter::LosslessImageRendering` and `QPainter::SmoothPixmapTransform`.
  * Ensure subpixel midpoint curve calculations use double-precision floating point.

---

## 5. Highlighter Multi-Stroke Blending Fix
* **Issue**: Multiple overlapping highlighter strokes currently experience composition clipping or dark overlap seams.
* **Solution**:
  * Highlighters should use uniform translucent alpha blending (e.g. `QColor(accent).setAlphaF(0.35)`) or render to an offscreen single-pass layer with `CompositionMode_Multiply` so overlapping strokes within the same gesture or page merge uniformly without dark blotches.

---

## 6. Dynamic Theme-Reactive Canvas Background
* **Issue**: Canvas workspace background was static `#1a1b26`.
* **Solution**:
  * Replace static color with `m_backend->themeBackground()` or pass `theme.background` to `CanvasItem`.
  * Connect to `Theme::changed` signal so when the user changes desktop themes (e.g., Tokyo Night to Catppuccin, Gruvbox, Nord), the background updates immediately.

---

## 7. Explicit Shortcut for Redo (<kbd>Ctrl</kbd> + <kbd>Y</kbd>)
* **Issue**: On Linux/X11/Wayland, `StandardKey.Redo` frequently maps only to `Ctrl+Shift+Z`, causing `Ctrl+Y` to fail silently.
* **Solution**:
  * Add explicit shortcut in `Main.qml`:
    ```qml
    Shortcut {
        sequence: "Ctrl+Y"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument && backend.canRedo
        onActivated: backend.redo()
    }
    ```

---

## 8. Minimalist Contextual Shelf (Color & Size Palette)
* **Design**:
  * An unobtrusive, collapsible vertical pill docked to the left edge:
    * When **Pen** is active: 4 curated ink dots (Theme Accent, Ink Dark, Crimson Red, Forest Green) + 3 stroke weights (Fine 1.5pt, Medium 3.0pt, Bold 5.0pt).
    * When **Highlighter** is active: 3 pastel colors (Yellow, Cyan, Pink) + 2 chisel sizes.
    * When **Eraser** is active: Small / Medium / Large radius.
  * Auto-hides or collapses with a single tap to maximize writing space.
