#include "RequestInterface.h"
#include <qbuffer.h>
#include <fstream>
#include <Windows.h>
#include <qsettings.h>

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
    m_strToken = setting.value("/config/token", "").toString();
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
void RequestInterface::postRequest(int cardType, QImage cardImg)
{
    printLog(QString("request type:%1 imgSize:%2*%3").arg(cardType).arg(cardImg.width()).arg(cardImg.height()));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QSslConfiguration conf = m_request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    m_request.setSslConfiguration(conf);

    QByteArray data;
    QByteArray imgByte;
    QBuffer buf(&imgByte);
    cardImg.save(&buf, "jpg");
    imgByte = imgByte.toBase64().toPercentEncoding();
    data.append("image=" + imgByte);
    buf.close();
    //printLog(data);

    switch (cardType) {
    case CARD_TYPE_ID:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/idcard?access_token=" + m_strToken));
        data.append("&id_card_side=front");
        break;
    case CARD_TYPE_ID_BACK:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/idcard?access_token=" + m_strToken));
        data.append("&id_card_side=back");
        break;
    case CARD_TYPE_BANK:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/bankcard?access_token=" + m_strToken));
        break;
    case CARD_TYPE_HK:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/HK_Macau_exitentrypermit?access_token=" + m_strToken));
        break;
    case CARD_TYPE_TW:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/taiwan_exitentrypermit?access_token=" + m_strToken));
        break;
    case CARD_TYPE_HOUSEHOLD:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/household_register?access_token=" + m_strToken));
        break;
    case CARD_TYPE_PASSPORT:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/passport?access_token=" + m_strToken));
        break;
    case CARD_TYPE_DRIVERLIC:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/driving_license?access_token=" + m_strToken));
        break;
    case CARD_TYPE_DRIVERLIC2:
        m_request.setUrl(QUrl("https://aip.baidubce.com/rest/2.0/ocr/v1/vehicle_license?access_token=" + m_strToken));
        break;
    default:
        emit sigResponse(0, "类型错误");
        return;
    }

    m_pReply = m_manager.post(m_request, data);
}

// 处理应答
void RequestInterface::onSigReply(QNetworkReply *reply)
{
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray resBytes = reply->readAll();
    printLog(resBytes);

    if (reply->error() == QNetworkReply::NoError)
    {
        ;
    }
    else
    {
        resBytes = reply->errorString().toUtf8();
    }

    emit sigResponse(code, resBytes);
}