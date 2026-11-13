// Theme watching follows Omacut and Folio (MIT, David Heinemeier Hansson).
#include "theme.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <cmath>

namespace {

const QHash<QString, QColor> &fallbackColors() {
    static const QHash<QString, QColor> colors{
        {"accent", QColor("#FFD60A")},
        {"background", QColor("#1a1b26")},
        {"dark_background", QColor("#16161e")},
        {"lighter_background", QColor("#24283b")},
        {"foreground", QColor("#a9b1d6")},
        {"bright_foreground", QColor("#c0caf5")},
    };
    return colors;
}

double luminance(const QColor &c) {
    auto linear = [](double v) { return v <= .04045 ? v / 12.92 : std::pow((v + .055) / 1.055, 2.4); };
    return .2126 * linear(c.redF()) + .7152 * linear(c.greenF()) + .0722 * linear(c.blueF());
}

QColor mix(const QColor &a, const QColor &b, double t) {
    return QColor::fromRgbF(a.redF() * (1 - t) + b.redF() * t,
                            a.greenF() * (1 - t) + b.greenF() * t,
                            a.blueF() * (1 - t) + b.blueF() * t);
}

struct Palette {
    QHash<QString, QColor> colors;
    bool light = false;
};

Palette readPalette(const QString &path) {
    Palette palette;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return palette;
    const QString text = QString::fromUtf8(file.readAll());
    static const QRegularExpression colorLine(R"re(^\s*([a-z_]+)\s*=\s*["'](#[0-9a-fA-F]{6})["'])re",
                                              QRegularExpression::MultilineOption);
    for (auto it = colorLine.globalMatch(text); it.hasNext();) {
        const auto match = it.next();
        palette.colors.insert(match.captured(1), QColor(match.captured(2)));
    }
    static const QRegularExpression modeLine(R"re(^\s*mode\s*=\s*["']light["'])re",
                                             QRegularExpression::MultilineOption);
    palette.light = modeLine.match(text).hasMatch();
    return palette;
}

} // namespace

Theme::Theme(const QString &directory, QObject *parent, const QString &hyprctl)
    : QObject(parent),
      m_directory(directory.isEmpty() ? QDir::homePath() + "/.local/state/omarchy/current" : directory) {
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(80);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] { m_debounce.start(); });
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] { m_debounce.start(); });
    connect(&m_debounce, &QTimer::timeout, this, &Theme::reload);

    m_hyprctl = !hyprctl.isEmpty() ? hyprctl
              : directory.isEmpty() && !qEnvironmentVariableIsEmpty("HYPRLAND_INSTANCE_SIGNATURE")
                  ? QStandardPaths::findExecutable("hyprctl") : QString();
    m_roundingTimeout.setSingleShot(true);
    m_roundingTimeout.setInterval(500);
    connect(&m_roundingTimeout, &QTimer::timeout, &m_roundingProcess, &QProcess::kill);
    connect(&m_roundingProcess, &QProcess::errorOccurred, this, [this] { m_roundingTimeout.stop(); });
    connect(&m_roundingProcess, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
        m_roundingTimeout.stop();
        const auto value = QJsonDocument::fromJson(m_roundingProcess.readAllStandardOutput()).object().value("int");
        if (code != 0 || status != QProcess::NormalExit || !value.isDouble() || value.toInt(-1) < 0)
            return;
        if (value.toInt() != m_radius) {
            m_radius = value.toInt();
            emit changed();
        }
    });
    m_roundingPoll.setInterval(1000);
    connect(&m_roundingPoll, &QTimer::timeout, this, &Theme::refreshRounding);
    if (!m_hyprctl.isEmpty())
        m_roundingPoll.start();
    reload();
}

Theme::~Theme() {
    if (m_roundingProcess.state() != QProcess::NotRunning) {
        m_roundingProcess.kill();
        m_roundingProcess.waitForFinished(1000);
    }
}

void Theme::refreshRounding() {
    if (m_hyprctl.isEmpty() || m_roundingProcess.state() != QProcess::NotRunning)
        return;
    m_roundingProcess.start(m_hyprctl, {"-j", "getoption", "decoration:rounding"});
    m_roundingTimeout.start();
}

QColor Theme::color(const QString &key) const {
    if (const auto it = m_colors.constFind(key); it != m_colors.constEnd())
        return *it;
    if (key.endsWith("background") && m_colors.contains("background"))
        return m_colors.value("background");
    if (key.endsWith("foreground") && m_colors.contains("foreground"))
        return m_colors.value("foreground");
    return fallbackColors().value(key, QColor("#888888"));
}

QColor Theme::lighterBackground() const {
    const QColor lighter = color("lighter_background");
    const QColor bg = background();
    const double gap = qAbs(luminance(lighter) - luminance(bg));
    if (gap > 0.004)
        return lighter;
    return mix(bg, foreground(), 0.08);
}

QColor Theme::accentForeground() const {
    return luminance(accent()) > .179 ? QColor("black") : QColor("white");
}

QColor Theme::secondaryForeground() const {
    return mix(background(), foreground(), 0.68);
}

QString Theme::readAccent(const QString &path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        const QRegularExpression expression(R"re(^\s*accent\s*=\s*["'](#[0-9a-fA-F]{6})["'])re",
                                            QRegularExpression::MultilineOption);
        const auto match = expression.match(QString::fromUtf8(file.readAll()));
        if (match.hasMatch())
            return match.captured(1);
    }
    return "#FFD60A";
}

void Theme::reload() {
    refreshRounding();
    const auto paths = m_watcher.files() + m_watcher.directories();
    if (!paths.isEmpty())
        m_watcher.removePaths(paths);

    QString ancestor = m_directory;
    while (!QFileInfo::exists(ancestor) && ancestor != "/")
        ancestor = QFileInfo(ancestor).absolutePath();
    QStringList candidates{ancestor, QFileInfo(ancestor).absolutePath(), m_directory,
                           m_directory + "/theme", m_directory + "/theme/colors.toml"};
    candidates.removeDuplicates();
    for (const auto &path : candidates)
        if (QFileInfo::exists(path))
            m_watcher.addPath(path);

    const Palette palette = readPalette(m_directory + "/theme/colors.toml");
    if (palette.colors != m_colors || palette.light != m_light) {
        m_colors = palette.colors;
        m_light = palette.light;
        emit changed();
    }
}
