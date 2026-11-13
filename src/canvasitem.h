#pragma once

#include <QColor>
#include <QPainterPath>
#include <QPointF>
#include <QQuickPaintedItem>
#include <QRectF>
#include <QVector>

#include "backend.h"

class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY(Backend *backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(qreal panX READ panX WRITE setPanX NOTIFY panChanged)
    Q_PROPERTY(qreal panY READ panY WRITE setPanY NOTIFY panChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);
    ~CanvasItem() override = default;

    Backend *backend() const { return m_backend; }
    void setBackend(Backend *b);

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal z);

    qreal panX() const { return m_panX; }
    void setPanX(qreal x);

    qreal panY() const { return m_panY; }
    void setPanY(qreal y);

    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor &color);

    void paint(QPainter *painter) override;

    Q_INVOKABLE void resetView();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void fitToWidth(qreal viewWidth, qreal viewHeight);
    Q_INVOKABLE void scrollToPage(int pageIndex);

    QRectF pageScreenRect(int index) const;
    int pageIndexAtScreenPos(const QPointF &screenPos, QPointF *outPagePos = nullptr) const;

signals:
    void backendChanged();
    void zoomChanged();
    void panChanged();
    void backgroundColorChanged();

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void touchEvent(QTouchEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    void checkAutoExpand();
    void updateActivePageIndexFromPan();
    void handlePointerPress(const QPointF &pos, float pressure, bool isRightOrBarrel);
    void handlePointerMove(const QPointF &pos, float pressure);
    void handlePointerRelease(const QPointF &pos);

    Backend *m_backend = nullptr;
    qreal m_zoom = 1.0;
    qreal m_panX = 0.0;
    qreal m_panY = 0.0;
    QColor m_backgroundColor = QColor("#1a1b26");

    bool m_isDrawing = false;
    bool m_isErasing = false;
    bool m_isPanning = false;
    QPointF m_lastPanPos;
    QPointF m_lastPagePos;
    int m_activePageIndex = 0;

    QPointF m_hoverPos;
    bool m_showEraserHover = false;
    bool m_hardwareEraserActive = false;

    // Two-finger pinch state
    bool m_isPinching = false;
    qreal m_pinchStartDist = 1.0;
    qreal m_pinchStartZoom = 1.0;
    QPointF m_pinchStartCentroid;

    Stroke m_activeStroke;
};
