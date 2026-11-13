#include <QColor>
#include <QDir>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QtTest>

#include "backend.h"
#include "canvasitem.h"
#include "filepicker.h"
#include "theme.h"
#include "translator.h"

class FakeFilePicker : public FilePicker {
    Q_OBJECT
public:
    explicit FakeFilePicker(QObject *parent = nullptr) : FilePicker(parent) {}

    void openPdf(const QString &folder = {}) override {
        Q_UNUSED(folder);
        m_openCalled = true;
    }

    void savePdf(const QUrl &suggestedUrl = {}) override {
        m_suggestedUrl = suggestedUrl;
        m_saveCalled = true;
    }

    bool m_openCalled = false;
    bool m_saveCalled = false;
    QUrl m_suggestedUrl;
};

class SplinknoteTests : public QObject {
    Q_OBJECT

private slots:
    void cleanSlateStartExperience() {
        FakeFilePicker picker;
        Backend backend(&picker);

        // On launch, no document should be open
        QCOMPARE(backend.hasDocument(), false);
        QCOMPARE(backend.pageCount(), 0);

        // New note triggers clean notebook
        backend.newDocument();
        QCOMPARE(backend.hasDocument(), true);
        QCOMPARE(backend.pageCount(), 1);
        QCOMPARE(backend.currentPageIndex(), 0);
        QCOMPARE(backend.isSplinkNoteOrigin(), true);
        QCOMPARE(backend.hasUnsavedChanges(), false);

        // Closing note returns to clean slate
        backend.closeDocument();
        QCOMPARE(backend.hasDocument(), false);
        QCOMPARE(backend.pageCount(), 0);
    }

    void toolSelectionAndSwitching() {
        Backend backend;
        backend.newDocument();

        // Default tool is Pen (0)
        QCOMPARE(backend.currentTool(), 0);

        // Switch to Highlighter (1)
        backend.setCurrentTool(1);
        QCOMPARE(backend.currentTool(), 1);

        // Switch to Eraser (2)
        backend.setCurrentTool(2);
        QCOMPARE(backend.currentTool(), 2);

        // Switch back to Pen (0)
        backend.setCurrentTool(0);
        QCOMPARE(backend.currentTool(), 0);
    }

    void strokeCommitAndUndoRedo() {
        Backend backend;
        backend.newDocument();

        QCOMPARE(backend.canUndo(), false);
        QCOMPARE(backend.canRedo(), false);

        Stroke s;
        s.tool = 0;
        s.color = QColor("#18181c");
        s.width = 2.0f;
        s.points.append({10.0f, 10.0f, 0.5f});
        s.points.append({20.0f, 20.0f, 0.7f});

        backend.commitStroke(s);
        QVERIFY(backend.currentPage() != nullptr);
        QCOMPARE(backend.currentPage()->strokes.size(), 1);
        QCOMPARE(backend.canUndo(), true);
        QCOMPARE(backend.canRedo(), false);
        QCOMPARE(backend.hasUnsavedChanges(), true);

        // Undo
        backend.undo();
        QCOMPARE(backend.currentPage()->strokes.size(), 0);
        QCOMPARE(backend.canUndo(), false);
        QCOMPARE(backend.canRedo(), true);

        // Redo
        backend.redo();
        QCOMPARE(backend.currentPage()->strokes.size(), 1);
        QCOMPARE(backend.canUndo(), true);
        QCOMPARE(backend.canRedo(), false);
    }

