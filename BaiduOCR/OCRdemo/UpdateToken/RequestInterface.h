#pragma once

#include <QObject>
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include <qtimer.h>

class RequestInterface : public QObject
{
    Q_OBJECT

public:
    RequestInterface(QObject *parent);
    ~RequestInterface();

    void postRequest();
private:
    QTimer m_timerCutdown;
    int m_nTimeout = 0;
    QString m_strToken;
    QNetworkRequest m_request;
    QNetworkAccessManager m_manager;
    QNetworkReply *m_pReply = nullptr;
    
public Q_SLOTS:
    void onSigReply(QNetworkReply *reply);
Q_SIGNALS:
    void sigResponse(int resCode, QByteArray resData);
};
