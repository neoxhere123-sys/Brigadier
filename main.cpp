#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QToolBar>
#include <QProgressBar>
#include <QStatusBar>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineDownloadRequest>
#include <QWebEngineFullScreenRequest>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QStyle>
#include <QAction>
#include <QEvent>
#include <QTimer> // Added for the refresh timer

class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    MyWebEngineView(QWidget *parent = nullptr) : QWebEngineView(parent) {
        settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
        settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
        
        connect(page(), &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
            if (request.toggleOn()) {
                if (window()) window()->showFullScreen();
                request.accept();
            } else {
                if (window()) window()->showNormal();
                request.accept();
            }
            // Force the video engine to redraw after the window snaps to size
            QTimer::singleShot(200, this, [this]() {
                this->update();
                this->repaint();
            });
        });
    }

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override {
        Q_UNUSED(type);
        return this; 
    }
};

class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser(QWidget *parent = nullptr) : QMainWindow(parent) {
        tabs = new QTabWidget(this);
        tabs->setTabsClosable(true);
        tabs->setMovable(true);
        tabs->setDocumentMode(true);
        setCentralWidget(tabs);

        setupTopBar();

        QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
        profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

        connect(profile, &QWebEngineProfile::downloadRequested, this, &Browser::handleDownload);
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        addNewTab(QUrl("https://google.com"));
    }

protected:
    void changeEvent(QEvent *event) override {
        if (event->type() == QEvent::WindowStateChange) {
            bool full = isFullScreen();
            mainToolBar->setVisible(!full);
            statusBar()->setVisible(!full);
            if (tabs->tabBar()) tabs->tabBar()->setVisible(!full);
        }
        QMainWindow::changeEvent(event);
    }

private slots:
    void setupTopBar() {
        mainToolBar = addToolBar("Main Toolbar");
        mainToolBar->setMovable(false);

        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        connect(newTabBtn, &QToolButton::clicked, this, [this]() { addNewTab(); });
        mainToolBar->addWidget(newTabBtn);

        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        mainToolBar->addWidget(spacer);

        QAction *dlAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Downloads", this);
        connect(dlAction, &QAction::triggered, this, &Browser::showDownloadsFolder);
        mainToolBar->addAction(dlAction);

        QAction *cfgAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), "Config", this);
        connect(cfgAction, &QAction::triggered, this, &Browser::openConfigPage);
        mainToolBar->addAction(cfgAction);
    }

    void handleDownload(QWebEngineDownloadRequest *download) {
        download->setDownloadDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        download->setDownloadFileName(download->suggestedFileName());
        download->accept();

        QProgressBar *progress = new QProgressBar(this);
        progress->setMaximumWidth(200);
        statusBar()->addWidget(progress);

        connect(download, &QWebEngineDownloadRequest::receivedBytesChanged, this, [this, download, progress]() {
            if (download->totalBytes() > 0) {
                int percent = (int)((download->receivedBytes() * 100) / download->totalBytes());
                progress->setValue(percent);
            }
        });

        connect(download, &QWebEngineDownloadRequest::stateChanged, this, [this, progress](QWebEngineDownloadRequest::DownloadState state) {
            if (state == QWebEngineDownloadRequest::DownloadCompleted) {
                statusBar()->removeWidget(progress);
                progress->deleteLater();
            }
        });
    }

    void openConfigPage() {
        QString html = "<html><body style='background:#121212;color:white;font-family:sans-serif;text-align:center;padding-top:100px;'>"
                       "<h1>Config</h1>"
                       "<p>Go grab the source from <br><a style='color:#00afff;' href='https://github.com/neoxhere123-sys/Brigadier'>"
                       "https://github.com/neoxhere123-sys/Brigadier</a></p></body></html>";
        MyWebEngineView *view = new MyWebEngineView(this);
        view->setHtml(html);
        tabs->addTab(view, "Config");
        tabs->setCurrentWidget(view);
    }

    void showDownloadsFolder() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    }

    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        MyWebEngineView *view = new MyWebEngineView(this);
        view->load(url);
        int index = tabs->addTab(view, "New Tab");
        tabs->setCurrentIndex(index);
        connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
            int idx = tabs->indexOf(view);
            if (idx != -1) tabs->setTabText(idx, title.left(15));
        });
    }

    void closeTab(int index) {
        if (tabs->count() > 1) {
            QWidget *w = tabs->widget(index);
            tabs->removeTab(index);
            w->deleteLater();
        } else { close(); }
    }

private:
    QTabWidget *tabs;
    QToolBar *mainToolBar;
};

int main(int argc, char *argv[]) {
    // Arch Linux Fix: Disable hardware overlay for video if it flickers
    qputenv("QT_GSTREAMER_WIDGET_VIDEOSINK", "glimagesink");
    
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Brigadier Beta v2.6");
    browser.resize(1280, 720);
    browser.show();
    return app.exec();
}

#include "main.moc"