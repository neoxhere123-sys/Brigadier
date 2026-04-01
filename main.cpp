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

class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    // Ensure we use the profile passed to it
    MyWebEngineView(QWebEngineProfile *profile, QWidget *parent = nullptr) : QWebEngineView(profile, parent) {
        settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        
        connect(page(), &QWebEnginePage::fullScreenRequested, this, [](QWebEngineFullScreenRequest request) {
            request.accept();
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

        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        tabs->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

        // --- Setup the Profile for the whole Browser ---
        QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
        
        // Use an absolute path for Brigadier storage
        QString storagePath = QDir::homePath() + "/.local/share/Brigadier/storage";
        QDir().mkpath(storagePath); 
        
        profile->setPersistentStoragePath(storagePath);
        profile->setCachePath(storagePath + "/cache");
        
        // CRITICAL FIX FOR LOGINS:
        // AllowPersistentCookies = standard. ForcePersistentCookies = aggressive.
        profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

        // Download handling
        connect(profile, &QWebEngineProfile::downloadRequested, this, [](QWebEngineDownloadRequest *download) {
            download->setDownloadDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            download->setDownloadFileName(download->suggestedFileName());
            download->accept();
        });

        connect(newTabBtn, &QToolButton::clicked, this, [this]() { addNewTab(); });
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        addNewTab(QUrl("https://google.com"));
    }

public slots:
    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        // Use the defaultProfile which we configured above
        MyWebEngineView *view = new MyWebEngineView(QWebEngineProfile::defaultProfile(), this);
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
            delete w;
        } else {
            this->close();
        }
    }

private:
    QTabWidget *tabs;
};

int main(int argc, char *argv[]) {
    // High DPI support for Arch Linux
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    
    Browser browser;
    browser.setWindowTitle("Brigadier Beta v2");
    browser.resize(1280, 720);
    browser.show();
    
    return app.exec();
}

#include "main.moc"