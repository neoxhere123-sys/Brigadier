#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QToolButton>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>
#include <QStatusBar>
#include <QShortcut>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineDownloadRequest>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QStyle>
#include <QAction>

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

private slots:
    void setupTopBar() {
        QToolBar *toolBar = addToolBar("Main Toolbar");
        toolBar->setMovable(false);

        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        connect(newTabBtn, &QToolButton::clicked, this, [this]() { addNewTab(); });
        toolBar->addWidget(newTabBtn);

        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        toolBar->addWidget(spacer);

        // FIX: Using SP_DialogSaveButton for downloads
        QAction *dlAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Downloads", this);
        connect(dlAction, &QAction::triggered, this, &Browser::showDownloadsFolder);
        toolBar->addAction(dlAction);

        // Hamburger-ish icon
        QAction *cfgAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), "Config", this);
        connect(cfgAction, &QAction::triggered, this, &Browser::openConfigPage);
        toolBar->addAction(cfgAction);
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
                int percent = (download->receivedBytes() * 100) / download->totalBytes();
                progress->setValue(percent);
                statusBar()->showMessage(QString("Downloading: %1%").arg(percent));
            }
        });

        connect(download, &QWebEngineDownloadRequest::stateChanged, this, [this, progress](QWebEngineDownloadRequest::DownloadState state) {
            // FIX: Using DownloadCompleted instead of DownloadFinished
            if (state == QWebEngineDownloadRequest::DownloadCompleted) {
                statusBar()->removeWidget(progress);
                statusBar()->showMessage("Download Complete!", 5000);
                progress->deleteLater();
            }
        });
    }

    void openConfigPage() {
        QString html = "<html><body style='background:#121212;color:white;font-family:sans-serif;text-align:center;padding-top:50px;'>"
                       "<h1>Config</h1>"
                       "<p>Welp, there's literally NOTHING, to change, go grab the source from <br>"
                       "<a style='color:#00afff;' href='https://github.com/neoxhere123-sys/Brigadier'>"
                       "https://github.com/neoxhere123-sys/Brigadier</a> and edit it yourself.</p>"
                       "</body></html>";
        QWebEngineView *view = new QWebEngineView(this);
        view->setHtml(html);
        tabs->addTab(view, "Config");
        tabs->setCurrentWidget(view);
    }

    void showDownloadsFolder() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    }

    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        QWebEngineView *view = new QWebEngineView(this);
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
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Brigadier Beta v2.5");
    browser.resize(1280, 720);
    browser.show();
    return app.exec();
}

#include "main.moc"