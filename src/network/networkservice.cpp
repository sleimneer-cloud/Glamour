#include "networkservice.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QUuid>
#include <QSettings>

// 서버 기본 주소 (추후 도메인으로 변경 시 이곳만 수정하면 됩니다)
const QString BASE_URL = "http://43.201.133.91:8000";

NetworkService::NetworkService(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &NetworkService::onReplyFinished);

    // 🌟 기기 고유 ID 생성 로직 (로그인이 없으므로 PC 식별용)
    QSettings settings("GlamourAI", "GlamourClient");
    if (!settings.contains("device_id")) {
        // 최초 실행 시 고유 UUID 발급 및 로컬 저장
        settings.setValue("device_id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    deviceId = settings.value("device_id").toString();
}

void NetworkService::requestFeedback(int userId, const QString &text, const QString &situation, const QString &intent)
{
    QJsonObject json;
    json["user_id"] = userId;
    json["text"] = text;
    json["situation"] = situation;
    json["intent"] = intent;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QUrl url(BASE_URL + "/process-text");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    networkManager->post(request, data);
}

void NetworkService::sendRetryCount(int logId, int retryCount)
{
    QJsonObject retryJson;
    retryJson["log_id"] = logId;
    retryJson["retry_count"] = retryCount;
    
    QJsonDocument retryDoc(retryJson);
    QByteArray retryData = retryDoc.toJson();

    QUrl retryUrl(BASE_URL + "/update-retries");
    QNetworkRequest retryReq(retryUrl);
    retryReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    networkManager->post(retryReq, retryData);
}

void NetworkService::sendAmplitudeEvent(const QString &eventType, const QJsonObject &eventProperties)
{
    QJsonObject event;
    // 발급받은 기기 고유 ID 사용 (10명의 친구가 전부 다르게 잡힙니다)
    event["device_id"] = deviceId; 
    event["event_type"] = eventType;
    
    if (!eventProperties.isEmpty()) {
        event["event_properties"] = eventProperties;
    }

    QJsonArray eventsArray;
    eventsArray.append(event);

    QJsonObject payload;
    // 제공해주신 키를 사용합니다.
    payload["api_key"] = "e866fb61498a0253bbf78cb1c6b81ca9";
    payload["events"] = eventsArray;

    QJsonDocument doc(payload);
    QByteArray data = doc.toJson();

    QUrl url("https://api2.amplitude.com/2/httpapi");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    networkManager->post(request, data);
}

void NetworkService::onReplyFinished(QNetworkReply *reply)
{
    // 오답 횟수 기록 API의 응답이라면 무시
    if (reply->request().url().toString().contains("update-retries"))
    {
        reply->deleteLater();
        return;
    }

    // Amplitude 이벤트 전송 응답 무시
    if (reply->request().url().host().contains("amplitude.com"))
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject jsonObj = doc.object();

        if (jsonObj["status"].toString() == "success")
        {
            int logId = jsonObj["log_id"].toInt();
            QJsonObject dataObj = jsonObj["data"].toObject();
            dataObj["past_errors_used"] = jsonObj["past_errors_used"]; // For UI compatibility
            emit feedbackReceived(logId, dataObj);
        }
        else
        {
            emit errorOccurred("서버 분석 실패: " + jsonObj["message"].toString());
        }
    }
    else
    {
        emit errorOccurred("서버 연결 에러: 메인 서버(8000)가 켜져 있는지 확인하세요!");
    }
    
    reply->deleteLater();
}
