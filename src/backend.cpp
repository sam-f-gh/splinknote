#include "backend.h"
#include "filepicker.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QSaveFile>
#include <algorithm>
#include <cmath>
#include <poppler-qt6.h>

namespace {
struct Pt { float x, y; };

float ptSegmentDistSq(const Pt &p, const Pt &a, const Pt &b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-6f) {
        float px = p.x - a.x;
        float py = p.y - a.y;
        return px * px + py * py;
    }
    float t = std::max(0.0f, std::min(1.0f, ((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq));
    float projX = a.x + t * dx;
    float projY = a.y + t * dy;
    float ex = p.x - projX;
    float ey = p.y - projY;
    return ex * ex + ey * ey;
}

bool segmentsIntersect(const Pt &p1, const Pt &p2, const Pt &q1, const Pt &q2) {
    auto cross = [](const Pt &a, const Pt &b, const Pt &c) -> float {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };
    float c1 = cross(p1, p2, q1);
    float c2 = cross(p1, p2, q2);
    float c3 = cross(q1, q2, p1);
    float c4 = cross(q1, q2, p2);
    if (((c1 > 0 && c2 < 0) || (c1 < 0 && c2 > 0)) &&
        ((c3 > 0 && c4 < 0) || (c3 < 0 && c4 > 0)))
        return true;
    return false;
}

float segmentSegmentDistSq(const Pt &p1, const Pt &p2, const Pt &q1, const Pt &q2) {
    if (segmentsIntersect(p1, p2, q1, q2))
        return 0.0f;
    float d1 = ptSegmentDistSq(p1, q1, q2);
    float d2 = ptSegmentDistSq(p2, q1, q2);
    float d3 = ptSegmentDistSq(q1, p1, p2);
    float d4 = ptSegmentDistSq(q2, p1, p2);
    return std::min({d1, d2, d3, d4});
}

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

Backend::Backend(FilePicker *filePicker, QObject *parent)
    : QObject(parent), m_filePicker(filePicker) {
    if (m_filePicker) {
        connect(m_filePicker, &FilePicker::pdfSelected, this, [this](const QUrl &url) {
            const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
            openPdfFile(path);
        });
        connect(m_filePicker, &FilePicker::saveSelected, this, [this](const QUrl &url) {
            const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
            savePdfFile(path);
        });
        connect(m_filePicker, &FilePicker::failed, this, [this](const QString &err) {
            setStatus(err);
        });
    }
}

Backend::~Backend() = default;

float Backend::pageWidth() const {
    const DocumentPage *page = currentPage();
    return page ? page->width : 595.28f;
}

float Backend::pageHeight() const {
    const DocumentPage *page = currentPage();
    return page ? page->height : 842.0f;
}

const DocumentPage *Backend::currentPage() const {
    if (m_currentPageIndex >= 0 && m_currentPageIndex < m_pages.size())
        return &m_pages[m_currentPageIndex];
    return nullptr;
}

DocumentPage *Backend::currentPage() {
    if (m_currentPageIndex >= 0 && m_currentPageIndex < m_pages.size())
        return &m_pages[m_currentPageIndex];
    return nullptr;
}

void Backend::newDocument() {
    m_pages.clear();
    clearHistory();

    DocumentPage firstPage;
    firstPage.id = 1;
    firstPage.width = 595.28f; // A4
    firstPage.height = 842.0f;
    m_pages.append(firstPage);

    m_currentPageIndex = 0;
    m_hasDocument = true;
    m_isSplinkNoteOrigin = true;
    m_hasUnsavedChanges = false;
    m_currentFilePath.clear();

    emit hasDocumentChanged();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit isSplinkNoteOriginChanged();
    emit hasUnsavedChangesChanged();
    emit currentFilePathChanged();
    emit canvasNeedsRepaint();

    setStatus(tr("New note started"));
}

void Backend::openDocument() {
    if (m_filePicker) {
        m_filePicker->openPdf();
    }
}

void Backend::closeDocument() {
    m_pages.clear();
    clearHistory();
    m_hasDocument = false;
    m_currentPageIndex = 0;
    m_currentFilePath.clear();
    m_hasUnsavedChanges = false;

    emit hasDocumentChanged();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit currentFilePathChanged();
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();

    setStatus(tr("Document closed"));
}

void Backend::saveDocument() {
    if (m_filePicker) {
        QUrl suggested;
        if (!m_currentFilePath.isEmpty()) {
            suggested = QUrl::fromLocalFile(m_currentFilePath);
        } else {
            suggested = QUrl::fromLocalFile(QDir::homePath() + "/note.pdf");
        }
        m_filePicker->savePdf(suggested);
    }
}

void Backend::setCurrentTool(int tool) {
    if (tool < 0 || tool > 2 || tool == m_currentTool)
        return;
    m_currentTool = tool;
    emit toolChanged();

    if (m_currentTool == 0)
        setStatus(tr("Tool: Pen"));
    else if (m_currentTool == 1)
        setStatus(tr("Tool: Highlighter"));
    else if (m_currentTool == 2)
        setStatus(tr("Tool: Eraser"));
}

void Backend::setCurrentPageIndex(int index) {
    if (index >= 0 && index < m_pages.size() && index != m_currentPageIndex) {
        m_currentPageIndex = index;
        emit currentPageIndexChanged();
        emit pageDimensionsChanged();
        emit canvasNeedsRepaint();
    }
}

void Backend::nextPage() {
    if (m_currentPageIndex < m_pages.size() - 1) {
        setCurrentPageIndex(m_currentPageIndex + 1);
    }
}

void Backend::previousPage() {
    if (m_currentPageIndex > 0) {
        setCurrentPageIndex(m_currentPageIndex - 1);
    }
}

void Backend::addPage() {
    if (!m_hasDocument)
        return;

    DocumentPage newPage;
    newPage.id = m_pages.size() + 1;
    const DocumentPage *cur = currentPage();
    if (cur) {
        newPage.width = cur->width;
        newPage.height = cur->height;
    }
    m_pages.append(newPage);
    m_currentPageIndex = m_pages.size() - 1;

    HistoryCommand cmd;
    cmd.type = HistoryType::AddPage;
    cmd.pageIndex = m_currentPageIndex;
    pushUndoCommand(cmd);

    m_hasUnsavedChanges = true;
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();
    setStatus(tr("Added page %1").arg(m_pages.size()));
}

void Backend::deleteCurrentPage() {
    if (m_pages.size() <= 1) {
        clearCurrentPage();
        return;
    }

    HistoryCommand cmd;
    cmd.type = HistoryType::DeletePage;
    cmd.pageIndex = m_currentPageIndex;
    cmd.deletedPage = m_pages[m_currentPageIndex];
    pushUndoCommand(cmd);

    m_pages.removeAt(m_currentPageIndex);
    if (m_currentPageIndex >= m_pages.size())
        m_currentPageIndex = m_pages.size() - 1;

    m_hasUnsavedChanges = true;
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();
    setStatus(tr("Deleted page"));
}

void Backend::clearCurrentPage() {
    DocumentPage *page = currentPage();
    if (!page || page->strokes.isEmpty())
        return;

    HistoryCommand cmd;
    cmd.type = HistoryType::ClearPage;
    cmd.pageIndex = m_currentPageIndex;
    cmd.strokes = page->strokes;
    pushUndoCommand(cmd);

    page->strokes.clear();
    m_hasUnsavedChanges = true;
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();
    setStatus(tr("Cleared page"));
}

void Backend::setPenColor(const QColor &color) {
    if (color.isValid() && color != m_penColor) {
        m_penColor = color;
        emit penColorChanged();
    }
}

void Backend::setStatus(const QString &msg) {
    if (msg != m_status) {
        m_status = msg;
        emit statusChanged();
    }
}

void Backend::commitStroke(const Stroke &stroke) {
    DocumentPage *page = currentPage();
    if (!page)
        return;

    Stroke s = stroke;
    s.id = m_nextStrokeId++;
    page->strokes.append(s);

    HistoryCommand cmd;
    cmd.type = HistoryType::AddStroke;
    cmd.pageIndex = m_currentPageIndex;
    cmd.strokes.append(s);
    pushUndoCommand(cmd);

    m_hasUnsavedChanges = true;
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();
}

bool Backend::eraseStrokes(const QPointF &p1, const QPointF &p2, float radius) {
    DocumentPage *page = currentPage();
    if (!page || page->strokes.isEmpty())
        return false;

    QVector<Stroke> erasedStrokes;
    const Pt q1{static_cast<float>(p1.x()), static_cast<float>(p1.y())};
    const Pt q2{static_cast<float>(p2.x()), static_cast<float>(p2.y())};

    for (int i = page->strokes.size() - 1; i >= 0; --i) {
        const Stroke &stroke = page->strokes[i];
        const float threshold = radius + (stroke.width * 0.5f);
        const float threshSq = threshold * threshold;

        bool hit = false;
        if (stroke.points.size() == 1) {
            const Pt sp{stroke.points[0].x, stroke.points[0].y};
            if (ptSegmentDistSq(sp, q1, q2) <= threshSq)
                hit = true;
        } else {
            for (int j = 0; j < stroke.points.size() - 1; ++j) {
                const Pt sp1{stroke.points[j].x, stroke.points[j].y};
                const Pt sp2{stroke.points[j + 1].x, stroke.points[j + 1].y};
                if (segmentSegmentDistSq(sp1, sp2, q1, q2) <= threshSq) {
                    hit = true;
                    break;
                }
            }
        }

        if (hit) {
            erasedStrokes.append(stroke);
            page->strokes.removeAt(i);
        }
    }

    if (!erasedStrokes.isEmpty()) {
        HistoryCommand cmd;
        cmd.type = HistoryType::EraseStrokes;
        cmd.pageIndex = m_currentPageIndex;
        cmd.strokes = erasedStrokes;
        pushUndoCommand(cmd);

        m_hasUnsavedChanges = true;
        emit hasUnsavedChangesChanged();
        emit canvasNeedsRepaint();
        return true;
    }

    return false;
}

void Backend::pushUndoCommand(const HistoryCommand &cmd) {
    m_undoStack.append(cmd);
    m_redoStack.clear();
    emit canUndoChanged();
    emit canRedoChanged();
}

void Backend::clearHistory() {
    m_undoStack.clear();
    m_redoStack.clear();
    emit canUndoChanged();
    emit canRedoChanged();
}

void Backend::undo() {
    if (m_undoStack.isEmpty())
        return;

    const HistoryCommand cmd = m_undoStack.takeLast();

    if (cmd.type == HistoryType::AddPage) {
        if (cmd.pageIndex < m_pages.size()) {
            m_pages.removeAt(cmd.pageIndex);
            if (m_currentPageIndex >= m_pages.size())
                m_currentPageIndex = std::max(0, static_cast<int>(m_pages.size() - 1));
            emit pageCountChanged();
            emit currentPageIndexChanged();
            emit pageDimensionsChanged();
        }
    } else if (cmd.type == HistoryType::DeletePage) {
        const int idx = std::min(cmd.pageIndex, static_cast<int>(m_pages.size()));
        m_pages.insert(idx, cmd.deletedPage);
        m_currentPageIndex = idx;
        emit pageCountChanged();
        emit currentPageIndexChanged();
        emit pageDimensionsChanged();
    } else {
        if (cmd.pageIndex < m_pages.size()) {
            DocumentPage &page = m_pages[cmd.pageIndex];
            switch (cmd.type) {
            case HistoryType::AddStroke:
                if (!cmd.strokes.isEmpty()) {
                    for (int i = page.strokes.size() - 1; i >= 0; --i) {
                        if (page.strokes[i].id == cmd.strokes.first().id) {
                            page.strokes.removeAt(i);
                            break;
                        }
                    }
                }
                break;
            case HistoryType::EraseStrokes:
                for (const auto &s : cmd.strokes)
                    page.strokes.append(s);
                break;
            case HistoryType::ClearPage:
                page.strokes = cmd.strokes;
                break;
            default:
                break;
            }
        }
    }

    m_redoStack.append(cmd);
    emit canUndoChanged();
    emit canRedoChanged();
    emit canvasNeedsRepaint();
    setStatus(tr("Undone"));
}

void Backend::redo() {
    if (m_redoStack.isEmpty())
        return;

    const HistoryCommand cmd = m_redoStack.takeLast();

    if (cmd.type == HistoryType::AddPage) {
        DocumentPage newPage;
        newPage.id = m_pages.size() + 1;
        m_pages.append(newPage);
        m_currentPageIndex = m_pages.size() - 1;
        emit pageCountChanged();
        emit currentPageIndexChanged();
        emit pageDimensionsChanged();
    } else if (cmd.type == HistoryType::DeletePage) {
        if (cmd.pageIndex < m_pages.size()) {
            m_pages.removeAt(cmd.pageIndex);
            if (m_currentPageIndex >= m_pages.size())
                m_currentPageIndex = std::max(0, static_cast<int>(m_pages.size() - 1));
            emit pageCountChanged();
            emit currentPageIndexChanged();
            emit pageDimensionsChanged();
        }
    } else {
        if (cmd.pageIndex < m_pages.size()) {
            DocumentPage &page = m_pages[cmd.pageIndex];
            switch (cmd.type) {
            case HistoryType::AddStroke:
                if (!cmd.strokes.isEmpty())
                    page.strokes.append(cmd.strokes.first());
                break;
            case HistoryType::EraseStrokes:
                for (const auto &erased : cmd.strokes) {
                    for (int i = page.strokes.size() - 1; i >= 0; --i) {
                        if (page.strokes[i].id == erased.id) {
                            page.strokes.removeAt(i);
                            break;
                        }
                    }
                }
                break;
            case HistoryType::ClearPage:
                page.strokes.clear();
                break;
            default:
                break;
            }
        }
    }

    m_undoStack.append(cmd);
    emit canUndoChanged();
    emit canRedoChanged();
    emit canvasNeedsRepaint();
    setStatus(tr("Redone"));
}

bool Backend::savePdfFile(const QString &filePath) {
    if (filePath.isEmpty() || m_pages.isEmpty())
        return false;

    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        setStatus(tr("Could not open file for saving: %1").arg(saveFile.errorString()));
        return false;
    }

    QPdfWriter writer(&saveFile);
    writer.setResolution(300);
    writer.setCreator(QStringLiteral("SplinkNote v3"));
    writer.setTitle(QFileInfo(filePath).baseName());

    QPainter painter;
    if (!painter.begin(&writer)) {
        setStatus(tr("Could not initialize PDF painter"));
        return false;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal scaleDpi = 300.0 / 72.0;

    for (int pageIdx = 0; pageIdx < m_pages.size(); ++pageIdx) {
        const DocumentPage &p = m_pages[pageIdx];
        if (pageIdx > 0)
            writer.newPage();

        writer.setPageSize(QPageSize(QSizeF(p.width, p.height), QPageSize::Point));

        painter.save();
        painter.scale(scaleDpi, scaleDpi);

        // Draw PDF page background if present (external PDF)
        if (!p.pdfBackground.isNull()) {
            painter.drawImage(QRectF(0, 0, p.width, p.height), p.pdfBackground);
        } else {
            // Draw clean white paper
            painter.fillRect(QRectF(0, 0, p.width, p.height), QColor("#ffffff"));
        }

        // Draw vector strokes
        for (const Stroke &s : p.strokes) {
            if (s.points.isEmpty())
                continue;

            QPainterPath path = buildSmoothPath(s.points);
            if (s.tool == 1) { // Highlighter
                painter.save();
                painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                QColor hlColor = s.color;
                hlColor.setAlphaF(0.40f);
                painter.setPen(QPen(hlColor, s.width, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
                painter.drawPath(path);
                painter.restore();
            } else { // Pen
                painter.setPen(QPen(s.color, s.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter.drawPath(path);
            }
        }

        painter.restore();
    }

    painter.end();

    // Serialize vector strokes into embedded JSON metadata
    QJsonObject rootObj;
    rootObj["v"] = 3;
    rootObj["generator"] = QStringLiteral("SplinkNote v3");
    rootObj["splinkOrigin"] = m_isSplinkNoteOrigin;
    rootObj["pageCount"] = m_pages.size();

    QJsonArray pagesArray;
    for (int i = 0; i < m_pages.size(); ++i) {
        const DocumentPage &p = m_pages[i];
        QJsonObject pageObj;
        pageObj["page"] = i;
        pageObj["w"] = p.width;
        pageObj["h"] = p.height;
        pageObj["hasExternalBg"] = !p.pdfBackground.isNull();

        QJsonArray strokesArray;
        for (const Stroke &s : p.strokes) {
            QJsonObject sObj;
            sObj["id"] = static_cast<double>(s.id);
            sObj["tool"] = s.tool;
            sObj["color"] = s.color.name(QColor::HexArgb);
            sObj["width"] = s.width;

            QJsonArray ptsArray;
            for (const auto &pt : s.points) {
                QJsonArray ptArr;
                ptArr.append(pt.x);
                ptArr.append(pt.y);
                ptArr.append(pt.pressure);
                ptsArray.append(ptArr);
            }
            sObj["pts"] = ptsArray;
            strokesArray.append(sObj);
        }
        pageObj["strokes"] = strokesArray;
        pagesArray.append(pageObj);
    }
    rootObj["pages"] = pagesArray;

    const QByteArray jsonBytes = QJsonDocument(rootObj).toJson(QJsonDocument::Compact);
    const QByteArray b64 = jsonBytes.toBase64();

    // Append metadata block to PDF stream
    saveFile.write("\n%SPLINKNOTE_DATA_V3_START%\n");
    saveFile.write(b64);
    saveFile.write("\n%SPLINKNOTE_DATA_V3_END%\n");

    if (!saveFile.commit()) {
        setStatus(tr("Could not write PDF file"));
        return false;
    }

    m_currentFilePath = filePath;
    m_hasUnsavedChanges = false;
    emit currentFilePathChanged();
    emit hasUnsavedChangesChanged();

    setStatus(tr("Saved %1").arg(QFileInfo(filePath).fileName()));
    return true;
}

bool Backend::openPdfFile(const QString &filePath) {
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        setStatus(tr("File does not exist"));
        return false;
    }

    // Read file bytes to check for embedded SplinkNote stroke data
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setStatus(tr("Could not read file"));
        return false;
    }
    const QByteArray fileData = file.readAll();
    file.close();

    bool hasEmbeddedStrokes = false;
    QJsonObject embeddedRoot;

    const QByteArray startMarker = "%SPLINKNOTE_DATA_V3_START%\n";
    const QByteArray endMarker = "\n%SPLINKNOTE_DATA_V3_END%";
    int startIdx = fileData.indexOf(startMarker);
    int endIdx = fileData.indexOf(endMarker, startIdx);

    if (startIdx != -1 && endIdx != -1 && endIdx > startIdx) {
        const int payloadStart = startIdx + startMarker.size();
        const QByteArray b64Payload = fileData.mid(payloadStart, endIdx - payloadStart).trimmed();
        const QByteArray jsonDecoded = QByteArray::fromBase64(b64Payload);
        const QJsonDocument doc = QJsonDocument::fromJson(jsonDecoded);
        if (doc.isObject()) {
            embeddedRoot = doc.object();
            hasEmbeddedStrokes = true;
        }
    }

    // Load PDF using Poppler
    std::unique_ptr<Poppler::Document> popplerDoc(Poppler::Document::load(filePath));
    if (!popplerDoc || popplerDoc->isLocked()) {
        setStatus(tr("Failed to load PDF document"));
        return false;
    }

    const int numPages = popplerDoc->numPages();
    if (numPages <= 0) {
        setStatus(tr("PDF has no pages"));
        return false;
    }

    bool splinkOrigin = false;
    if (hasEmbeddedStrokes) {
        splinkOrigin = embeddedRoot.value("splinkOrigin").toBool(false);
    } else {
        const QString creator = popplerDoc->info("Creator");
        if (creator.contains("SplinkNote"))
            splinkOrigin = true;
    }

    QVector<DocumentPage> loadedPages;
    const QJsonArray embeddedPages = embeddedRoot.value("pages").toArray();

    for (int i = 0; i < numPages; ++i) {
        std::unique_ptr<Poppler::Page> popplerPage(popplerDoc->page(i));
        if (!popplerPage)
            continue;

        DocumentPage page;
        page.id = i + 1;
        const QSizeF ptSize = popplerPage->pageSizeF();
        page.width = ptSize.width() > 0 ? ptSize.width() : 595.28f;
        page.height = ptSize.height() > 0 ? ptSize.height() : 842.0f;

        bool hasExternalBg = !splinkOrigin;
        if (hasEmbeddedStrokes && i < embeddedPages.size()) {
            const QJsonObject pObj = embeddedPages[i].toObject();
            if (pObj.contains("hasExternalBg"))
                hasExternalBg = pObj.value("hasExternalBg").toBool();
        }

        // Only render the Poppler background raster image if it's an external PDF
        // or a document that had an external background
        if (hasExternalBg) {
            page.pdfBackground = popplerPage->renderToImage(150.0, 150.0);
        }

        // Rehydrate embedded strokes if available
        if (hasEmbeddedStrokes && i < embeddedPages.size()) {
            const QJsonObject pObj = embeddedPages[i].toObject();
            const QJsonArray strokesArr = pObj.value("strokes").toArray();
            for (const auto &sVal : strokesArr) {
                const QJsonObject sObj = sVal.toObject();
                Stroke s;
                s.id = static_cast<quint64>(sObj.value("id").toDouble());
                if (s.id >= m_nextStrokeId)
                    m_nextStrokeId = s.id + 1;
                s.tool = sObj.value("tool").toInt();
                s.color = QColor(sObj.value("color").toString("#18181c"));
                s.width = sObj.value("width").toDouble(2.0);

                const QJsonArray ptsArr = sObj.value("pts").toArray();
                for (const auto &ptVal : ptsArr) {
                    const QJsonArray ptArr = ptVal.toArray();
                    if (ptArr.size() >= 2) {
                        StrokePoint pt;
                        pt.x = ptArr[0].toDouble();
                        pt.y = ptArr[1].toDouble();
                        pt.pressure = ptArr.size() >= 3 ? ptArr[2].toDouble(0.5) : 0.5f;
                        s.points.append(pt);
                    }
                }
                page.strokes.append(s);
            }
        }

        loadedPages.append(page);
    }

    if (loadedPages.isEmpty()) {
        setStatus(tr("Could not render any pages from PDF"));
        return false;
    }

    m_pages = loadedPages;
    m_currentPageIndex = 0;
    m_hasDocument = true;
    m_isSplinkNoteOrigin = splinkOrigin;
    m_currentFilePath = filePath;
    m_hasUnsavedChanges = false;
    clearHistory();

    emit hasDocumentChanged();
    emit pageCountChanged();
    emit currentPageIndexChanged();
    emit pageDimensionsChanged();
    emit isSplinkNoteOriginChanged();
    emit currentFilePathChanged();
    emit hasUnsavedChangesChanged();
    emit canvasNeedsRepaint();

    setStatus(tr("Opened %1 (%n pages)", "", numPages).arg(QFileInfo(filePath).fileName()));
    return true;
}
