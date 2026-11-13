// SplinkNote — Minimalist stylus note-taking and PDF annotation for Omarchy.

#include <QCommandLineParser>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "backend.h"
#include "canvasitem.h"
#include "portalfilepicker.h"
#include "theme.h"
#include "translator.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("splinknote");
    app.setApplicationVersion("3.0.0");
    app.setDesktopFileName("splinknote");
    app.setWindowIcon(QIcon::fromTheme("splinknote"));

    QQuickStyle::setStyle("Material");

    qmlRegisterType<CanvasItem>("SplinkNote", 1, 0, "CanvasItem");

    Translator translator;
    app.installTranslator(&translator);

    Theme theme;
    PortalFilePicker filePicker;
    Backend backend(&filePicker);

    // Command-line handling: support opening a file passed as an argument
    QCommandLineParser parser;
    parser.setApplicationDescription("SplinkNote: Minimalist stylus note-taking and PDF annotation");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "PDF document to open");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        const QString path = QFileInfo(args.first()).absoluteFilePath();
        backend.openPdfFile(path);
    }

    QQmlApplicationEngine engine;
    QObject::connect(&translator, &Translator::changed, &engine, &QQmlEngine::retranslate);
    engine.rootContext()->setContextProperty("translator", &translator);
    engine.rootContext()->setContextProperty("theme", &theme);
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
