#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    // 🌟 우분투 ibus 꼬리물기 버그 해결을 위한 환경 변수 세팅
    // ibus가 글자를 비동기로 처리하지 않고, 즉시 꽂아넣도록 설정합니다.
    qputenv("QT_IM_MODULE", QByteArray("ibus"));
    qputenv("IBUS_ENABLE_SYNC_MODE", QByteArray("1")); // 👈 이 줄이 핵심입니다! 동기 모드 강제.
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
