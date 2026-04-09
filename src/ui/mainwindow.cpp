#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QInputMethod>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(950, 850);                 // 창 크기 고정
    ui->inputTextEdit->setFixedHeight(120);       // 텍스트 입력칸 크기 고정
    ui->outputTextBrowser->setMinimumHeight(400); // 피드백 출력칸 최소 높이 설정

    // ibus 유령 글자 퇴마 로직
    connect(qApp, &QApplication::focusChanged, this, [](QWidget *oldWidget, QWidget *nowWidget)
            {
        // 1. 이전 칸에서 나갈 때: "지금까지 친 거 무조건 확정해!"
        if (oldWidget && oldWidget->inherits("QLineEdit")) {
            QGuiApplication::inputMethod()->commit();
        }

        // 2. 새 칸에 들어올 때: "혹시라도 따라온 유령 글자가 있다면 0.01초 만에 지워버려!"
        if (nowWidget && nowWidget->inherits("QLineEdit")) {
            QLineEdit *edit = qobject_cast<QLineEdit*>(nowWidget);
            // 아주 미세한 지연(Timer)을 주어 ibus가 글자를 던진 직후에 덮어씌웁니다.
            QTimer::singleShot(10, edit, [edit]() {
                if (!edit->text().isEmpty()) {
                    // 만약 사용자가 타이핑하기도 전에 글자가 있다면 그건 유령입니다.
                    edit->clear();
                }
            });
        } });

    // 입력칸에 힌트(Placeholder) 텍스트 추가
    ui->inputTextEdit->setPlaceholderText("하나의 문장을 완성할 단어들을 입력해 주세요. (최대 100자)");

    // 텍스트가 변경될 때마다 글자 수를 검사하는 로직 연결
    connect(ui->inputTextEdit, &QTextEdit::textChanged, this, [this]()
            {
        QString text = ui->inputTextEdit->toPlainText();
        int maxLength = 100; // 최대 글자 수 제한

        if (text.length() > maxLength) {
            // 제한을 넘기면 마지막에 친 글자를 자릅니다.
            text.chop(text.length() - maxLength);
            ui->inputTextEdit->setPlainText(text);
            
            // 커서를 다시 맨 끝으로 이동시켜줍니다.
            QTextCursor cursor = ui->inputTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->inputTextEdit->setTextCursor(cursor);
        } });

    // 🌟 분리된 서비스 / 매니저 객체 생성 및 시그널 연결
    networkService = new NetworkService(this);
    gameManager = new ChipGameManager(this);

    connect(networkService, &NetworkService::feedbackReceived, this, &MainWindow::onFeedbackReceived);
    connect(networkService, &NetworkService::errorOccurred, this, &MainWindow::onNetworkError);
    connect(gameManager, &ChipGameManager::gameCleared, this, &MainWindow::onGameCleared);
    
    // 🌟 Amplitude: 칩 상호작용 로깅 연결
    connect(gameManager, &ChipGameManager::chipInteracted, this, [this](bool isCorrect, const QString &chipText) {
        QJsonObject props;
        props["is_correct"] = isCorrect;
        props["chip_text"] = chipText;
        networkService->sendAmplitudeEvent("interact_chip", props);
    });

    // 🌟 Amplitude: 육하원칙 키워드 입력 시작 로깅 (1회만 기록)
    auto inputLogger = [this]() {
        if (!isInputKeywordLogged) {
            networkService->sendAmplitudeEvent("input_keywords");
            isInputKeywordLogged = true;
        }
    };
    connect(ui->whoEdit, &QLineEdit::textEdited, this, inputLogger);
    connect(ui->whenEdit, &QLineEdit::textEdited, this, inputLogger);
    connect(ui->whereEdit, &QLineEdit::textEdited, this, inputLogger);
    connect(ui->whatEdit, &QLineEdit::textEdited, this, inputLogger);
    connect(ui->howEdit, &QLineEdit::textEdited, this, inputLogger);
    connect(ui->whyEdit, &QLineEdit::textEdited, this, inputLogger);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 💡 [AI 피드백 받기] 버튼을 눌렀을 때 실행되는 곳!
void MainWindow::on_submitButton_clicked()
{
    // 🌟 Amplitude: AI 피드백 버튼 클릭 로깅
    networkService->sendAmplitudeEvent("request_ai_feedback");
    QString text = ui->inputTextEdit->toPlainText();
    QString situation = ui->situationComboBox->currentText();

    if (text.isEmpty())
    {
        ui->outputTextBrowser->setText("텍스트를 입력해주세요!");
        return;
    }

    ui->outputTextBrowser->setText("AI 서버가 분석 중입니다... 잠시만 기다려주세요 🚀");

    QString who = ui->whoEdit->text();
    QString when = ui->whenEdit->text();
    QString where = ui->whereEdit->text();
    QString what = ui->whatEdit->text();
    QString how = ui->howEdit->text();
    QString why = ui->whyEdit->text();

    QString intentForAI = QString("누가: %1, 무엇을: %2, 어떻게: %3, 어디서: %4, 언제: %5, 왜: %6")
                              .arg(who, what, how, where, when, why);

    // 🌟 분리된 NetworkService 를 이용해 요청 전송
    networkService->requestFeedback(1, text, situation, intentForAI);
}

