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
#include <QWebEngineDownloadRequest>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>

class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser() {
        // --- 1. Tabs Manager Setup ---
        tabs = new QTabWidget(this);
        tabs->setTabsClosable(true);
        tabs->setMovable(true); // Fancy touch
        setCentralWidget(tabs);

        // The "+" button for new tabs
        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        newTabBtn->setCursor(Qt::PointingHandCursor);
        newTabBtn->setStyleSheet("QToolButton:hover { background-color: #444; }"); // Basic hover animation
        tabs->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

        // --- 2. Persistent Storage ---
        // This keeps you logged into sites like GitHub/YouTube
        QString storagePath = QDir::homePath() + "/.local/share/BrowserProject/storage";
        QWebEngineProfile::defaultProfile()->setPersistentStoragePath(storagePath);
        QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        // --- 3. Downloads Shortcut ---
        new QShortcut(QKeySequence("Ctrl+J"), this, SLOT(showDownloads()));

        // Signals
        connect(newTabBtn, &QToolButton::clicked, this, &Browser::addNewTab);
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        // Open initial tab
        addNewTab();
    }

public slots:
    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        MyWebEngineView *view = new MyWebEngineView();
        view->load(url);
        
        int index = tabs->addTab(view, "New Tab");
        tabs->setCurrentIndex(index);

        // Update tab title when page loads
        connect(view, &QWebEngineView::titleChanged, [=](const QString &title) {
            tabs->setTabText(tabs->indexOf(view), title);
        });
    }

    void closeTab(int index) {
        if (tabs->count() > 1) {
            QWidget *w = tabs->widget(index);
            tabs->removeTab(index);
            delete w;
        } else {
            close(); // Exit if last tab closed
        }
    }

    void showDownloads() {
        // For Beta v2, we can just open the Downloads folder in the file manager
        // A full UI manager can be added in v2.1
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    }

private:
    QTabWidget *tabs;
};