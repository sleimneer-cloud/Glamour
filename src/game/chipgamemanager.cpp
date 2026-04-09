#include "chipgamemanager.h"
#include <QJsonValue>
#include <QPushButton>
#include <QTimer>
#include <algorithm>
#include <random>

ChipGameManager::ChipGameManager(QObject *parent) 
    : QObject(parent), currentChipIndex(0), retryCount(0)
{
}

void ChipGameManager::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr)
    {
        if (child->widget())
        {
            delete child->widget();
        }
        delete child;
    }
}

void ChipGameManager::startGame(const QJsonArray &chipsArray, QLayout *targetLayout, QWidget *parentWidget)
{
    clearLayout(targetLayout);
    originalChips.clear();
    currentChipIndex = 0;
    retryCount = 0;

    for (const QJsonValue &val : chipsArray)
    {
        originalChips << val.toString();
    }

    QStringList shuffledChips = originalChips;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffledChips.begin(), shuffledChips.end(), g);

    for (const QString &chipText : shuffledChips)
    {
        QPushButton *chipBtn = new QPushButton(chipText, parentWidget);
        chipBtn->setMinimumHeight(40);
        chipBtn->setCursor(Qt::PointingHandCursor);
        chipBtn->setStyleSheet("background-color: #F4E8FF; border: 1px solid #C4B5E3; border-radius: 10px; padding: 10px; font-weight: bold; font-size: 14px; color: #5E35B1;");
        
        targetLayout->addWidget(chipBtn);

        connect(chipBtn, &QPushButton::clicked, this, [this, chipBtn, chipText]() {
            if (chipText == originalChips[currentChipIndex]) {
                // 정답 액션
                emit chipInteracted(true, chipText);
                chipBtn->setStyleSheet("background-color: #E8F5E9; border: 1px solid #81C784; border-radius: 10px; padding: 10px; font-weight: bold; font-size: 14px; color: #2E7D32;");
                chipBtn->setEnabled(false);

                currentChipIndex++;

                if (currentChipIndex == originalChips.size()) {
                    emit gameCleared(retryCount);
                }
            } else {
                // 오답 액션
                retryCount++; 
                emit chipInteracted(false, chipText);
                chipBtn->setStyleSheet("background-color: #FFEBEE; border: 1px solid #E57373; border-radius: 10px; padding: 10px; font-weight: bold; font-size: 14px; color: #C62828;");

                QTimer::singleShot(300, chipBtn, [chipBtn]() {
                    chipBtn->setStyleSheet("background-color: #F4E8FF; border: 1px solid #C4B5E3; border-radius: 10px; padding: 10px; font-weight: bold; font-size: 14px; color: #5E35B1;");
                });
            }
        });
    }
}
