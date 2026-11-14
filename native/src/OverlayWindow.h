#pragma once

#include <QElapsedTimer>
#include <QNetworkCookie>
#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QTimer>
#include <QUrl>
#include <QWidget>

class QWebEngineView;
class QPushButton;
class QScreen;
namespace LayerShellQt { class Window; }

namespace AptNative {

class Logger;

class OverlayWindow final : public QWidget {
    Q_OBJECT
public:
    explicit OverlayWindow(QString profilePath,
                           bool useLayerShell,
                           Logger *logger,
                           QWidget *parent = nullptr);

    void load(const QUrl &url);
    QByteArray browserUserAgent() const;
    void setInteractive(bool interactive);
    bool isInteractive() const;
    void toggleInteractive();
    void beginTrackArea(const QPoint &from,
                        const QRect &area,
                        int closeThreshold,
                        const QString &holdKey = {});
    void updateCursorPosition(const QPoint &position);
    void focusGame();
    void selectScreen(const QString &name);
    QString selectedScreenName() const;

signals:
    void pageLoaded();
    void interactionChanged(bool interactive);
    void hideExclusiveWidgetRequested();
    void focusGameRequested();
    void visibilityRequested(bool visible);
    void proxyCookieAvailable(const QNetworkCookie &cookie);
    void screenSelected(const QString &name);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void checkTrackedArea();
    void checkUiVisibility();
    void syncEmbeddedBrowser();

private:
    void configureLayerShell();
    void openExternalUrl(const QUrl &url);
    void setUiVisible(bool visible);
    QPoint desktopCursorPosition() const;
    void positionMoveHandle();

    QWebEngineView *m_view;
    QWebEngineView *m_browserView;
    LayerShellQt::Window *m_layerWindow = nullptr;
    Logger *m_logger;
    bool m_useLayerShell;
    QPushButton *m_moveHandle;
    QString m_settingsPath;
    QString m_selectedScreenName;
    QPointer<QScreen> m_outputScreen;
    bool m_draggingScreen = false;
    bool m_interactive = true;
    QPoint m_trackFrom;
    QRect m_trackArea;
    int m_closeThreshold = 0;
    QString m_trackHoldKey;
    QPoint m_cursorPosition;
    bool m_hasCursorPosition = false;
    QTimer m_trackTimer;
    QElapsedTimer m_trackGraceTimer;
    bool m_trackPositionSettled = false;
    QTimer m_visibilityTimer;
    QTimer m_hideUiTimer;
    QTimer m_browserSyncTimer;
    bool m_uiVisible = true;
    QUrl m_browserUrl;
};

} // namespace AptNative
