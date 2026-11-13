#include "canvasitem.h"

#include <QHoverEvent>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QPainter>
#include <QTabletEvent>
#include <QTouchEvent>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace {
QPainterPath buildSmoothPath(const QVector<StrokePoint> &pts) {
    QPainterPath path;
    if (pts.isEmpty())
        return path;
    if (pts.size() == 1) {
        path.moveTo(pts[0].x, pts[0].y);
        path.lineTo(pts[0].x + 0.1f, pts[0].y + 0.1f);
        return path;
    }
    path.moveTo(pts[0].x, pts[0].y);
    if (pts.size() == 2) {
        path.lineTo(pts[1].x, pts[1].y);
        return path;
    }
    for (int i = 1; i < pts.size() - 1; ++i) {
        const QPointF mid((pts[i].x + pts[i + 1].x) * 0.5f,
                          (pts[i].y + pts[i + 1].y) * 0.5f);
        path.quadTo(pts[i].x, pts[i].y, mid.x(), mid.y());
    }
    path.lineTo(pts.last().x, pts.last().y);
    return path;
}
}

CanvasItem::CanvasItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptTouchEvents(true);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
    setMipmap(true);
}

void CanvasItem::setBackend(Backend *b) {
    if (m_backend == b)
        return;

    if (m_backend)
        disconnect(m_backend, nullptr, this, nullptr);

    m_backend = b;

    if (m_backend) {
        connect(m_backend, &Backend::canvasNeedsRepaint, this, [this]() {
            update();
        });
        connect(m_backend, &Backend::pageDimensionsChanged, this, [this]() {
            update();
        });
        connect(m_backend, &Backend::pageCountChanged, this, [this]() {
            update();
        });
        connect(m_backend, &Backend::currentPageIndexChanged, this, [this]() {
            update();
        });
    }

    emit backendChanged();
    update();
}

void CanvasItem::setZoom(qreal z) {
    z = std::clamp(z, 0.25, 4.0);
    if (!qFuzzyCompare(m_zoom, z)) {
        m_zoom = z;
        emit zoomChanged();
        checkAutoExpand();
        update();
    }
}

void CanvasItem::setPanX(qreal x) {
    if (!qFuzzyCompare(m_panX, x)) {
        m_panX = x;
        emit panChanged();
        update();
    }
}

void CanvasItem::setPanY(qreal y) {
    if (!qFuzzyCompare(m_panY, y)) {
        m_panY = y;
        emit panChanged();
        updateActivePageIndexFromPan();
        checkAutoExpand();
        update();
    }
}

void CanvasItem::setBackgroundColor(const QColor &color) {
    if (m_backgroundColor != color && color.isValid()) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
        update();
    }
}

void CanvasItem::resetView() {
    m_zoom = 1.0;
    m_panX = 0.0;
    m_panY = 0.0;
    emit zoomChanged();
    emit panChanged();
    update();
}

void CanvasItem::zoomIn() {
    setZoom(m_zoom * 1.25);
}

void CanvasItem::zoomOut() {
    setZoom(m_zoom / 1.25);
}

void CanvasItem::fitToWidth(qreal viewWidth, qreal viewHeight) {
    Q_UNUSED(viewHeight);
    if (!m_backend || viewWidth <= 0)
        return;
    const qreal pageW = m_backend->pageWidth();
    if (pageW <= 0)
        return;

    const qreal targetZoom = (viewWidth - 48.0) / pageW;
    setZoom(std::clamp(targetZoom, 0.35, 2.5));
    m_panX = 0.0;
    m_panY = 0.0;
    emit panChanged();
    update();
}

void CanvasItem::scrollToPage(int pageIndex) {
    if (!m_backend || pageIndex < 0 || pageIndex >= m_backend->pageCount())
        return;
    const qreal pageTopW = m_backend->pageTopWorld(pageIndex);
    setPanY(-pageTopW * m_zoom);
}

QRectF CanvasItem::pageScreenRect(int index) const {
    if (!m_backend || index < 0 || index >= m_backend->pageCount())
        return QRectF();

    const DocumentPage *p = m_backend->pageAt(index);
    if (!p)
        return QRectF();

    const qreal scaledW = p->width * m_zoom;
    const qreal scaledH = p->height * m_zoom;

    qreal originX = m_panX;
    if (width() > scaledW) {
        originX += (width() - scaledW) * 0.5;
    }
    const qreal pageTopW = m_backend->pageTopWorld(index);
    const qreal originY = m_panY + 24.0 + (pageTopW * m_zoom);

    return QRectF(originX, originY, scaledW, scaledH);
}

