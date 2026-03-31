#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
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

// 1. Custom Web View to handle target="_blank"
class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    MyWebEngineView(QWidget *parent = nullptr) : QWebEngineView(parent) {}

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override {
        Q_UNUSED(type);
        // Returns the same view to force links to open in the current tab
        return this; 
    }
};

// 2. Main Browser Logic
class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser(QWidget *parent = nullptr) : QMainWindow(parent) {
        // --- Tabs Manager ---
        tabs = new QTabWidget(this);
        tabs->setTabsClosable(true);
        tabs->setMovable(true);
        tabs->setDocumentMode(true); // Sleeker look on Arch/KDE/Gnome
        setCentralWidget(tabs);

        // --- "+" Tab Button ---
        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        newTabBtn->setCursor(Qt::PointingHandCursor);
        newTabBtn->setStyleSheet("QToolButton { padding: 4px; font-weight: bold; } QToolButton:hover { background: #555; }");
        tabs->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

        // --- Persistent Storage (Cache/Cookies) ---
        QString storagePath = QDir::homePath() + "/.local/share/BrowserProject/storage";
        QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();
        defaultProfile->setPersistentStoragePath(storagePath);
        defaultProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        // --- Shortcut for Downloads (Ctrl+J) ---
        new QShortcut(QKeySequence("Ctrl+J"), this, SLOT(showDownloads()));

        // --- Connections ---
        connect(newTabBtn, &QToolButton::clicked, this, [this]() { addNewTab(); });
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        // Initial Tab
        addNewTab(QUrl("https://google.com"));
    }

public slots:
    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        MyWebEngineView *view = new MyWebEngineView(this);
        
        // Settings: Enable common features
        view->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        view->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);

        view->load(url);
        
        int index = tabs->addTab(view, "New Tab");
        tabs->setCurrentIndex(index);

        // Update tab title dynamically
        connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
            int idx = tabs->indexOf(view);
            if (idx != -1) {
                QString shortTitle = title.length() > 20 ? title.left(17) + "..." : title;
                tabs->setTabText(idx, shortTitle);
            }
        });
    }

    void closeTab(int index) {
        if (tabs->count() > 1) {
            QWidget *w = tabs->widget(index);
            tabs->removeTab(index);
            delete w;
        } else {
            this->close(); // Exit app if last tab is closed
        }
    }

    void showDownloads() {
        QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }

private:
    QTabWidget *tabs;
};

// 3. Execution Entry Point
int main(int argc, char *argv[]) {
    // Crucial for high-DPI screens on Linux
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QApplication app(argc, argv);
    
    Browser browser;
    browser.setWindowTitle("Browser Beta v2");
    browser.resize(1200, 800);
    browser.show();
    
    return app.exec();
}

#include "main.moc"