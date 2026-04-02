#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWebEngineView>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QWebEngineDownloadRequest>

// The Home Page HTML string
const QString BRIGADIER_HOME = R"(
<html>
<head>
    <style>
        body { background: #121212; color: #e0e0e0; font-family: sans-serif; 
               display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }
        h1 { font-size: 72px; letter-spacing: 10px; color: #ffffff; margin-bottom: 20px; }
        input { width: 50%; padding: 15px 25px; border-radius: 30px; border: 1px solid #333; 
                background: #1e1e1e; color: white; font-size: 18px; outline: none; }
    </style>
</head>
<body>
    <h1>BRIGADIER</h1>
    <input type="text" id="search" placeholder="Search or enter URL..." onkeydown="if(event.key==='Enter') search()">
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
    MyWebEngineView(QWidget *parent = nullptr);
protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;
};

class Browser : public QMainWindow {
    Q_OBJECT
public:
    Browser(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void setupTopBar();
    void handleAddressInput();
    void updateAddressBar(int index);
    void handleDownload(QWebEngineDownloadRequest *download);
    void addNewTab(const QUrl &url = QUrl());
    void closeTab(int index);

private:
    QTabWidget *tabs;
    QToolBar *mainToolBar;
    QLineEdit *addressBar;
    QAction *backAction;
    QAction *forwardAction;
    MyWebEngineView* currentView() const;
};

#endif