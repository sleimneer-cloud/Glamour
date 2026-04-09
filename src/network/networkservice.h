#ifndef NETWORKSERVICE_H
#define NETWORKSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class NetworkService : public QObject
{
    Q_OBJECT
public:
    explicit NetworkService(QObject *parent = nullptr);
    
    // 분석 요청 보내기
    void requestFeedback(int userId, const QString &text, const QString &situation, const QString &intent);
    
    // 게임 클리어 후 오답 횟수 전송
    void sendRetryCount(int logId, int retryCount);

    // Amplitude 이벤트 전송
    void sendAmplitudeEvent(const QString &eventType, const QJsonObject &eventProperties = QJsonObject());

signals:
    // 서버 응답이 성공적으로 도착했을 때 (logId와 data(feedback, chips 등) 객체 반환)
    void feedbackReceived(int logId, const QJsonObject &dataObj);
    
    // 네트워크 오류나 서버 측 에러 발생 시
    void errorOccurred(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QString deviceId;
};

#endif // NETWORKSERVICE_H