int CanvasItem::pageIndexAtScreenPos(const QPointF &screenPos, QPointF *outPagePos) const {
    if (!m_backend || m_backend->pageCount() == 0)
        return -1;

    const int count = m_backend->pageCount();

    // 1. Direct hit check
    for (int i = 0; i < count; ++i) {
        QRectF r = pageScreenRect(i);
        if (screenPos.y() >= r.top() && screenPos.y() <= r.bottom()) {
            if (outPagePos) {
                outPagePos->setX((screenPos.x() - r.left()) / m_zoom);
                outPagePos->setY((screenPos.y() - r.top()) / m_zoom);
            }
            return i;
        }
    }

    // 2. Above first page
    if (screenPos.y() < pageScreenRect(0).top()) {
        QRectF r0 = pageScreenRect(0);
        if (outPagePos) {
            outPagePos->setX((screenPos.x() - r0.left()) / m_zoom);
            outPagePos->setY((screenPos.y() - r0.top()) / m_zoom);
        }
        return 0;
    }

    // 3. In-between gap check
    for (int i = 0; i < count - 1; ++i) {
        QRectF curr = pageScreenRect(i);
        QRectF next = pageScreenRect(i + 1);
        if (screenPos.y() > curr.bottom() && screenPos.y() < next.top()) {
            // Assign to closest page
            const bool closerToNext = (screenPos.y() - curr.bottom()) > (next.top() - screenPos.y());
            const int targetIdx = closerToNext ? (i + 1) : i;
            QRectF targetRect = pageScreenRect(targetIdx);
            if (outPagePos) {
                outPagePos->setX((screenPos.x() - targetRect.left()) / m_zoom);
                outPagePos->setY((screenPos.y() - targetRect.top()) / m_zoom);
            }
            return targetIdx;
        }
    }

    // 4. Below last page
    const int lastIdx = count - 1;
    QRectF rLast = pageScreenRect(lastIdx);
    if (outPagePos) {
        outPagePos->setX((screenPos.x() - rLast.left()) / m_zoom);
        outPagePos->setY((screenPos.y() - rLast.top()) / m_zoom);
    }
    return lastIdx;
}

void CanvasItem::checkAutoExpand() {
    if (!m_backend || !m_backend->isSplinkNoteOrigin() || m_backend->pageCount() == 0)
        return;

    const int count = m_backend->pageCount();
    QRectF lastRect = pageScreenRect(count - 1);
    // If the bottom of the last page is within 600px of viewport bottom, expand
    if (lastRect.bottom() <= height() + 600.0) {
        m_backend->autoExpandPages(count + 1);
    }
}

void CanvasItem::updateActivePageIndexFromPan() {
    if (!m_backend || m_backend->pageCount() <= 1)
        return;

    const QPointF center(width() * 0.5, height() * 0.5);
    const int centerIdx = pageIndexAtScreenPos(center);
    if (centerIdx >= 0 && centerIdx != m_backend->currentPageIndex()) {
        m_backend->setCurrentPageIndex(centerIdx);
    }
}

