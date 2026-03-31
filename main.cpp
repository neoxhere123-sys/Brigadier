#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QToolButton>
#include <QShortcut>
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

// 1. Custom View to handle both Fullscreen and Tab behavior
class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    MyWebEngineView(QWidget *parent = nullptr) : QWebEngineView(parent) {
        // Enable Fullscreen Support in Settings
        settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        
        // Handle Fullscreen Signal
        connect(page(), &QWebEnginePage::fullScreenRequested, this, [](QWebEngineFullScreenRequest request) {
            request.accept(); // This tells the browser "Yes, you can go fullscreen"
        });
    }

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override {
        Q_UNUSED(type);
        return this; 
    }
};

// 2. Main Browser Logic
class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser(QWidget *parent = nullptr) : QMainWindow(parent) {
        tabs = new QTabWidget(this);
        tabs->setTabsClosable(true);
        tabs->setMovable(true);
        tabs->setDocumentMode(true);
        setCentralWidget(tabs);

        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        tabs->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

        // --- Persistence ---
        QString storagePath = QDir::homePath() + "/.local/share/BrowserProject/storage";
        QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
        profile->setPersistentStoragePath(storagePath);
        profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        // --- NEW: Download Manager ---
        connect(profile, &QWebEngineProfile::downloadRequested, this, [](QWebEngineDownloadRequest *download) {
            // Suggest the default "Downloads" folder on your Arch system
            QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) 
                           + "/" + download->suggestedFileName();
            
            download->setDownloadDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            download->setDownloadFileName(download->suggestedFileName());
            download->accept(); // Starts the download
        });

        new QShortcut(QKeySequence("Ctrl+J"), this, SLOT(showDownloads()));
        connect(newTabBtn, &QToolButton::clicked, this, [this]() { addNewTab(); });
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        addNewTab(QUrl("https://youtube.com"));
    }

public slots:
    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        MyWebEngineView *view = new MyWebEngineView(this);
        view->load(url);
        
        int index = tabs->addTab(view, "Loading...");
        tabs->setCurrentIndex(index);

        connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
            int idx = tabs->indexOf(view);
            if (idx != -1) tabs->setTabText(idx, title.left(20));
        });
    }

    void closeTab(int index) {
        if (tabs->count() > 1) {
            QWidget *w = tabs->widget(index);
            tabs->removeTab(index);
            delete w;
        } else {
            this->close();
        }
    }

    void showDownloads() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    }

private:
    QTabWidget *tabs;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Browser Pro v1.0");
    browser.resize(1280, 720);
    browser.show();
    return app.exec();
}

#include "main.moc"