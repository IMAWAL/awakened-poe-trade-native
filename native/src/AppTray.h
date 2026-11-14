#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class QSystemTrayIcon;

namespace AptNative {

class AppTray final : public QObject {
    Q_OBJECT
public:
    explicit AppTray(QObject *parent = nullptr);
    void setAppUrl(const QUrl &url);
    void setOverlayKey(const QString &overlayKey);
    void setDataDirectory(const QString &dataDirectory);
    void setOverlayScreenName(const QString &name);

signals:
    void toggleOverlay();
    void quitRequested();
    void overlayScreenSelected(const QString &name);

private:
    QSystemTrayIcon *m_tray;
    QUrl m_appUrl;
    QString m_overlayKey = QStringLiteral("Shift + Space");
    QString m_dataDirectory;
    QString m_overlayScreenName;
};

} // namespace AptNative
