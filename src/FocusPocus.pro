QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 1.0.0.1

SOURCES += \
    aboutdialog.cpp \
    exportdialog.cpp \
    imageprocessing.cpp \
    main.cpp \
    mainwindow.cpp \
    oddslider.cpp \
    oddspinbox.cpp \
    settings.cpp

HEADERS += \
    aboutdialog.h \
    exportdialog.h \
    imageprocessing.h \
    mainwindow.h \
    oddslider.h \
    oddspinbox.h \
    settings.h

FORMS += \
    aboutdialog.ui \
    exportdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += D:/OpenCV/opencv/build/include
DEPENDPATH += D:/OpenCV/opencv/build/include

# if OS is windows
win32 {
    win32:CONFIG(release, debug|release): LIBS += -LD:/OpenCV/opencv/build/x64/vc16/lib/ -lopencv_world4100
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OpenCV/opencv/build/x64/vc16/lib/ -lopencv_world4100d
    RC_ICONS = focuspocus.ico
}

RESOURCES += \
    resources.qrc
