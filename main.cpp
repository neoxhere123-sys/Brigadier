#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWebEngineView>
#include <QHBoxLayout>

class Browser : public QMainWindow {
public:
    Browser() {
        // 1. Create the widgets
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        QHBoxLayout *controlsLayout = new QHBoxLayout();
        addressBar = new QLineEdit();
        QPushButton *goButton = new QPushButton("Go");
        
        webView = new QWebEngineView();

        // 2. Set up the layout
        controlsLayout->addWidget(addressBar);
        controlsLayout->addWidget(goButton);
        
        layout->addLayout(controlsLayout);
        layout->addWidget(webView);
        
        setCentralWidget(centralWidget);

        // 3. Connect signals and slots
        connect(goButton, &QPushButton::clicked, this, &Browser::loadUrl);
        connect(addressBar, &QLineEdit::returnPressed, this, &Browser::loadUrl);

        // Default page
        addressBar->setText("https://www.google.com");
        loadUrl();
    }

private slots:
    void loadUrl() {
        QString url = addressBar->text();
        if (!url.startsWith("http")) {
            url = "https://" + url;
        }
        webView->load(QUrl(url));
    }

private:
    QLineEdit *addressBar;
    QWebEngineView *webView;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    Browser browser;
    browser.setWindowTitle("Browser");
    browser.resize(1024, 768);
    browser.show();
    
    return app.exec();
}