// 🌟 NetworkService 로부터 서버 응답 성공 시그널을 받을 때
void MainWindow::onFeedbackReceived(int logId, const QJsonObject &dataObj)
{
    currentLogId = logId;
    QJsonObject feedbackObj = dataObj["feedback"].toObject();

    // ==========================================================
    // 💡 1. 피드백 은닉 (화면에 바로 안 띄우고 currentFeedbackHtml에 저장)
    // ==========================================================
    currentFeedbackHtml = "<b>[교정된 문장]</b><br>" + dataObj["corrected"].toString() + "<br><br>";
    currentFeedbackHtml += "<b>[오류 유형]</b><br>" + dataObj["error_category"].toString() + "<br><br>";
    currentFeedbackHtml += "<b>[상세 설명]</b><br>";

    QString strengths = feedbackObj["strengths"].toString();
    if (!strengths.isEmpty())
        currentFeedbackHtml += "<span style='color:#2e7d32;'><b>✅ 잘한 점:</b></span> " + strengths + "<br>";

    QString improvements = feedbackObj["improvements"].toString();
    if (!improvements.isEmpty())
        currentFeedbackHtml += "<span style='color:#c62828;'><b>⚠️ 보완할 점:</b></span> " + improvements + "<br>";

    QString cheerUp = feedbackObj["cheer_up"].toString();
    if (!cheerUp.isEmpty())
        currentFeedbackHtml += "<span style='color:#1565c0;'><b>✨ 응원:</b></span> <i>" + cheerUp + "</i><br><br>";
    else
        currentFeedbackHtml += "<br>";

    currentFeedbackHtml += "<b>[과거 오답노트 반영]</b><br>" + dataObj["past_errors_used"].toString();

    // 화면(outputTextBrowser)에는 피드백 대신 조립 시작 안내를 띄웁니다!
    ui->outputTextBrowser->setHtml("<h3 style='color:#1565c0;'>🧩 칩이 준비되었습니다! 순서대로 클릭하여 문장을 완성해 보세요.</h3>");

    // ==========================================================
    // 💡 2. 분리된 ChipGameManager 를 이용해 게임 시작!
    // ==========================================================
    QJsonArray chipsArray = dataObj["chips"].toArray();
    gameManager->startGame(chipsArray, ui->chipsLayout, this);
}

// 🌟 NetworkService 로부터 네트워크 오류/실패 시그널을 받을 때
void MainWindow::onNetworkError(const QString &errorMessage)
{
    ui->outputTextBrowser->setText(errorMessage);
}

// 🌟 ChipGameManager 로부터 게임 클리어 시그널을 받을 때
void MainWindow::onGameCleared(int retryCount)
{
    // 🌟 Amplitude: 칩 조립 완료 로깅
    QJsonObject props;
    props["retry_count"] = retryCount;
    networkService->sendAmplitudeEvent("complete_chip_assembly", props);
    
    // 다음 문장 생성을 위해 로깅 상태 리셋
    isInputKeywordLogged = false;

    // 게임 클리어!
    ui->outputTextBrowser->setHtml(
        "<h2 style='color:#2e7d32;'>🎉 완벽하게 조립했습니다!</h2>" +
        currentFeedbackHtml
    );

    // 게임이 끝나면 오답 횟수를 서버로 통신 객체를 통해 백그라운드 전송!
    if (currentLogId != -1 && retryCount > 0) {
        networkService->sendRetryCount(currentLogId, retryCount);
    }
}

void MainWindow::on_generateButton_clicked()
{
    // 🌟 Amplitude: 가이드 생성 로깅
    networkService->sendAmplitudeEvent("click_generate_guide");

    // 1. 왼쪽 폼에서 사용자가 적은 텍스트 싹 다 가져오기
    QString who = ui->whoEdit->text();
    QString when = ui->whenEdit->text();
    QString where = ui->whereEdit->text();
    QString what = ui->whatEdit->text();
    QString how = ui->howEdit->text();
    QString why = ui->whyEdit->text();

    // 🌟 핵심 방어 로직 추가: 'what'이 비어있지 않고, 끝이 '다'로 끝나지 않으면 서술어 느낌을 더해줌
    if (!what.isEmpty() && !what.endsWith("다"))
    {
        what += " (한다/했다)";
    }

    // 2. 영어 어순(SVOMPT + Reason)으로 문자열 조립하기 올바른 수정본!
    // 순서: 누가 ➔ 무엇을 ➔ 어떻게 ➔ 어디서 ➔ 언제 ➔ 왜
    QString guideText = "[ " + who + " ] ➔ [ " + what + " ] ➔ [ " + how + " ] ➔ [ " + where + " ] ➔ [ " + when + " ] ➔ [ " + why + " ]";

    // 3. 오른쪽 상단 가이드 라벨에 조립된 문자열 띄우기
    ui->guideLabel->setText(guideText);
}
