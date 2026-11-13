#pragma once

#include <QColor>
#include <QImage>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>

class FilePicker;

struct StrokePoint {
    float x = 0.0f;
    float y = 0.0f;
    float pressure = 0.5f;
};

struct Stroke {
    quint64 id = 0;
    int tool = 0; // 0 = Pen, 1 = Highlighter
    QColor color;
    float width = 2.0f;
    QVector<StrokePoint> points;
};

struct DocumentPage {
    int id = 1;
    float width = 595.28f;  // Standard A4 width (72 dpi pt)
    float height = 842.0f;  // Standard A4 height (72 dpi pt)
    QImage pdfBackground;   // Underlying page image if loaded from external PDF
    QVector<Stroke> strokes;
};

enum class HistoryType {
    AddStroke,
    EraseStrokes,
    AddPage,
    DeletePage,
    ClearPage
};

struct HistoryCommand {
    HistoryType type;
    int pageIndex = 0;
    QVector<Stroke> strokes;
    DocumentPage deletedPage;
};

class Backend : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool hasDocument READ hasDocument NOTIFY hasDocumentChanged)
    Q_PROPERTY(int currentTool READ currentTool WRITE setCurrentTool NOTIFY toolChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int effectivePageCount READ effectivePageCount NOTIFY effectivePageCountChanged)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex WRITE setCurrentPageIndex NOTIFY currentPageIndexChanged)
    Q_PROPERTY(float pageWidth READ pageWidth NOTIFY pageDimensionsChanged)
    Q_PROPERTY(float pageHeight READ pageHeight NOTIFY pageDimensionsChanged)
    Q_PROPERTY(bool isSplinkNoteOrigin READ isSplinkNoteOrigin NOTIFY isSplinkNoteOriginChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY hasUnsavedChangesChanged)
    Q_PROPERTY(QString currentFilePath READ currentFilePath NOTIFY currentFilePathChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged)
    Q_PROPERTY(float penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
    Q_PROPERTY(QColor highlighterColor READ highlighterColor WRITE setHighlighterColor NOTIFY highlighterColorChanged)
    Q_PROPERTY(float highlighterWidth READ highlighterWidth WRITE setHighlighterWidth NOTIFY highlighterWidthChanged)
    Q_PROPERTY(float eraserRadius READ eraserRadius WRITE setEraserRadius NOTIFY eraserRadiusChanged)

public:
    explicit Backend(FilePicker *filePicker = nullptr, QObject *parent = nullptr);
    ~Backend() override;

    bool hasDocument() const { return m_hasDocument; }
    int currentTool() const { return m_currentTool; }
    int pageCount() const { return m_pages.size(); }
    int effectivePageCount() const;
    int currentPageIndex() const { return m_currentPageIndex; }
    float pageWidth() const;
    float pageHeight() const;
    bool isSplinkNoteOrigin() const { return m_isSplinkNoteOrigin; }
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }
    QString currentFilePath() const { return m_currentFilePath; }
    QString status() const { return m_status; }
    QColor penColor() const { return m_penColor; }
    float penWidth() const { return m_penWidth; }
    QColor highlighterColor() const { return m_highlighterColor; }
    float highlighterWidth() const { return m_highlighterWidth; }
    float eraserRadius() const { return m_eraserRadius; }

    // Direct access for CanvasItem
    const DocumentPage *currentPage() const;
    DocumentPage *currentPage();
    const QVector<DocumentPage> &pages() const { return m_pages; }
    DocumentPage *pageAt(int index);
    const DocumentPage *pageAt(int index) const;

    qreal pageTopWorld(int pageIndex) const;
    qreal totalDocumentHeightWorld() const;

    Q_INVOKABLE void newDocument();
    Q_INVOKABLE void openDocument();
    Q_INVOKABLE void closeDocument();
    Q_INVOKABLE void saveDocument();
    Q_INVOKABLE bool openPdfFile(const QString &filePath);
    Q_INVOKABLE bool savePdfFile(const QString &filePath);

    Q_INVOKABLE void setCurrentTool(int tool);
    Q_INVOKABLE void setCurrentPageIndex(int index);
    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void previousPage();
    Q_INVOKABLE void addPage();
    Q_INVOKABLE void deleteCurrentPage();
    Q_INVOKABLE void clearCurrentPage();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void setPenColor(const QColor &color);
    Q_INVOKABLE void setPenWidth(float w);
    Q_INVOKABLE void setHighlighterColor(const QColor &color);
    Q_INVOKABLE void setHighlighterWidth(float w);
    Q_INVOKABLE void setEraserRadius(float r);
    Q_INVOKABLE void setStatus(const QString &msg);
    Q_INVOKABLE void autoExpandPages(int targetCount);

    // Inking and stroke methods
    void commitStroke(const Stroke &stroke);
    void commitStroke(int pageIndex, const Stroke &stroke);
    bool eraseStrokes(const QPointF &p1, const QPointF &p2, float radius);
    bool eraseStrokes(int pageIndex, const QPointF &p1, const QPointF &p2, float radius);

signals:
    void hasDocumentChanged();
    void toolChanged();
    void pageCountChanged();
    void effectivePageCountChanged();
    void currentPageIndexChanged();
    void pageDimensionsChanged();
    void isSplinkNoteOriginChanged();
    void canUndoChanged();
    void canRedoChanged();
    void hasUnsavedChangesChanged();
    void currentFilePathChanged();
    void statusChanged();
    void penColorChanged();
    void penWidthChanged();
    void highlighterColorChanged();
    void highlighterWidthChanged();
    void eraserRadiusChanged();
    void canvasNeedsRepaint();

private:
    void pushUndoCommand(const HistoryCommand &cmd);
    void clearHistory();

    FilePicker *m_filePicker = nullptr;
    bool m_hasDocument = false;
    int m_currentTool = 0; // 0 = Pen, 1 = Highlighter, 2 = Eraser
    int m_currentPageIndex = 0;
    bool m_isSplinkNoteOrigin = true;
    bool m_hasUnsavedChanges = false;
    QString m_currentFilePath;
    QString m_status;

    QColor m_penColor = QColor("#18181c"); // True ink black default
    float m_penWidth = 2.5f;
    QColor m_highlighterColor = QColor("#FFD60A");
    float m_highlighterWidth = 18.0f;
    float m_eraserRadius = 3.0f; // Fine precision point radius

    QVector<DocumentPage> m_pages;
    QVector<HistoryCommand> m_undoStack;
    QVector<HistoryCommand> m_redoStack;
    quint64 m_nextStrokeId = 1;
};