    void sweptEraserHitTesting() {
        Backend backend;
        backend.newDocument();

        // Stroke drawn diagonally from (100, 100) to (200, 200)
        Stroke s;
        s.tool = 0;
        s.color = QColor("#18181c");
        s.width = 2.0f;
        s.points.append({100.0f, 100.0f, 0.5f});
        s.points.append({200.0f, 200.0f, 0.5f});
        backend.commitStroke(s);
        QCOMPARE(backend.currentPage()->strokes.size(), 1);

        // Erase swipe that crosses the stroke: (150, 50) to (150, 250)
        bool hit = backend.eraseStrokes(QPointF(150.0, 50.0), QPointF(150.0, 250.0), 10.0f);
        QVERIFY(hit);
        QCOMPARE(backend.currentPage()->strokes.size(), 0);

        // Erasing again should miss
        bool hitAgain = backend.eraseStrokes(QPointF(150.0, 50.0), QPointF(150.0, 250.0), 10.0f);
        QVERIFY(!hitAgain);

        // Undo restores erased stroke
        backend.undo();
        QCOMPARE(backend.currentPage()->strokes.size(), 1);
    }

    void pageNavigationAndDynamicAddition() {
        Backend backend;
        backend.newDocument();

        QCOMPARE(backend.pageCount(), 1);
        QCOMPARE(backend.currentPageIndex(), 0);

        // Add page
        backend.addPage();
        QCOMPARE(backend.pageCount(), 2);
        QCOMPARE(backend.currentPageIndex(), 1);

        // Add another page
        backend.addPage();
        QCOMPARE(backend.pageCount(), 3);
        QCOMPARE(backend.currentPageIndex(), 2);

        // Previous page
        backend.previousPage();
        QCOMPARE(backend.currentPageIndex(), 1);
        backend.previousPage();
        QCOMPARE(backend.currentPageIndex(), 0);

        // Next page
        backend.nextPage();
        QCOMPARE(backend.currentPageIndex(), 1);

        // Delete current page
        backend.deleteCurrentPage();
        QCOMPARE(backend.pageCount(), 2);

        // Undo deletion
        backend.undo();
        QCOMPARE(backend.pageCount(), 3);
    }

    void pdfSaveWithEmbeddedStrokePersistence() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString pdfPath = tempDir.filePath("splink_test.pdf");

        // 1. Author note in backend 1
        {
            Backend backend1;
            backend1.newDocument();

            // Page 1 stroke
            Stroke s1;
            s1.tool = 0; // Pen
            s1.color = QColor("#18181c");
            s1.width = 2.0f;
            s1.points.append({50.0f, 50.0f, 0.5f});
            s1.points.append({60.0f, 70.0f, 0.8f});
            backend1.commitStroke(s1);

            // Add Page 2 and highlighter stroke
            backend1.addPage();
            Stroke s2;
            s2.tool = 1; // Highlighter
            s2.color = QColor("#FFD60A");
            s2.width = 16.0f;
            s2.points.append({100.0f, 150.0f, 0.5f});
            s2.points.append({300.0f, 150.0f, 0.5f});
            backend1.commitStroke(s2);

            QVERIFY(backend1.savePdfFile(pdfPath));
            QVERIFY(QFile::exists(pdfPath));
        }

