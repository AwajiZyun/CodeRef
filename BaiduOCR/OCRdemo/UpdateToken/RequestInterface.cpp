#include "RequestInterface.h"
#include <qbuffer.h>
#include <fstream>
#include <Windows.h>
#include <qsettings.h>
#include <QMessageBox>
#include <qjsondocument.h>
#include <qjsonobject.h>
#pragma execution_character_set("utf-8")

#define AK  "OlsAbm3zC3kDlotM6hhPNK3V"
#define SK  "AtCFZY3SVvihnIfL54u7omaV0zlATezA"

void printLog(QString log) 
{
    std::ofstream out;
    out.open("log.log", std::ios::app);
    if (out.is_open()) {
        SYSTEMTIME sysTime;
        ::GetLocalTime(&sysTime);
        char szTime[64] = { 0 };
        sprintf_s(szTime, "%02d/%02d-%02d:%02d:%02d-%03d\t",
            sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
        out.write(szTime, strlen(szTime));
        out.write(log.toStdString().data(), log.toStdString().size());
        out.write("\r\n", sizeof("\r\n") - 1);
        out.close();
    }
}

RequestInterface::RequestInterface(QObject *parent)
    : QObject(parent)
{
    QSettings setting("OCRdemo.ini", QSettings::IniFormat);
    m_nTimeout = setting.value("/confit/timeout", 5000).toInt();
    m_timerCutdown.setInterval(m_nTimeout);
    m_timerCutdown.setSingleShot(true);

    connect(&m_manager, &QNetworkAccessManager::finished, this, &RequestInterface::onSigReply, Qt::QueuedConnection);
    connect(&m_timerCutdown, &QTimer::timeout, this, [this]() {
        printLog("reply timeout");
        if (m_pReply) {
            m_pReply->close();
            m_pReply = nullptr;
        }
    });
}

RequestInterface::~RequestInterface()
{
}

// 发送OCR请求
void RequestInterface::postRequest()
{
    QSslConfiguration conf = m_request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    m_request.setSslConfiguration(conf);

    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    m_request.setUrl(QUrl(QString("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=%1&client_secret=%2")
        .arg(AK).arg(SK)));
    //printf(m_request.url().toString().toStdString().data());
    m_pReply = m_manager.post(m_request, "");
}

// 处理应答
void RequestInterface::onSigReply(QNetworkReply *reply)
{
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray resBytes = reply->readAll();
    printLog(resBytes);

    if (reply->error() == QNetworkReply::NoError)
    {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(resBytes);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            //QMessageBox::critical(nullptr, "错误", "错误应答");
            printf("wrong response");
        }
        else {
            //printf(resBytes.toStdString().data());
            QJsonObject json = jsonDoc.object();
            QString token = json["access_token"].toString();
            QSettings setting("OCRdemo.ini", QSettings::IniFormat);
            setting.setValue("/config/token", token);
            //QMessageBox::critical(nullptr, "提示", "token已更新");
            printf("OK");
        }
    }
    else
    {
        resBytes = reply->errorString().toUtf8();
        //QMessageBox::critical(nullptr, "警告", "更新token失败:" + resBytes);
        printf(resBytes);
    }

    emit sigResponse(code, resBytes);
}