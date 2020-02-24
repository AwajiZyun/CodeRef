#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OCRdemo.h"
#include "common.h"
#include "RequestInterface.h"
#include <qdatetime.h>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QTimer>

class OCRdemo : public QMainWindow
{
    Q_OBJECT

public:
    OCRdemo(QWidget *parent = Q_NULLPTR);

private:
    Ui::OCRdemoClass ui;
    QImage m_loadImg;

    int m_nCardType = 0;
    bool m_bWaitingResponse = false;
    RequestInterface m_req;

    int m_nFinCnts = 0;
    quint64 m_nStartTime = 0;

    QCamera *m_pCam = nullptr;
    QCameraViewfinder *m_pViewFinder = nullptr;
    QCameraImageCapture *m_pImageCapture = nullptr;
    QTimer m_timer;
public Q_SLOTS:
    void onBtnLoadImg();
    void onBtnIDCard();
    void onBtnIDCardBack();
    void onBtnBankCard();
    void onBtnHKCard();
    void onBtnTWCard();
    void onBtnHousehold();
    void onBtnPassport();
    void onBtnDriverLic();
    void onBtnDriverLic2();
    void onBtnCam();
    void onImageCaptured(int id, QImage img);
    void onImageReady(bool state);

    void onSigResponse(int resCode, QByteArray resData);
};