void CanvasItem::paint(QPainter *painter) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::LosslessImageRendering, true);

    // Workspace background filled with Omarchy theme tone
    painter->fillRect(boundingRect(), m_backgroundColor);

    const qreal viewH = height();
    const int count = m_backend->pageCount();

    // Continuous Multi-Page Vertical Loop with Frustum Culling
    for (int i = 0; i < count; ++i) {
        const QRectF r = pageScreenRect(i);

        // Frustum culling: skip pages completely outside the visible viewport
        if (r.bottom() < -60.0 || r.top() > viewH + 60.0)
            continue;

        // Ambient page drop shadow
        painter->fillRect(QRectF(r.left() - 1, r.top() + 2, r.width() + 2, r.height() + 6), QColor(0, 0, 0, 80));
        painter->fillRect(QRectF(r.left() - 2, r.top() + 4, r.width() + 4, r.height() + 8), QColor(0, 0, 0, 40));

        // Crisp white paper surface
        painter->fillRect(r, QColor("#ffffff"));

        // Subtle paper border outline
        painter->setPen(QPen(QColor(180, 185, 195), 1.0));
        painter->drawRect(r);

        // Page number tag above page header
        if (count > 1) {
            painter->save();
            painter->setPen(QColor(140, 145, 160));
            QFont pageTagFont(QStringLiteral("JetBrainsMono Nerd Font"));
            pageTagFont.setPixelSize(10);
            pageTagFont.setBold(true);
            painter->setFont(pageTagFont);
            painter->drawText(QPointF(r.left() + 4, r.top() - 8),
                              QStringLiteral("PAGE %1 OF %2").arg(i + 1).arg(count));
            painter->restore();
        }

        const DocumentPage *page = m_backend->pageAt(i);
        if (!page)
            continue;

        // High-DPI Poppler PDF background (300 DPI supersampling)
        if (!page->pdfBackground.isNull()) {
            painter->drawImage(r, page->pdfBackground);
        }

        // Vector Inking for this page
        painter->save();
        painter->translate(r.topLeft());
        painter->scale(m_zoom, m_zoom);

        // Pass 1: Highlighters (drawn beneath pen ink with uniform SourceOver alpha)
        for (const Stroke &s : page->strokes) {
            if (s.tool != 1 || s.points.isEmpty())
                continue;
            QPainterPath path = buildSmoothPath(s.points);
            QColor hlColor = s.color;
            hlColor.setAlphaF(0.40f);
            painter->setPen(QPen(hlColor, s.width, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
            painter->drawPath(path);
        }

        // Active in-flight highlighter on this page
        if (m_isDrawing && m_activePageIndex == i && m_activeStroke.tool == 1 && !m_activeStroke.points.isEmpty()) {
            QPainterPath path = buildSmoothPath(m_activeStroke.points);
            QColor hlColor = m_activeStroke.color;
            hlColor.setAlphaF(0.40f);
            painter->setPen(QPen(hlColor, m_activeStroke.width, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
            painter->drawPath(path);
        }

        // Pass 2: Pen strokes (crisp vector curves on top)
        for (const Stroke &s : page->strokes) {
            if (s.tool != 0 || s.points.isEmpty())
                continue;
            QPainterPath path = buildSmoothPath(s.points);
            painter->setPen(QPen(s.color, s.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(path);
        }

        // Active in-flight pen on this page
        if (m_isDrawing && m_activePageIndex == i && m_activeStroke.tool == 0 && !m_activeStroke.points.isEmpty()) {
            QPainterPath path = buildSmoothPath(m_activeStroke.points);
            painter->setPen(QPen(m_activeStroke.color, m_activeStroke.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(path);
        }

        painter->restore();
    }

    // Swept Eraser indicator ring (rendered in screen coordinates)
    const bool isEraserMode = (m_backend->currentTool() == 2) || m_hardwareEraserActive || m_isErasing;
    if ((isEraserMode || m_showEraserHover) && !m_hoverPos.isNull()) {
        const float r = m_backend->eraserRadius() * static_cast<float>(m_zoom);
        painter->save();
        painter->setPen(QPen(QColor(240, 80, 80, 220), 1.5, Qt::DashLine));
        painter->setBrush(QColor(240, 80, 80, 40));
        painter->drawEllipse(m_hoverPos, r, r);
        painter->restore();
    }
}

void CanvasItem::handlePointerPress(const QPointF &pos, float pressure, bool isRightOrBarrel) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    m_hoverPos = pos;
    QPointF pagePos;
    m_activePageIndex = pageIndexAtScreenPos(pos, &pagePos);
    if (m_activePageIndex < 0)
        return;

    m_lastPagePos = pagePos;

    const bool isEraser = (m_backend->currentTool() == 2) || isRightOrBarrel || m_hardwareEraserActive;

    if (isEraser) {
        m_isErasing = true;
        m_isDrawing = false;
        m_backend->eraseStrokes(m_activePageIndex, pagePos, pagePos, m_backend->eraserRadius());
        update();
        return;
    }

    m_isDrawing = true;
    m_isErasing = false;

    m_activeStroke.id = 0;
    m_activeStroke.tool = m_backend->currentTool();
    m_activeStroke.color = (m_activeStroke.tool == 1) ? m_backend->highlighterColor() : m_backend->penColor();

    float baseW = (m_activeStroke.tool == 1) ? m_backend->highlighterWidth() : m_backend->penWidth();
    if (m_activeStroke.tool == 0 && pressure > 0.0f) {
        baseW *= std::clamp(0.35f + 1.3f * pressure, 0.35f, 2.0f);
    }
    m_activeStroke.width = baseW;
    m_activeStroke.points.clear();

    StrokePoint pt;
    pt.x = static_cast<float>(pagePos.x());
    pt.y = static_cast<float>(pagePos.y());
    pt.pressure = pressure;
    m_activeStroke.points.append(pt);

    checkAutoExpand();
    update();
}

void CanvasItem::handlePointerMove(const QPointF &pos, float pressure) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    m_hoverPos = pos;
    QPointF pagePos;
    const int pageIdx = pageIndexAtScreenPos(pos, &pagePos);

    if (m_isErasing) {
        const int targetPage = (pageIdx >= 0) ? pageIdx : m_activePageIndex;
        m_backend->eraseStrokes(targetPage, m_lastPagePos, pagePos, m_backend->eraserRadius());
        m_lastPagePos = pagePos;
        update();
        return;
    }

    if (m_isDrawing) {
        StrokePoint pt;
        pt.x = static_cast<float>(pagePos.x());
        pt.y = static_cast<float>(pagePos.y());
        pt.pressure = pressure;
        m_activeStroke.points.append(pt);
        m_lastPagePos = pagePos;
        checkAutoExpand();
        update();
    }
}

void CanvasItem::handlePointerRelease(const QPointF &pos) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    m_hoverPos = pos;

    if (m_isErasing) {
        m_isErasing = false;
        update();
        return;
    }

    if (m_isDrawing) {
        m_isDrawing = false;
        QPointF pagePos;
        pageIndexAtScreenPos(pos, &pagePos);

        if (m_activeStroke.points.isEmpty() ||
            (m_activeStroke.points.last().x != pagePos.x() || m_activeStroke.points.last().y != pagePos.y())) {
            StrokePoint pt;
            pt.x = static_cast<float>(pagePos.x());
            pt.y = static_cast<float>(pagePos.y());
            pt.pressure = 0.5f;
            m_activeStroke.points.append(pt);
        }

        m_backend->commitStroke(m_activePageIndex, m_activeStroke);
        m_activeStroke.points.clear();
        checkAutoExpand();
        update();
    }
}

bool CanvasItem::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::NativeGesture: {
        auto *ge = static_cast<QNativeGestureEvent *>(event);
        if (ge->gestureType() == Qt::ZoomNativeGesture) {
            const qreal scaleFactor = 1.0 + ge->value();
            if (scaleFactor > 0.05 && scaleFactor < 20.0) {
                const qreal oldZoom = m_zoom;
                const qreal newZoom = std::clamp(oldZoom * scaleFactor, 0.25, 4.0);
                const QPointF focalPos = ge->position();
                m_panY = focalPos.y() - (focalPos.y() - m_panY) * (newZoom / oldZoom);
                m_zoom = newZoom;
                emit zoomChanged();
                emit panChanged();
                updateActivePageIndexFromPan();
                checkAutoExpand();
                update();
            }
            ge->accept();
            return true;
        }
        break;
    }
    case QEvent::TabletPress: {
        auto *te = static_cast<QTabletEvent *>(event);
        float p = static_cast<float>(te->pressure());
        if (p <= 0.001f) p = 0.5f;

        const bool isHardwareEraser = (te->pointerType() == QPointingDevice::PointerType::Eraser);
        m_hardwareEraserActive = isHardwareEraser;
        const bool isSideButton = (te->buttons() & (Qt::RightButton | Qt::TaskButton));

        handlePointerPress(te->position(), p, isHardwareEraser || isSideButton);
        event->accept();
        return true;
    }
    case QEvent::TabletMove: {
        auto *te = static_cast<QTabletEvent *>(event);
        float p = static_cast<float>(te->pressure());
        if (p <= 0.001f) p = 0.5f;

        const bool isHardwareEraser = (te->pointerType() == QPointingDevice::PointerType::Eraser);
        m_hardwareEraserActive = isHardwareEraser;

        handlePointerMove(te->position(), p);
        event->accept();
        return true;
    }
    case QEvent::TabletRelease: {
        auto *te = static_cast<QTabletEvent *>(event);
        handlePointerRelease(te->position());
        m_hardwareEraserActive = false;
        event->accept();
        return true;
    }
    default:
        break;
    }
    return QQuickPaintedItem::event(event);
}

void CanvasItem::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        const qreal factor = (event->angleDelta().y() > 0) ? 1.15 : 0.85;
        const qreal oldZoom = m_zoom;
        const qreal newZoom = std::clamp(oldZoom * factor, 0.25, 4.0);
        const QPointF mousePos = event->position();
        m_panY = mousePos.y() - (mousePos.y() - m_panY) * (newZoom / oldZoom);
        m_zoom = newZoom;
        emit zoomChanged();
        emit panChanged();
        updateActivePageIndexFromPan();
        checkAutoExpand();
        update();
        event->accept();
        return;
    }

    setPanY(m_panY + event->angleDelta().y() * 0.8);
    event->accept();
}

void CanvasItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPos = event->position();
        event->accept();
        return;
    }

    const bool isRight = (event->button() == Qt::RightButton);
    handlePointerPress(event->position(), 0.5f, isRight);
    event->accept();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPanning) {
        const QPointF delta = event->position() - m_lastPanPos;
        m_lastPanPos = event->position();
        setPanX(m_panX + delta.x());
        setPanY(m_panY + delta.y());
        event->accept();
        return;
    }

    handlePointerMove(event->position(), 0.5f);
    event->accept();
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        event->accept();
        return;
    }

    handlePointerRelease(event->position());
    event->accept();
}

void CanvasItem::touchEvent(QTouchEvent *event) {
    const auto &points = event->points();
    if (points.isEmpty())
        return;

    // Two-finger touch: Pinch-to-zoom and Pan
    if (points.size() >= 2) {
        if (m_isDrawing) {
            m_isDrawing = false;
            m_activeStroke.points.clear();
        }

        const QPointF p0 = points[0].position();
        const QPointF p1 = points[1].position();
        const qreal dist = std::max<qreal>(10.0, std::hypot(p1.x() - p0.x(), p1.y() - p0.y()));
        const QPointF centroid = (p0 + p1) * 0.5;

        if (!m_isPinching) {
            m_isPinching = true;
            m_pinchStartDist = dist;
            m_pinchStartZoom = m_zoom;
            m_pinchStartCentroid = centroid;
        } else {
            const qreal scale = dist / m_pinchStartDist;
            const qreal oldZoom = m_zoom;
            const qreal newZoom = std::clamp(m_pinchStartZoom * scale, 0.25, 4.0);

            // Zoom around gesture centroid
            const qreal ratio = newZoom / oldZoom;
            m_panX = centroid.x() - (centroid.x() - m_panX) * ratio;
            m_panY = centroid.y() - (centroid.y() - m_panY) * ratio;

            // Pan delta
            const QPointF panDelta = centroid - m_pinchStartCentroid;
            m_panX += panDelta.x();
            m_panY += panDelta.y();
            m_pinchStartCentroid = centroid;

            m_zoom = newZoom;
            emit zoomChanged();
            emit panChanged();
            updateActivePageIndexFromPan();
            checkAutoExpand();
            update();
        }
        event->accept();
        return;
    }

    m_isPinching = false;

    // Single touch point: drawing or erasing
    const auto &pt = points.first();
    const float pressure = pt.pressure() > 0.01f ? static_cast<float>(pt.pressure()) : 0.5f;

    switch (pt.state()) {
    case QEventPoint::Pressed:
        handlePointerPress(pt.position(), pressure, false);
        break;
    case QEventPoint::Updated:
        handlePointerMove(pt.position(), pressure);
        break;
    case QEventPoint::Released:
        handlePointerRelease(pt.position());
        break;
    default:
        break;
    }
    event->accept();
}

void CanvasItem::hoverMoveEvent(QHoverEvent *event) {
    m_hoverPos = event->position();
    m_showEraserHover = (m_backend && m_backend->currentTool() == 2) || m_hardwareEraserActive;
    if (m_showEraserHover) {
        update();
    }
    event->accept();
}

void CanvasItem::hoverLeaveEvent(QHoverEvent *event) {
    m_showEraserHover = false;
    m_hoverPos = QPointF();
    update();
    event->accept();
}
