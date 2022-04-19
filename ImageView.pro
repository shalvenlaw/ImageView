QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CommonLibrary/GlobalTools/globaltools.cpp \
    ImageView1/imageview1.cpp \
    ImageView2/imageview2.cpp \
    VisionLibrary/visionlibrary.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CommonLibrary/GlobalTools/globaltools.h \
    ImageView1/imageview1.h \
    ImageView2/imageview2.h \
    VisionLibrary/visionlibrary.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 使用OpenCV 4.5.0 world
OPENCV450_BUILD = G:/OpenSource/OpenCV/4_5_0/install/opencv/build
win32:CONFIG(release, debug|release): LIBS += -L$$OPENCV450_BUILD/x64/vc15/lib/ -lopencv_world450
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OPENCV450_BUILD/x64/vc15/lib/ -lopencv_world450d
INCLUDEPATH += $$OPENCV450_BUILD/include


