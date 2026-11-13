# SplinkNote v3 — Technical Feedback & Roadmap Backlog

> **Document Purpose**: Verified task backlog and architectural plan for SplinkNote v3 improvements.  
> **Source**: User feedback & design alignment sessions.

---

## Sprint 2: Core UX, Inking Boundary, and Input Refinements

### 1. Centered Header Tool Cluster (`[Pen, Highlighter, Eraser]`)
* **Objective**: True aesthetic symmetry in the top navigation pill ([ControlPill.qml](file:///home/sam/Work/splinknote/src/ControlPill.qml)).
* **Design**:
  * Anchor the center tools directly to `horizontalCenter: parent.horizontalCenter` of the window.
  * Implement responsive clamping/scrunching: if window width narrows in tiling mode, clamp the cluster between the left title section and right navigation/save section to avoid overlapping or clipping.

### 2. Static Contextual Shelf (Remove Collapse Toggle)
* **Objective**: Remove unnecessary buttons and UI noise in [ToolShelf.qml](file:///home/sam/Work/splinknote/src/ToolShelf.qml).
* **Design**:
  * Remove `‹` / `›` toggle button, `collapsed` state, and width animation.
  * Shelf is a fixed, clean 50px pill docked to the left edge with curated color swatches and stroke size presets.

### 3. Strict Page Boundaries (No Drawing Off the Page)
* **Objective**: Drawing must be strictly confined to the page surface.
* **Design**:
  * In [canvasitem.cpp](file:///home/sam/Work/splinknote/src/canvasitem.cpp), apply `painter->setClipRect(pageRect)` when rendering vector strokes and highlighters so ink can never bleed into margins.
  * Disallow starting inking strokes outside the page rect.

### 4. Margin Scrolling (Vertical Page Navigation)
* **Objective**: Natural tablet touch navigation without leaving accidental ink marks.
* **Design**:
  * Touching or dragging with finger or stylus **outside** the page (in the margin or page gaps) initiates vertical scrolling (`panY`).
  * Inking occurs on the page; scrolling occurs in the margin.

### 5. Smart Horizontal Panning Lock & Auto-Centering
* **Objective**: Prevent lateral drift during reading and scrolling.
* **Design**:
  * When `pageWidth * zoom <= windowWidth`, lock horizontal panning (`panX = 0`) and keep the document centered.
  * When `pageWidth * zoom > windowWidth` (zoomed in past full width), enable horizontal panning clamped to page edges.
  * Auto-recenter: When zooming back out to fit or <= window width, automatically reset `panX = 0`.

### 6. Direct Fast-Save vs. First-Time "Save As"
* **Objective**: Eliminate repeated file dialog prompts on save in [backend.cpp](file:///home/sam/Work/splinknote/src/backend.cpp).
* **Design**:
  * If `currentFilePath` is non-empty (opened existing PDF or already saved once), clicking Save or pressing <kbd>Ctrl</kbd>+<kbd>S</kbd> writes directly to that file immediately with no dialog.
  * If `currentFilePath` is empty (new untitled note), clicking Save triggers the portal file chooser (Save As) for the first save only, then remembers the path.

### 7. Robust Stylus Hardware Eraser & Barrel Button Support
* **Objective**: Reliable physical eraser tip and side barrel button detection across Wayland / libinput styluses.
* **Design**:
  * Inspect `QTabletEvent`, `QMouseEvent`, and `QPointerEvent`.
  * Detect:
    1. `pointingDevice()->pointerType() == QPointingDevice::PointerType::Eraser`
    2. `buttons() & (Qt::RightButton | Qt::TaskButton | Qt::SecondaryButton | Qt::ExtraButton1)`
  * Support dynamic mid-stroke switching: pressing the barrel button while drawing switches seamlessly into swept eraser mode without lifting the stylus.
  * Show dashed hover indicator ring whenever the eraser button is held or eraser tip is active.

### 8. Fixed Small-Point Precision Eraser (Immutable Size)
* **Objective**: Prevent accidental broad erasing of adjacent notes; precision stylus eraser tip.
* **Design**:
  * Fix the eraser radius to a small precision point (~6.0pt).
  * Remove mutable eraser size presets (Small/Medium/Large) from [ToolShelf.qml](file:///home/sam/Work/splinknote/src/ToolShelf.qml).
  * Eraser tool shelf simply provides a clean "Clear Page" action button.
