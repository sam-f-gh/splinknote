#include "canvasitem.h"

#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTabletEvent>
#include <QTouchEvent>
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
    }

    emit backendChanged();
    update();
}

void CanvasItem::setZoom(qreal z) {
    z = std::clamp(z, 0.25, 4.0);
    if (!qFuzzyCompare(m_zoom, z)) {
        m_zoom = z;
        emit zoomChanged();
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
    setZoom(m_zoom * 1.2);
}

void CanvasItem::zoomOut() {
    setZoom(m_zoom / 1.2);
}

void CanvasItem::fitToWidth(qreal viewWidth, qreal viewHeight) {
    if (!m_backend || viewWidth <= 0 || viewHeight <= 0)
        return;
    const qreal pageW = m_backend->pageWidth();
    if (pageW <= 0)
        return;

    const qreal targetZoom = (viewWidth - 32.0) / pageW;
    setZoom(std::clamp(targetZoom, 0.35, 2.5));
    m_panX = 0.0;
    m_panY = 0.0;
    emit panChanged();
    update();
}

QPointF CanvasItem::mapToPage(const QPointF &itemPos) const {
    if (!m_backend)
        return itemPos;

    const qreal pageW = m_backend->pageWidth();
    const qreal scaledW = pageW * m_zoom;

    qreal originX = m_panX;
    if (width() > scaledW) {
        originX += (width() - scaledW) * 0.5;
    }
    const qreal originY = m_panY + 20.0;

    return QPointF((itemPos.x() - originX) / m_zoom, (itemPos.y() - originY) / m_zoom);
}

void CanvasItem::paint(QPainter *painter) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    const DocumentPage *page = m_backend->currentPage();
    if (!page)
        return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const qreal pageW = page->width;
    const qreal pageH = page->height;
    const qreal scaledW = pageW * m_zoom;

    qreal originX = m_panX;
    if (width() > scaledW) {
        originX += (width() - scaledW) * 0.5;
    }
    const qreal originY = m_panY + 20.0;

    painter->save();
    painter->translate(originX, originY);
    painter->scale(m_zoom, m_zoom);

    // Subtle paper shadow
    painter->fillRect(QRectF(3.0, 3.0, pageW, pageH), QColor(0, 0, 0, 45));

    // Crisp paper surface
    painter->fillRect(QRectF(0, 0, pageW, pageH), QColor("#ffffff"));

    // Draw background if loaded from external PDF
    if (!page->pdfBackground.isNull()) {
        painter->drawImage(QRectF(0, 0, pageW, pageH), page->pdfBackground);
    }

    // Committed vector strokes
    for (const Stroke &s : page->strokes) {
        if (s.points.isEmpty())
            continue;

        QPainterPath path = buildSmoothPath(s.points);
        if (s.tool == 1) { // Highlighter
            painter->save();
            painter->setCompositionMode(QPainter::CompositionMode_Multiply);
            QColor hlColor = s.color;
            hlColor.setAlphaF(0.40f);
            painter->setPen(QPen(hlColor, s.width, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
            painter->drawPath(path);
            painter->restore();
        } else { // Pen
            painter->setPen(QPen(s.color, s.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(path);
        }
    }

    // Active in-flight stroke
    if (m_isDrawing && !m_activeStroke.points.isEmpty()) {
        QPainterPath path = buildSmoothPath(m_activeStroke.points);
        if (m_activeStroke.tool == 1) { // Highlighter
            painter->save();
            painter->setCompositionMode(QPainter::CompositionMode_Multiply);
            QColor hlColor = m_activeStroke.color;
            hlColor.setAlphaF(0.40f);
            painter->setPen(QPen(hlColor, m_activeStroke.width, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
            painter->drawPath(path);
            painter->restore();
        } else { // Pen
            painter->setPen(QPen(m_activeStroke.color, m_activeStroke.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(path);
        }
    }

    // Active eraser indicator
    if (m_isErasing) {
        const float r = m_backend->eraserRadius();
        painter->save();
        painter->setPen(QPen(QColor(240, 80, 80, 200), 1.5 / m_zoom, Qt::DashLine));
        painter->setBrush(QColor(240, 80, 80, 40));
        painter->drawEllipse(m_lastPagePos, r, r);
        painter->restore();
    }

    painter->restore();
}

void CanvasItem::handlePointerPress(const QPointF &pos, float pressure, bool isRightOrBarrel) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    const QPointF pagePos = mapToPage(pos);
    m_lastPagePos = pagePos;

    // Check if tool is Eraser, or if hardware eraser / right-click is engaged
    const bool isEraser = (m_backend->currentTool() == 2) || isRightOrBarrel;

    if (isEraser) {
        m_isErasing = true;
        m_isDrawing = false;
        m_backend->eraseStrokes(pagePos, pagePos, m_backend->eraserRadius());
        update();
        return;
    }

    // Drawing mode (Pen or Highlighter)
    m_isDrawing = true;
    m_isErasing = false;

    m_activeStroke.id = 0;
    m_activeStroke.tool = m_backend->currentTool();
    m_activeStroke.color = (m_activeStroke.tool == 1) ? QColor("#FFD60A") : m_backend->penColor();

    float baseW = (m_activeStroke.tool == 1) ? m_backend->highlighterWidth() : m_backend->penWidth();
    if (m_activeStroke.tool == 0 && pressure > 0.0f) {
        // Dynamic pressure modulation for real ink feel
        baseW *= std::clamp(0.35f + 1.3f * pressure, 0.35f, 2.0f);
    }
    m_activeStroke.width = baseW;
    m_activeStroke.points.clear();

    StrokePoint pt;
    pt.x = static_cast<float>(pagePos.x());
    pt.y = static_cast<float>(pagePos.y());
    pt.pressure = pressure;
    m_activeStroke.points.append(pt);

    update();
}

void CanvasItem::handlePointerMove(const QPointF &pos, float pressure) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    const QPointF pagePos = mapToPage(pos);

    if (m_isErasing) {
        m_backend->eraseStrokes(m_lastPagePos, pagePos, m_backend->eraserRadius());
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
        update();
    }
}

void CanvasItem::handlePointerRelease(const QPointF &pos) {
    if (!m_backend || !m_backend->hasDocument())
        return;

    if (m_isErasing) {
        m_isErasing = false;
        update();
        return;
    }

    if (m_isDrawing) {
        m_isDrawing = false;
        const QPointF pagePos = mapToPage(pos);
        if (m_activeStroke.points.isEmpty() ||
            (m_activeStroke.points.last().x != pagePos.x() || m_activeStroke.points.last().y != pagePos.y())) {
            StrokePoint pt;
            pt.x = static_cast<float>(pagePos.x());
            pt.y = static_cast<float>(pagePos.y());
            pt.pressure = 0.5f;
            m_activeStroke.points.append(pt);
        }

        m_backend->commitStroke(m_activeStroke);
        m_activeStroke.points.clear();
        update();
    }
}

bool CanvasItem::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::TabletPress: {
        auto *te = static_cast<QTabletEvent *>(event);
        float p = static_cast<float>(te->pressure());
        if (p <= 0.001f) p = 0.5f;
        const bool isEraser = (te->pointerType() == QPointingDevice::PointerType::Eraser) ||
                              (te->buttons() & Qt::RightButton);
        handlePointerPress(te->position(), p, isEraser);
        event->accept();
        return true;
    }
    case QEvent::TabletMove: {
        auto *te = static_cast<QTabletEvent *>(event);
        float p = static_cast<float>(te->pressure());
        if (p <= 0.001f) p = 0.5f;
        handlePointerMove(te->position(), p);
        event->accept();
        return true;
    }
    case QEvent::TabletRelease: {
        auto *te = static_cast<QTabletEvent *>(event);
        handlePointerRelease(te->position());
        event->accept();
        return true;
    }
    default:
        break;
    }
    return QQuickPaintedItem::event(event);
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
    event->accept();
}
