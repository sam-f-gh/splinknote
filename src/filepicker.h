#pragma once

#include <QObject>
#include <QUrl>

class FilePicker : public QObject {
    Q_OBJECT

public:
    explicit FilePicker(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~FilePicker() = default;

    virtual void openPdf(const QString &folder = {}) = 0;
    virtual void savePdf(const QUrl &suggestedUrl = {}) = 0;

signals:
    void pdfSelected(const QUrl &url);
    void saveSelected(const QUrl &url);
    void failed(const QString &message);
};
