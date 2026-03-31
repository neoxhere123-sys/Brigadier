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

// 1. MUST COME FIRST: The Custom View
class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    MyWebEngineView(QWidget *parent = nullptr) : QWebEngineView(parent) {}

protected:
    QWebEngineView *createWindow(QWebEngineType type) override {
        Q_UNUSED(type);
        return this; // Force target="_blank" into same tab
    }
};

// 2. MUST COME SECOND: The Main Browser Window
class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser(QWidget *parent = nullptr) : QMainWindow(parent) {
        // Tabs Setup
        tabs = new QTabWidget(this);
        tabs->setTabsClosable(true);
        tabs->setMovable(true);
        setCentralWidget(tabs);

        // New Tab Button
        QToolButton *newTabBtn = new QToolButton();
        newTabBtn->setText("+");
        tabs->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

        // Persistence
        QString storagePath = QDir::homePath() + "/.local/share/BrowserProject/storage";
        QWebEngineProfile::defaultProfile()->setPersistentStoragePath(storagePath);
        QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        // Shortcut
        new QShortcut(QKeySequence("Ctrl+J"), this, SLOT(showDownloads()));

        // Connections
        connect(newTabBtn, &QToolButton::clicked, this, [=]() { addNewTab(); });
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        
        addNewTab(QUrl("https://google.com"));
    }

public slots:
    void addNewTab(const QUrl &url = QUrl("https://google.com")) {
        MyWebEngineView *view = new MyWebEngineView(this);
        view->load(url);
        
        int index = tabs->addTab(view, "Loading...");
        tabs->setCurrentIndex(index);

        connect(view, &QWebEngineView::titleChanged, this, [=](const QString &title) {
            int idx = tabs->indexOf(view);
            if (idx != -1) tabs->setTabText(idx, title);
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

// 3. MAIN FUNCTION
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Browser Beta v2");
    browser.resize(1200, 800);
    browser.show();
    return app.exec();
}

#include "main.moc"