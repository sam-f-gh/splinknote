#include <QColor>
#include <QDir>
#include <QFile>
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
        QCOMPARE(theme.accent(), QString("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QString("black"));
    }

    void themeReadsAccent() {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("theme");
        QFile colors(dir.path() + "/theme/colors.toml");
        QVERIFY(colors.open(QIODevice::WriteOnly));
        colors.write("accent = \"#112233\"\n");
        colors.close();

        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#112233"));
        QCOMPARE(theme.accentForeground(), QString("white"));
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

private:
    static void write(const QString &path, const QByteArray &contents) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
    }
};

QTEST_MAIN(SplinknoteTests)
#include "splinknote_tests.moc"
