#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QToolBar>
#include <QLineEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineDownloadRequest>
#include <QWebEngineFullScreenRequest>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QStyle>
#include <QAction>
#include <QEvent>
#include <QTimer>

// --- THE BRIGADIER HOME PAGE HTML ---
const QString BRIGADIER_HOME = R"(
<html>
<head>
    <style>
        body { background: #121212; color: #e0e0e0; font-family: 'Segoe UI', sans-serif; 
               display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }
        h1 { font-size: 72px; letter-spacing: 10px; color: #ffffff; margin-bottom: 20px; text-shadow: 2px 2px 10px rgba(0,0,0,0.5); }
        .search-container { width: 50%; position: relative; }
        input { width: 100%; padding: 15px 25px; border-radius: 30px; border: 1px solid #333; 
                background: #1e1e1e; color: white; font-size: 18px; outline: none; transition: 0.3s; }
        input:focus { border-color: #00afff; box-shadow: 0 0 15px rgba(0, 175, 255, 0.2); }
        p { margin-top: 20px; color: #666; font-size: 14px; }
    </style>
</head>
<body>
    <h1>BRIGADIER</h1>
    <div class="search-container">
        <input type="text" id="search" placeholder="Search with Google or enter URL..." onkeydown="if(event.key==='Enter') search()">
    </div>
    <p>Beta v3.0 | Fast. Minimal. Arch Ready.</p>
    <script>
        function search() {
            var val = document.getElementById('search').value;
            if (val.includes('.') && !val.includes(' ')) {
                window.location.href = val.startsWith('http') ? val : 'https://' + val;
            } else {
                window.location.href = 'https://www.google.com/search?q=' + encodeURIComponent(val);
            }
        }
    </script>
</body>
</html>
)";

class MyWebEngineView : public QWebEngineView {
    Q_OBJECT
public:
    MyWebEngineView(QWidget *parent = nullptr) : QWebEngineView(parent) {
        settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        connect(page(), &QWebEnginePage::fullScreenRequested, this, [this](QWebEngineFullScreenRequest request) {
            if (request.toggleOn()) {
                if (window()) window()->showFullScreen();
                request.accept();
            } else {
                if (window()) window()->showNormal();
                request.accept();
            }
            QTimer::singleShot(200, this, [this]() { this->update(); this->repaint(); });
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

        QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
        connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, this, &Browser::handleDownload);
        connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
        connect(tabs, &QTabWidget::currentChanged, this, &Browser::updateAddressBar);

        addNewTab(); // Opens to Home Page by default
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
        mainToolBar = addToolBar("Navigation");
        mainToolBar->setMovable(false);

        // Back/Forward Buttons
        backAction = mainToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Back");
        forwardAction = mainToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Forward");
        
        connect(backAction, &QAction::triggered, this, [this]() {
            if (auto *v = currentView()) v->back();
        });
        connect(forwardAction, &QAction::triggered, this, [this]() {
            if (auto *v = currentView()) v->forward();
        });

        mainToolBar->addSeparator();

        // Address Bar (Omnibox)
        addressBar = new QLineEdit(this);
        addressBar->setPlaceholderText("Enter URL or Search...");
        connect(addressBar, &QLineEdit::returnPressed, this, &Browser::handleAddressInput);
        mainToolBar->addWidget(addressBar);

        // Standard Actions
        QAction *newTabAct = mainToolBar->addAction("+");
        connect(newTabAct, &QAction::triggered, this, [this]() { addNewTab(); });

        mainToolBar->addSeparator();
        
        QAction *dlAction = mainToolBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Downloads");
        connect(dlAction, &QAction::triggered, this, &Browser::showDownloadsFolder);
    }

    void handleAddressInput() {
        QString input = addressBar->text();
        if (input.isEmpty()) return;

        QUrl url;
        if (input.contains('.') && !input.contains(' ')) {
            url = QUrl::fromUserInput(input);
        } else {
            url = QUrl("https://www.google.com/search?q=" + input);
        }
        
        if (auto *v = currentView()) v->load(url);
    }

    void updateAddressBar(int index) {
        if (auto *v = currentView()) {
            QString url = v->url().toString();
            // Don't show the messy HTML string if we are on home page
            addressBar->setText(url.startsWith("data") ? "" : url);
        }
    }

    MyWebEngineView* currentView() const {
        return qobject_cast<MyWebEngineView*>(tabs->currentWidget());
    }

    void addNewTab(const QUrl &url = QUrl()) {
        MyWebEngineView *view = new MyWebEngineView(this);
        if (url.isEmpty()) {
            view->setHtml(BRIGADIER_HOME);
        } else {
            view->load(url);
        }

        int index = tabs->addTab(view, "Home");
        tabs->setCurrentIndex(index);

        connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &title) {
            int idx = tabs->indexOf(view);
            if (idx != -1) tabs->setTabText(idx, title.left(15));
        });

        connect(view, &QWebEngineView::urlChanged, this, [this]() {
            updateAddressBar(tabs->currentIndex());
        });
    }

    void handleDownload(QWebEngineDownloadRequest *download) {
        download->setDownloadDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        download->accept();
        statusBar()->showMessage("Download started...", 3000);
    }

    void showDownloadsFolder() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
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
    QLineEdit *addressBar;
    QAction *backAction;
    QAction *forwardAction;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Brigadier Beta v3.0");
    browser.resize(1280, 720);
    browser.show();
    return app.exec();
}

#include "main.moc"