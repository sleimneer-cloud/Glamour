#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include "networkservice.h"
#include "chipgamemanager.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_submitButton_clicked();           // 버튼 클릭 시 실행될 함수
    // 서버 응답 처리를 위한 새로운 슬롯들
    void onFeedbackReceived(int logId, const QJsonObject &dataObj);
    void onNetworkError(const QString &errorMessage);
    void onGameCleared(int retryCount);

    void on_generateButton_clicked();

private:
    Ui::MainWindow *ui;
    
    NetworkService *networkService;
    ChipGameManager *gameManager;
    
    QString currentFeedbackHtml; // 숨겨둘 AI 피드백 HTML
    int currentLogId = -1; // 🌟 현재 진행 중인 문장의 DB 로그 ID
    
    bool isInputKeywordLogged = false; // 1회성 로깅 제어
};
#endif // MAINWINDOW_H