        // 2. Open saved PDF in a fresh backend 2 and verify full stroke rehydration
        {
            Backend backend2;
            QVERIFY(backend2.openPdfFile(pdfPath));
            QCOMPARE(backend2.hasDocument(), true);
            QCOMPARE(backend2.pageCount(), 2);
            QCOMPARE(backend2.isSplinkNoteOrigin(), true);

            // Verify Page 1 strokes
            backend2.setCurrentPageIndex(0);
            const DocumentPage *p0 = backend2.currentPage();
            QVERIFY(p0 != nullptr);
            QCOMPARE(p0->strokes.size(), 1);
            QCOMPARE(p0->strokes[0].tool, 0);
            QCOMPARE(p0->strokes[0].points.size(), 2);

            // Verify Page 2 strokes
            backend2.setCurrentPageIndex(1);
            const DocumentPage *p1 = backend2.currentPage();
            QVERIFY(p1 != nullptr);
            QCOMPARE(p1->strokes.size(), 1);
            QCOMPARE(p1->strokes[0].tool, 1);
            QCOMPARE(p1->strokes[0].points.size(), 2);

            // 3. Test non-destructive editing: erase stroke on page 2 and undo
            bool erased = backend2.eraseStrokes(QPointF(200.0, 100.0), QPointF(200.0, 200.0), 10.0f);
            QVERIFY(erased);
            QCOMPARE(p1->strokes.size(), 0);

            backend2.undo();
            QCOMPARE(p1->strokes.size(), 1);
        }
    }

    void themeFallsBack() {
        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QColor("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QColor("black"));
    }

    void themeReadsAccent() {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("theme");
        QFile colors(dir.path() + "/theme/colors.toml");
        QVERIFY(colors.open(QIODevice::WriteOnly));
        colors.write("accent = \"#112233\"\n");
        colors.close();

        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QColor("#112233"));
        QCOMPARE(theme.accentForeground(), QColor("white"));
    }

    void themeFollowsHyprlandRounding() {
        QTemporaryDir dir;
        const QString response = dir.filePath("response.json"), command = dir.filePath("hyprctl");
        auto write = [](const QString &path, const QByteArray &data) {
            QFile file(path);
            return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
        };
        QVERIFY(write(command, "#!/bin/sh\ncat '" + response.toUtf8() + "'\n"));
        QVERIFY(QFile::setPermissions(command, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(write(response, "{\"int\":0}"));

        Theme theme(dir.path(), nullptr, command);
        QCOMPARE(theme.radius(), 0);
        QVERIFY(write(response, "{\"int\":8}"));
        QTRY_COMPARE(theme.radius(), 8);
    }

    void translatorFallsBackToEnglish() {
        QTemporaryDir dir;
        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("en"));
        QVERIFY(translator.translate("Main", "Save").isNull());
    }

    void translatorFollowsTheLanguageFile() {
        QTemporaryDir dir;
        write(dir.path() + "/language", "da\n");
        write(dir.path() + "/da.json", R"({"New Note": "Ny note", "Save": "Gem"})");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("da"));
        QCOMPARE(translator.translate("Main", "New Note"), QString("Ny note"));
        QCOMPARE(translator.translate("Main", "Save"), QString("Gem"));
        QVERIFY(translator.translate("Main", "Quit").isNull());
    }

    void translatorPicksPluralForms() {
        QTemporaryDir dir;
        write(dir.path() + "/language", "en\n");
        write(dir.path() + "/en.json", R"json({"Opened %1 (%n pages)": {"one": "Opened %1 (%n page)", "other": "Opened %1 (%n pages)"}})json");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.translate("Main", "Opened %1 (%n pages)", nullptr, 1), QString("Opened %1 (%n page)"));
        QCOMPARE(translator.translate("Main", "Opened %1 (%n pages)", nullptr, 5), QString("Opened %1 (%n pages)"));
    }

    void canvasAndWindowRendering() {
        qmlRegisterType<CanvasItem>("SplinkNote", 1, 0, "CanvasItem");
        qmlRegisterUncreatableType<Backend>("SplinkNote", 1, 0, "Backend", "Backend is provided by C++");
        Translator translator;
        Theme theme;
        Backend backend;
        backend.newDocument();

        // Add a vibrant highlighter stroke and an ink pen stroke
        Stroke hl;
        hl.tool = 1;
        hl.color = QColor("#FFD60A");
        hl.width = 24.0f;
        hl.points.append({100.0f, 100.0f, 0.5f});
        hl.points.append({250.0f, 100.0f, 0.5f});
        hl.points.append({400.0f, 100.0f, 0.5f});
        backend.commitStroke(0, hl);

        Stroke pen;
        pen.tool = 0;
        pen.color = QColor("#18181c");
        pen.width = 3.0f;
        pen.points.append({80.0f, 90.0f, 0.6f});
        pen.points.append({250.0f, 120.0f, 0.8f});
        pen.points.append({420.0f, 90.0f, 0.7f});
        backend.commitStroke(0, pen);

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("translator", &translator);
        engine.rootContext()->setContextProperty("theme", &theme);
        engine.rootContext()->setContextProperty("backend", &backend);

        engine.load(QUrl("qrc:/Main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto *win = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(win != nullptr);
        win->show();

        QImage img = win->grabWindow();
        QColor centerCol = img.pixelColor(img.width() / 2, img.height() / 2);
        QCOMPARE(centerCol, QColor("#ffffff"));
        img.save("/home/sam/Work/splinknote/test_grab.png");
    }

    void pdfRenderingVerification() {
        Backend backend;
        if (QFile::exists("/home/sam/Documents/test.pdf")) {
            QVERIFY(backend.openPdfFile("/home/sam/Documents/test.pdf"));
            QCOMPARE(backend.hasDocument(), true);
            QVERIFY(backend.pageCount() > 0);
            QVERIFY(!backend.currentPage()->pdfBackground.isNull());
            // High-DPI supersampling produces crisp dimensions (>1000px at 300 DPI)
            QVERIFY(backend.currentPage()->pdfBackground.width() > 1000);
        }
    }

    void multiPageContinuousLayout() {
        Backend backend;
        backend.newDocument();
        QCOMPARE(backend.pageCount(), 1);
        QCOMPARE(backend.pageTopWorld(0), 0.0);

        backend.addPage();
        QCOMPARE(backend.pageCount(), 2);
        const qreal expectedPage1Top = backend.pageAt(0)->height + 32.0;
        QCOMPARE(backend.pageTopWorld(1), expectedPage1Top);
        QCOMPARE(backend.totalDocumentHeightWorld(), backend.pageAt(0)->height + backend.pageAt(1)->height + 32.0);

        // Auto-expand notebook pages
        backend.autoExpandPages(4);
        QCOMPARE(backend.pageCount(), 4);
    }

    void stylusAndToolProperties() {
        Backend backend;
        backend.newDocument();

        // Pen width and color
        backend.setPenWidth(3.5f);
        QCOMPARE(backend.penWidth(), 3.5f);
        backend.setPenColor(QColor("#f7768e"));
        QCOMPARE(backend.penColor(), QColor("#f7768e"));

        // Highlighter color and width
        backend.setHighlighterColor(QColor("#70c0e8"));
        QCOMPARE(backend.highlighterColor(), QColor("#70c0e8"));
        backend.setHighlighterWidth(26.0f);
        QCOMPARE(backend.highlighterWidth(), 26.0f);

        // Eraser radius (defaults to 3.0pt fine precision point)
        QCOMPARE(backend.eraserRadius(), 3.0f);
        backend.setEraserRadius(10.0f);
        QCOMPARE(backend.eraserRadius(), 10.0f);
    }

    void multipleHighlighterStrokes() {
        Backend backend;
        backend.newDocument();

        // Commit multiple highlighter strokes on page 0
        Stroke h1;
        h1.tool = 1;
        h1.color = QColor("#FFD60A");
        h1.width = 18.0f;
        h1.points.append({50.0f, 50.0f, 0.5f});
        h1.points.append({150.0f, 50.0f, 0.5f});
        backend.commitStroke(0, h1);

        Stroke h2;
        h2.tool = 1;
        h2.color = QColor("#70C0E8");
        h2.width = 18.0f;
        h2.points.append({50.0f, 100.0f, 0.5f});
        h2.points.append({150.0f, 100.0f, 0.5f});
        backend.commitStroke(0, h2);

        QCOMPARE(backend.pageAt(0)->strokes.size(), 2);
        QCOMPARE(backend.pageAt(0)->strokes[0].tool, 1);
        QCOMPARE(backend.pageAt(0)->strokes[1].tool, 1);

        // Undo and redo
        backend.undo();
        QCOMPARE(backend.pageAt(0)->strokes.size(), 1);
        backend.redo();
        QCOMPARE(backend.pageAt(0)->strokes.size(), 2);
    }

    void continuousCanvasCoordinateMapping() {
        Backend backend;
        backend.newDocument();
        backend.addPage();
        QCOMPARE(backend.pageCount(), 2);

        CanvasItem canvas;
        canvas.setBackend(&backend);
        canvas.setSize(QSizeF(1000, 2000));
        canvas.setZoom(1.0);
        canvas.setPanX(0.0);
        canvas.setPanY(0.0);

        QRectF r0 = canvas.pageScreenRect(0);
        QRectF r1 = canvas.pageScreenRect(1);
        QVERIFY(r0.isValid());
        QVERIFY(r1.isValid());
        QVERIFY(r1.top() > r0.bottom());

        // Screen pos on page 0
        QPointF p0;
        int idx0 = canvas.pageIndexAtScreenPos(QPointF(r0.center().x(), r0.center().y()), &p0);
        QCOMPARE(idx0, 0);

        // Screen pos on page 1
        QPointF p1;
        int idx1 = canvas.pageIndexAtScreenPos(QPointF(r1.center().x(), r1.center().y()), &p1);
        QCOMPARE(idx1, 1);
    }

    void directSaveVersusSaveAs() {
        FakeFilePicker picker;
        Backend backend(&picker);
        backend.newDocument();

        // 1. New untitled document: saveDocument invokes file picker (Save As)
        picker.m_saveCalled = false;
        backend.saveDocument();
        QCOMPARE(picker.m_saveCalled, true);

        // 2. Once a file path exists, saveDocument directly saves without picker dialog
        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        const QString path = tmp.fileName();
        tmp.close();

        QVERIFY(backend.savePdfFile(path));
        QCOMPARE(backend.currentFilePath(), path);

        // Modify document
        backend.addPage();
        QCOMPARE(backend.hasUnsavedChanges(), true);

        // Save again: should directly write without invoking file picker
        picker.m_saveCalled = false;
        backend.saveDocument();
        QCOMPARE(picker.m_saveCalled, false);
        QCOMPARE(backend.hasUnsavedChanges(), false);
    }

    void smartHorizontalPanLocking() {
        Backend backend;
        backend.newDocument();

        CanvasItem canvas;
        canvas.setBackend(&backend);
        canvas.setSize(QSizeF(1000, 1000));
        canvas.setZoom(1.0); // Page width is ~595.28 <= 1000 -> locked!

        // Attempting to pan horizontally while page fits in width must stay locked to 0
        canvas.setPanX(60.0);
        QCOMPARE(canvas.panX(), 0.0);

        // When zoomed in so page width exceeds window width (e.g. 595.28 * 2.0 = 1190.56 > 1000)
        canvas.setZoom(2.0);
        canvas.setPanX(40.0);
        QCOMPARE(canvas.panX(), 40.0);

        // When zooming back out so page fits in width, panX must auto-recenter to 0
        canvas.setZoom(1.0);
        QCOMPARE(canvas.panX(), 0.0);
    }

    void strictPageBoundaryHitTesting() {
        Backend backend;
        backend.newDocument();

        CanvasItem canvas;
        canvas.setBackend(&backend);
        canvas.setSize(QSizeF(1000, 1000));
        canvas.setZoom(1.0);
        canvas.setPanX(0.0);
        canvas.setPanY(0.0);

        QRectF r0 = canvas.pageScreenRect(0);
        QVERIFY(r0.isValid());

        // Hit testing inside page
        QCOMPARE(canvas.pageIndexHit(r0.center()), 0);

        // Hit testing outside page in the margin
        QPointF marginPoint(r0.left() - 50.0, r0.center().y());
        QCOMPARE(canvas.pageIndexHit(marginPoint), -1);
    }

    void insertPageAfterCurrentAndUndo() {
        Backend backend;
        backend.newDocument();
        QCOMPARE(backend.pageCount(), 1);
        QCOMPARE(backend.currentPageIndex(), 0);

        // Add page 2 (index 1) and page 3 (index 2)
        backend.addPage();
        QCOMPARE(backend.pageCount(), 2);
        QCOMPARE(backend.currentPageIndex(), 1);

        backend.addPage();
        QCOMPARE(backend.pageCount(), 3);
        QCOMPARE(backend.currentPageIndex(), 2);

        // Navigate back to page 2 (index 1)
        backend.setCurrentPageIndex(1);
        QCOMPARE(backend.currentPageIndex(), 1);

        // Add page: MUST insert directly after current page (at index 2), NOT at the end
        backend.addPage();
        QCOMPARE(backend.pageCount(), 4);
        QCOMPARE(backend.currentPageIndex(), 2);

        // Undo page insertion
        QCOMPARE(backend.canUndo(), true);
        backend.undo();
        QCOMPARE(backend.pageCount(), 3);
        QCOMPARE(backend.currentPageIndex(), 1);

        // Redo page insertion
        QCOMPARE(backend.canRedo(), true);
        backend.redo();
        QCOMPARE(backend.pageCount(), 4);
        QCOMPARE(backend.currentPageIndex(), 2);
    }

    void effectivePageCountPruning() {
        Backend backend;
        backend.newDocument();

        // 1. Fresh document has 1 blank page: effective count is 1
        QCOMPARE(backend.pageCount(), 1);
        QCOMPARE(backend.effectivePageCount(), 1);

        // 2. Auto-expand to 4 pages (e.g. infinite scroll expansion)
        backend.autoExpandPages(4);
        QCOMPARE(backend.pageCount(), 4);
        // All pages are blank, but page 1 is always included -> effective count is 1
        QCOMPARE(backend.effectivePageCount(), 1);

        // 3. Draw a stroke on page index 2 (the 3rd page)
        Stroke s;
        s.tool = 0;
        s.color = QColor("#18181c");
        s.width = 2.0f;
        s.points.append({50.0f, 50.0f, 0.5f});
        s.points.append({60.0f, 60.0f, 0.5f});
        backend.commitStroke(2, s);

        // Pages 0, 1 (blank), and 2 (with data) -> effective page count is 3!
        QCOMPARE(backend.pageCount(), 4);
        QCOMPARE(backend.effectivePageCount(), 3);

        // 4. Clear page 2
        backend.setCurrentPageIndex(2);
        backend.clearCurrentPage();
        // Now only blank pages remain -> effective count drops back to 1
        QCOMPARE(backend.effectivePageCount(), 1);
    }

    void deletePageWithUndo() {
        Backend backend;
        backend.newDocument();
        backend.addPage(); // Page 1
        backend.addPage(); // Page 2
        QCOMPARE(backend.pageCount(), 3);

        // Set to middle page (index 1) and draw a stroke
        backend.setCurrentPageIndex(1);
        Stroke s;
        s.tool = 0;
        s.color = QColor("#18181c");
        s.width = 2.0f;
        s.points.append({30.0f, 30.0f, 0.5f});
        backend.commitStroke(1, s);
        QCOMPARE(backend.pageAt(1)->strokes.size(), 1);

        // Delete page 1
        backend.deleteCurrentPage();
        QCOMPARE(backend.pageCount(), 2);
        QCOMPARE(backend.canUndo(), true);

        // Undo deletion: restores deleted page with its stroke
        backend.undo();
        QCOMPARE(backend.pageCount(), 3);
        QCOMPARE(backend.currentPageIndex(), 1);
        QCOMPARE(backend.pageAt(1)->strokes.size(), 1);
    }

    void defaultPenColorAndPalette() {
        Backend backend;
        backend.newDocument();

        // Default pen color must be ink black #18181c
        QCOMPARE(backend.penColor(), QColor("#18181c"));
    }

private:
    static void write(const QString &path, const QByteArray &contents) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
    }
};

QTEST_MAIN(SplinknoteTests)
#include "splinknote_tests.moc"
