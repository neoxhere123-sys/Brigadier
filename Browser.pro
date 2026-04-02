TEMPLATE = app
TARGET = Browser

QT += widgets webenginewidgets

SOURCES += main.cpp

# Ensure the compiler can find the headers
INCLUDEPATH += .

# Optimization for Arch
CONFIG += c++17