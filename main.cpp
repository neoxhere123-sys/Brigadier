#include "browser.h"
#include <QApplication>
#include <QTabBar>
#include <QProgressBar>
#include <QStatusBar>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineFullScreenRequest>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QStyle>
#include <QTimer>
#include <QEvent>

MyWebEngineView::MyWebEngineView(QWidget *parent) : QWebEngineView(parent) {
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    connect(page(), &QWebEnginePage::fullScreenRequested, this, [](QWebEngineFullScreenRequest request) {
        if (request.toggleOn()) { 
            if (auto w = qobject_cast<QMainWindow*>(qApp->activeWindow())) w->showFullScreen(); 
        } else { 
            if (auto w = qobject_cast<QMainWindow*>(qApp->activeWindow())) w->showNormal(); 
        }
        request.accept();
    });
}

QWebEngineView* MyWebEngineView::createWindow(QWebEnginePage::WebWindowType type) { 
    Q_UNUSED(type);
    return this; 
}

Browser::Browser(QWidget *parent) : QMainWindow(parent) {
    tabs = new QTabWidget(this);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    tabs->setDocumentMode(true);
    setCentralWidget(tabs);
    setupTopBar();

    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, this, &Browser::handleDownload);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &Browser::closeTab);
    connect(tabs, &QTabWidget::currentChanged, this, &Browser::updateAddressBar);

    addNewTab();
}

void Browser::setupTopBar() {
    mainToolBar = addToolBar("Nav");
    backAction = mainToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "");
    forwardAction = mainToolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "");
    
    connect(backAction, &QAction::triggered, this, [this]() { if(currentView()) currentView()->back(); });
    connect(forwardAction, &QAction::triggered, this, [this]() { if(currentView()) currentView()->forward(); });

    addressBar = new QLineEdit(this);
    addressBar->setPlaceholderText("Search or enter URL...");
    connect(addressBar, &QLineEdit::returnPressed, this, &Browser::handleAddressInput);
    mainToolBar->addWidget(addressBar);

    QAction *newTabAct = mainToolBar->addAction("+");
    connect(newTabAct, &QAction::triggered, this, [this]() { addNewTab(); });
}

void Browser::handleAddressInput() {
    QString input = addressBar->text();
    QUrl url = (input.contains('.') && !input.contains(' ')) ? QUrl::fromUserInput(input) : QUrl("https://google.com/search?q="+input);
    if(currentView()) currentView()->load(url);
}

void Browser::updateAddressBar(int index) {
    Q_UNUSED(index);
    if(currentView()) {
        QString urlStr = currentView()->url().toString();
        addressBar->setText(urlStr.startsWith("data") ? "" : urlStr);
    }
}

MyWebEngineView* Browser::currentView() const { 
    return qobject_cast<MyWebEngineView*>(tabs->currentWidget()); 
}

void Browser::addNewTab(const QUrl &url) {
    MyWebEngineView *view = new MyWebEngineView(this);
    if(url.isEmpty()) {
        view->setHtml(BRIGADIER_HOME);
    } else {
        view->load(url);
    }
    
    tabs->addTab(view, "Home");
    tabs->setCurrentWidget(view);
    
    connect(view, &QWebEngineView::titleChanged, this, [this, view](const QString &t){
        int i = tabs->indexOf(view); 
        if(i != -1) tabs->setTabText(i, t.left(12));
    });
}

void Browser::closeTab(int index) { 
    if(tabs->count() > 1) { 
        QWidget *w = tabs->widget(index);
        tabs->removeTab(index);
        w->deleteLater();
    } else {
        close(); 
    }
}

void Browser::handleDownload(QWebEngineDownloadRequest *d) { 
    d->accept(); 
    statusBar()->showMessage("Download started...", 2000); 
}

void Browser::changeEvent(QEvent *e) {
    if(e->type() == QEvent::WindowStateChange) {
        bool f = isFullScreen();
        mainToolBar->setVisible(!f);
        if(tabs->tabBar()) tabs->tabBar()->setVisible(!f);
    }
    QMainWindow::changeEvent(e);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Browser b;
    b.setWindowTitle("Brigadier Beta v3.0");
    b.resize(1280, 720);
    b.show();
    return app.exec();
}