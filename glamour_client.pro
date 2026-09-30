QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 빌드 시 생성되는 파일들(오브젝트, moc 등)을 별도 폴더로 모아서 루트 디렉토리 정리
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
UI_DIR = build/ui
RCC_DIR = build/rcc
DESTDIR = build/bin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src src/ui src/network src/game

SOURCES += \
    src/main.cpp \
    src/ui/mainwindow.cpp \
    src/network/networkservice.cpp \
    src/game/chipgamemanager.cpp

HEADERS += \
    src/ui/mainwindow.h \
    src/network/networkservice.h \
    src/game/chipgamemanager.h

FORMS += \
    src/ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/res.qrc

# 최신 Xcode SDK에서 AGL(레거시 OpenGL API) 프레임워크가 제거되어 링크 실패함.
# 이 앱은 AGL을 직접 쓰지 않으므로 Qt가 기본으로 붙이는 링크 목록에서 제외.
macx: LIBS -= -framework AGL
