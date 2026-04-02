#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWebEngineView>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QWebEngineDownloadRequest>

// Robust Raw String for the Home Page
const QString BRIGADIER_HOME = R"=====(
<html>
<head>
    <style>
        body { background: #121212; color: #e0e0e0; font-family: sans-serif; 
               display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }
        h1 { font-size: 72px; letter-spacing: 10px; color: #ffffff; margin-bottom: 20px; text-transform: uppercase; }
        .search-box { width: 50%; position: relative; }
        input { width: 100%; padding: 15px 25px; border-radius: 30px; border: 1px solid #333; 
                background: #1e1e1e; color: white; font-size: 18px; outline: none; transition: 0.3s; }
        input:focus { border-color: #00afff; box-shadow: 0 0 10px rgba(0,175,255,0.3); }
    </style>
</head>
<body>
    <h1>BRIGADIER</h1>
    <div class="search-box">
        <input type="text" id="search" placeholder="Search or enter URL..." onkeydown="if(event.key==='Enter') search()">
    </div>
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
)=====";

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