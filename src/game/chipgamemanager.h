#ifndef CHIPGAMEMANAGER_H
#define CHIPGAMEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QLayout>
#include <QWidget>
#include <QJsonArray>

class ChipGameManager : public QObject
{
    Q_OBJECT
public:
    explicit ChipGameManager(QObject *parent = nullptr);

    // 새 게임 시작 (칩스 배열, 버튼들을 추가할 레이아웃, 버튼들의 부모 위젯 지정)
    void startGame(const QJsonArray &chipsArray, QLayout *targetLayout, QWidget *parentWidget);

signals:
    // 게임 클리어 (오답 횟수 전달)
    void gameCleared(int retryCount);
    // 칩 상호작용 (Amplitude 로깅용)
    void chipInteracted(bool isCorrect, const QString &chipText);

private:
    QStringList originalChips;
    int currentChipIndex;
    int retryCount;
    
    // 레이아웃 초기화
    void clearLayout(QLayout *layout);
};

#endif // CHIPGAMEMANAGER_H
