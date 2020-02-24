#include "OCRdemo.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#pragma execution_character_set("utf-8")

OCRdemo::OCRdemo(QWidget *parent)
    : QMainWindow(parent)
    , m_req(this)
{
    ui.setupUi(this);

    ui.labelImg->setScaledContents(false);
    ui.statusBar->setSizeGripEnabled(false);

    connect(ui.pushButtonLoad, &QPushButton::clicked, this, &OCRdemo::onBtnLoadImg);
    connect(ui.toolButtonIDCard, &QPushButton::clicked, this, &OCRdemo::onBtnIDCard);
    connect(ui.toolButtonIDCardBack, &QPushButton::clicked, this, &OCRdemo::onBtnIDCardBack);
    connect(ui.toolButtonBankCard, &QPushButton::clicked, this, &OCRdemo::onBtnBankCard);
    connect(ui.toolButtonHKcard, &QPushButton::clicked, this, &OCRdemo::onBtnHKCard);
    connect(ui.toolButtonTWcard, &QPushButton::clicked, this, &OCRdemo::onBtnTWCard);
    connect(ui.toolButtonResident, &QPushButton::clicked, this, &OCRdemo::onBtnHousehold);
    connect(ui.toolButtonPassport, &QPushButton::clicked, this, &OCRdemo::onBtnPassport);
    connect(ui.toolButtonDriverLic, &QPushButton::clicked, this, &OCRdemo::onBtnDriverLic);
    connect(ui.toolButtonDriverLic2, &QPushButton::clicked, this, &OCRdemo::onBtnDriverLic2);
    connect(ui.pushButtonCam, &QPushButton::clicked, this, &OCRdemo::onBtnCam);

    connect(&m_req, &RequestInterface::sigResponse, this, &OCRdemo::onSigResponse);
    
    m_timer.setInterval(333);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
    });
}

// 打开关闭相机
void OCRdemo::onBtnCam()
{
    static bool open = false;
    if (!open) {
        // 遍历打开指定相机
        const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        if (cameras.isEmpty()) {
            QMessageBox::critical(nullptr, tr("错误"), tr("未找到相机"));
            return;
        }
        for (int idx = 0; idx < cameras.size(); ++idx) {
            if (ui.comboBoxCam->currentIndex() == idx)
                m_pCam = new QCamera(cameras.at(idx));
        }
        if (!m_pCam) {
            QMessageBox::critical(nullptr, tr("错误"), tr("该相机打开失败"));
            return;
        }
        
        m_pImageCapture = new QCameraImageCapture(m_pCam);
        connect(m_pImageCapture, &QCameraImageCapture::imageCaptured, this, &OCRdemo::onImageCaptured, Qt::QueuedConnection);
        connect(m_pImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &OCRdemo::onImageReady, Qt::QueuedConnection);
        m_pCam->setCaptureMode(QCamera::QCamera::CaptureStillImage);
        m_pCam->start();

        // 设置分辨率
        QList<QCameraViewfinderSettings> viewSets = m_pCam->supportedViewfinderSettings();
        int largestIdx = 0;
        for (int idx = 0; idx < viewSets.size(); ++idx) {
            if (viewSets.at(largestIdx).resolution().width() <= viewSets.at(idx).resolution().width()) {
                largestIdx = idx;
            }
        }
        m_pCam->setViewfinderSettings(viewSets.at(largestIdx));
        open = true;
        ui.pushButtonCam->setText("关闭相机");
    }
    else {
        // 关闭相机
        if (m_pCam) {
            m_pCam->stop();
            delete m_pCam;
            m_pCam = nullptr;
            delete m_pImageCapture;
        }
        disconnect(m_pImageCapture, &QCameraImageCapture::imageCaptured, this, &OCRdemo::onImageCaptured);
        disconnect(m_pImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &OCRdemo::onImageReady);
        open = false;
        ui.pushButtonCam->setText("打开相机");
    }
}

// 捕获状态
void OCRdemo::onImageReady(bool state)
{
    if (state && m_pCam) {
        m_pCam->searchAndLock();
        m_pImageCapture->capture();
        m_pCam->unlock();
    }
}

// 捕获图像
void OCRdemo::onImageCaptured(int id, QImage img)
{
    m_loadImg = img;
    if (!m_loadImg.isNull()) {
        ui.labelImg->setPixmap(
            QPixmap::fromImage(m_loadImg).scaled(ui.labelImg->width() - 5, ui.labelImg->height() - 5, Qt::KeepAspectRatio));
    }
}

// 加载图片
void OCRdemo::onBtnLoadImg()
{
    QString path = QFileDialog::getOpenFileName(this, "选择图片", ".", "image(*.jpg *.png *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        ui.lineEditPath->setText(path);
        m_loadImg = QImage(path);
        if (!m_loadImg.isNull()) {
            ui.labelImg->setPixmap(QPixmap::fromImage(m_loadImg).scaled(ui.labelImg->size(), Qt::KeepAspectRatio));
        }
        else {
            QMessageBox::warning(nullptr, tr("警告"), tr("图片路径无效，请重新选择"));
        }
    }
}

// OCR请求
void OCRdemo::onBtnIDCard()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_ID;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnIDCardBack()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_ID_BACK;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnBankCard()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_BANK;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnHKCard()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_HK;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnTWCard()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_TW;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnHousehold()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_HOUSEHOLD;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnPassport()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_PASSPORT;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnDriverLic()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_DRIVERLIC;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

void OCRdemo::onBtnDriverLic2()
{
    if (m_bWaitingResponse) {
        QMessageBox::information(nullptr, tr("提示"), tr("正在识别，请稍后"));
        return;
    }
    if (m_loadImg.isNull()) {
        QMessageBox::information(nullptr, tr("提示"), tr("请先选择图片，再进行识别"));
        return;
    }
    m_bWaitingResponse = true;
    m_nCardType = CARD_TYPE_DRIVERLIC2;
    m_req.postRequest(m_nCardType, m_loadImg);
    m_nStartTime = QDateTime::currentMSecsSinceEpoch();
}

// OCR请求应答
void OCRdemo::onSigResponse(int resCode, QByteArray resData)
{
    m_bWaitingResponse = false;
    //QMessageBox::information(nullptr, "", resData);
    do {
        if (200 == resCode) {
            ui.statusBar->showMessage(QString("识别完成 \t用时%1ms \t已识别%2张")
                .arg(QDateTime::currentMSecsSinceEpoch() - m_nStartTime).arg(++m_nFinCnts));

            QString writeContents = QDateTime::currentDateTime().toString("MM-dd hh:mm:ss");
            QJsonDocument jsonDoc = QJsonDocument::fromJson(resData);
            if (jsonDoc.isNull() || !jsonDoc.isObject()) {
                QMessageBox::critical(nullptr, "错误", "错误应答");
                break;
            }
            QJsonObject json = jsonDoc.object();
            // 错误应答
            QString errMsg = json["error_msg"].toString();
            if (!errMsg.isEmpty()) {
                QMessageBox::critical(nullptr, "错误", errMsg);
            }
            else {

                switch (m_nCardType) {
                case CARD_TYPE_ID: {
                    // 身份证
                    ui.stackedWidget->setCurrentIndex(0);
                    ui.lineEditIDaddr->setText("");
                    ui.lineEditIDbirth->setText("");
                    ui.lineEditIDname->setText("");
                    ui.lineEditIDnumber->setText("");
                    ui.lineEditIDsex->setText("");
                    ui.lineEditIDnation->setText("");

                    writeContents += ",身份证";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["姓名"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditIDname->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonSex = data1["性别"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditIDsex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonNation = data1["民族"].toObject();
                        if (!jsonNation.isEmpty()) {
                            QString nation = jsonNation["words"].toString();
                            ui.lineEditIDnation->setText(nation);
                            writeContents += "," + nation;
                        }
                        QJsonObject jsonBirth = data1["出生"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditIDbirth->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonAddr = data1["住址"].toObject();
                        if (!jsonAddr.isEmpty()) {
                            QString addr = jsonAddr["words"].toString();
                            ui.lineEditIDaddr->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonNumber = data1["公民身份号码"].toObject();
                        if (!jsonNumber.isEmpty()) {
                            QString number = jsonNumber["words"].toString();
                            ui.lineEditIDnumber->setText(number);
                            writeContents += ",'" + number;
                        }
                    }
                    break;
                }
                case CARD_TYPE_ID_BACK: {
                    // 身份证国徽面
                    ui.stackedWidget->setCurrentIndex(1);
                    ui.lineEditIDendDate->setText("");
                    ui.lineEditIDangency->setText("");
                    ui.lineEditIDstartDate->setText("");

                    writeContents += ",身份证国徽面";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonAngency = data1["签发机关"].toObject();
                        if (!jsonAngency.isEmpty()) {
                            QString angency = jsonAngency["words"].toString();
                            ui.lineEditIDangency->setText(angency);
                            writeContents += "," + angency;
                        }
                        QJsonObject jsonStartDate = data1["签发日期"].toObject();
                        if (!jsonStartDate.isEmpty()) {
                            QString startDate = jsonStartDate["words"].toString();
                            ui.lineEditIDstartDate->setText(startDate);
                            writeContents += "," + startDate;
                        }
                        QJsonObject jsonEndDate = data1["失效日期"].toObject();
                        if (!jsonEndDate.isEmpty()) {
                            QString endDate = jsonEndDate["words"].toString();
                            ui.lineEditIDendDate->setText(endDate);
                            writeContents += "," + endDate;
                        }
                    }
                    break;
                }
                case CARD_TYPE_BANK: {
                    // 银行卡
                    ui.stackedWidget->setCurrentIndex(2);
                    ui.lineEditBankNum->setText("");
                    ui.lineEditBankValid->setText("");
                    ui.lineEditBankType->setText("");
                    ui.lineEditBankName->setText("");

                    writeContents += ",银行卡";
                    QJsonObject data1 = json["result"].toObject();
                    if (!data1.isEmpty()) {
                        QString number = data1["bank_card_number"].toString();
                        ui.lineEditBankNum->setText(number);
                        writeContents += ",'" + number;
                        QString valid = data1["valid_date"].toString();
                        ui.lineEditBankValid->setText(valid);
                        writeContents += "," + valid;
                        int type = data1["bank_card_type"].toInt();
                        ui.lineEditBankType->setText(0 == type ? "不能识别" : (1 == type ? "借记卡" : "信用卡"));
                        writeContents += "," + (0 == type) ? "不能识别" : (1 == type ? "借记卡" : "信用卡");
                        QString name = data1["bank_name"].toString();
                        ui.lineEditBankName->setText(name);
                        writeContents += "," + name;
                    }
                    break;
                }
                case CARD_TYPE_HK: {
                    // 港澳通行证
                    ui.stackedWidget->setCurrentIndex(3);
                    ui.lineEditHKname->setText("");
                    ui.lineEditHKsex->setText("");
                    ui.lineEditHKbirth->setText("");
                    ui.lineEditHKnum->setText("");
                    ui.lineEditHKengName->setText("");
                    ui.lineEditHKAddress->setText("");
                    ui.lineEditHKvalid->setText("");

                    writeContents += ",港澳通行证";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["NameChn"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditHKname->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonEngName = data1["NameEng"].toObject();
                        if (!jsonEngName.isEmpty()) {
                            QString name = jsonEngName["words"].toString();
                            ui.lineEditHKengName->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonBirth = data1["Birthday"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditHKbirth->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonSex = data1["Sex"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditHKsex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonNum = data1["CardNum"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditHKnum->setText(num);
                            writeContents += ",'" + num;
                        }
                        QJsonObject jsonAddr = data1["Address"].toObject();
                        if (!jsonAddr.isEmpty()) {
                            QString addr = jsonAddr["words"].toString();
                            ui.lineEditHKAddress->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonValid = data1["ValidDate"].toObject();
                        if (!jsonValid.isEmpty()) {
                            QString valid = jsonValid["words"].toString();
                            ui.lineEditHKvalid->setText(valid);
                            writeContents += "," + valid;
                        }
                    }
                    break;
                }
                case CARD_TYPE_TW: {
                    // 台湾通行证
                    ui.stackedWidget->setCurrentIndex(3);
                    ui.lineEditHKname->setText("");
                    ui.lineEditHKsex->setText("");
                    ui.lineEditHKbirth->setText("");
                    ui.lineEditHKnum->setText("");
                    ui.lineEditHKengName->setText("");
                    ui.lineEditHKAddress->setText("");
                    ui.lineEditHKvalid->setText("");

                    writeContents += ",台湾通行证";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["NameChn"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditHKname->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonEngName = data1["NameEng"].toObject();
                        if (!jsonEngName.isEmpty()) {
                            QString name = jsonEngName["words"].toString();
                            ui.lineEditHKengName->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonSex = data1["Sex"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditHKsex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonBirth = data1["Birthday"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditHKbirth->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonAddr = data1["Address"].toObject();
                        if (!jsonAddr.isEmpty()) {
                            QString addr = jsonAddr["words"].toString();
                            ui.lineEditHKAddress->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonNum = data1["CardNum"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditHKnum->setText(num);
                            writeContents += ",'" + num;
                        }
                        QJsonObject jsonValid = data1["ValidDate"].toObject();
                        if (!jsonValid.isEmpty()) {
                            QString valid = jsonValid["words"].toString();
                            ui.lineEditHKvalid->setText(valid);
                            writeContents += "," + valid;
                        }
                    }
                    break;
                }
                case CARD_TYPE_HOUSEHOLD: {
                    // 户口本
                    ui.stackedWidget->setCurrentIndex(5);
                    ui.lineEditHouseHoldBirthPlace->setText("");
                    ui.lineEditHouseHoldBirthday->setText("");
                    ui.lineEditHouseHoldNum->setText("");
                    ui.lineEditHouseHoldName->setText("");
                    ui.lineEditHouseHoldNation->setText("");
                    ui.lineEditHouseHoldRelation->setText("");
                    ui.lineEditHouseHoldSex->setText("");

                    writeContents += ",户口本";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["Name"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditHouseHoldName->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonSex = data1["Sex"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditHouseHoldSex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonNation = data1["Nation"].toObject();
                        if (!jsonNation.isEmpty()) {
                            QString nation = jsonNation["words"].toString();
                            ui.lineEditHouseHoldNation->setText(nation);
                            writeContents += "," + nation;
                        }
                        QJsonObject jsonBirth = data1["Birthday"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditHouseHoldBirthday->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonBirthAddr = data1["BirthAddress"].toObject();
                        if (!jsonBirthAddr.isEmpty()) {
                            QString addr = jsonBirthAddr["words"].toString();
                            ui.lineEditHouseHoldBirthPlace->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonNum = data1["CardNo"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditHouseHoldNum->setText(num);
                            writeContents += ",'" + num;
                        }
                        QJsonObject jsonRelation = data1["Relationship"].toObject();
                        if (!jsonRelation.isEmpty()) {
                            QString relation = jsonRelation["words"].toString();
                            ui.lineEditHouseHoldRelation->setText(relation);
                            writeContents += "," + relation;
                        }

                    }
                    break;
                }
                case CARD_TYPE_PASSPORT: {
                    // 护照
                    ui.stackedWidget->setCurrentIndex(4);
                    ui.lineEditPassportNation->setText("");
                    ui.lineEditPassportName->setText("");
                    ui.lineEditPassportSex->setText("");
                    ui.lineEditPassportBirth->setText("");
                    ui.lineEditPassportAddress->setText("");
                    ui.lineEditPassportValid->setText("");
                    ui.lineEditPassportNum->setText("");
                    ui.lineEditPassportStartDate->setText("");
                    ui.lineEditPassportNamePinYin->setText("");
                    ui.lineEditPassportBirthPlace->setText("");

                    writeContents += ",护照";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["姓名"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditPassportName->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonNamePinYin = data1["姓名拼音"].toObject();
                        if (!jsonNamePinYin.isEmpty()) {
                            QString name = jsonNamePinYin["words"].toString();
                            ui.lineEditPassportNamePinYin->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonSex = data1["性别"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditPassportSex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonBirth = data1["生日"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditPassportBirth->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonBirthPlace = data1["出生地点"].toObject();
                        if (!jsonBirthPlace.isEmpty()) {
                            QString birthPlace = jsonBirthPlace["words"].toString();
                            ui.lineEditPassportBirthPlace->setText(birthPlace);
                            writeContents += "," + birthPlace;
                        }
                        QJsonObject jsonNation = data1["国家码"].toObject();
                        if (!jsonNation.isEmpty()) {
                            QString nation = jsonNation["words"].toString();
                            ui.lineEditPassportNation->setText(nation);
                            writeContents += "," + nation;
                        }
                        QJsonObject jsonAddress = data1["护照签发地点"].toObject();
                        if (!jsonAddress.isEmpty()) {
                            QString addr = jsonAddress["words"].toString();
                            ui.lineEditPassportAddress->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonStartDate = data1["签发日期"].toObject();
                        if (!jsonStartDate.isEmpty()) {
                            QString startDate = jsonStartDate["words"].toString();
                            ui.lineEditPassportStartDate->setText(startDate);
                            writeContents += "," + startDate;
                        }
                        QJsonObject jsonValid = data1["有效期至"].toObject();
                        if (!jsonValid.isEmpty()) {
                            QString valid = jsonValid["words"].toString();
                            ui.lineEditPassportValid->setText(valid);
                            writeContents += "," + valid;
                        }
                        QJsonObject jsonNum = data1["护照号码"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditPassportNum->setText(num);
                            writeContents += ",'" + num;
                        }
                    }
                    break;
                }
                case CARD_TYPE_DRIVERLIC: {
                    // 驾驶证
                    ui.stackedWidget->setCurrentIndex(6);
                    ui.lineEditDriverNum->setText("");
                    ui.lineEditDriverValid->setText("");
                    ui.lineEditDriverType->setText("");
                    ui.lineEditDriverStartDate->setText("");
                    ui.lineEditDriverAddr->setText("");
                    ui.lineEditDriverName->setText("");
                    ui.lineEditDriverNation->setText("");
                    ui.lineEditDriverBirth->setText("");
                    ui.lineEditDriverSex->setText("");
                    ui.lineEditDriverFirstDate->setText("");

                    writeContents += ",驾驶证";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonName = data1["姓名"].toObject();
                        if (!jsonName.isEmpty()) {
                            QString name = jsonName["words"].toString();
                            ui.lineEditDriverName->setText(name);
                            writeContents += "," + name;
                        }
                        QJsonObject jsonSex = data1["性别"].toObject();
                        if (!jsonSex.isEmpty()) {
                            QString sex = jsonSex["words"].toString();
                            ui.lineEditDriverSex->setText(sex);
                            writeContents += "," + sex;
                        }
                        QJsonObject jsonBirth = data1["出生日期"].toObject();
                        if (!jsonBirth.isEmpty()) {
                            QString birth = jsonBirth["words"].toString();
                            ui.lineEditDriverBirth->setText(birth);
                            writeContents += "," + birth;
                        }
                        QJsonObject jsonNation = data1["国籍"].toObject();
                        if (!jsonNation.isEmpty()) {
                            QString nation = jsonNation["words"].toString();
                            ui.lineEditDriverNation->setText(nation);
                            writeContents += "," + nation;
                        }
                        QJsonObject jsonNum = data1["证号"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditDriverNum->setText(num);
                            writeContents += ",'" + num;
                        }
                        QJsonObject jsonAddress = data1["住址"].toObject();
                        if (!jsonAddress.isEmpty()) {
                            QString addr = jsonAddress["words"].toString();
                            ui.lineEditDriverAddr->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonValid = data1["有效期限"].toObject();
                        if (!jsonValid.isEmpty()) {
                            QString valid = jsonValid["words"].toString();
                            ui.lineEditDriverValid->setText(valid);
                            writeContents += "," + valid;
                        }
                        QJsonObject jsonType = data1["准驾车型"].toObject();
                        if (!jsonType.isEmpty()) {
                            QString type = jsonType["words"].toString();
                            ui.lineEditDriverType->setText(type);
                            writeContents += "," + type;
                        }
                        QJsonObject jsonStartDate = data1["有效起始日期"].toObject();
                        if (!jsonStartDate.isEmpty()) {
                            QString startDate = jsonStartDate["words"].toString();
                            ui.lineEditDriverStartDate->setText(startDate);
                            writeContents += "," + startDate;
                        }

                        QJsonObject jsonFirstDate = data1["初次领证日期"].toObject();
                        if (!jsonFirstDate.isEmpty()) {
                            QString firstDate = jsonFirstDate["words"].toString();
                            ui.lineEditDriverFirstDate->setText(firstDate);
                            writeContents += "," + firstDate;
                        }
                    }
                    break;
                }
                case CARD_TYPE_DRIVERLIC2: {
                    // 行驶证
                    ui.stackedWidget->setCurrentIndex(7);
                    ui.lineEditDriver2Type->setText("");
                    ui.lineEditDriver2StartDate->setText("");
                    ui.lineEditDriver2Purpose->setText("");
                    ui.lineEditDriver2Engine->setText("");
                    ui.lineEditDriver2num->setText("");
                    ui.lineEditDriver2Owner->setText("");
                    ui.lineEditDriver2Addr->setText("");
                    ui.lineEditDriver2RegisterDate->setText("");
                    ui.lineEditDriver2CarNum->setText("");
                    ui.lineEditDriverCarType->setText("");

                    writeContents += ",行驶证";
                    QJsonObject data1 = json["words_result"].toObject();
                    if (!data1.isEmpty()) {
                        QJsonObject jsonType = data1["品牌型号"].toObject();
                        if (!jsonType.isEmpty()) {
                            QString type = jsonType["words"].toString();
                            ui.lineEditDriver2Type->setText(type);
                            writeContents += "," + type;
                        }
                        QJsonObject jsonStartDate = data1["发证日期"].toObject();
                        if (!jsonStartDate.isEmpty()) {
                            QString startDate = jsonStartDate["words"].toString();
                            ui.lineEditDriver2StartDate->setText(startDate);
                            writeContents += "," + startDate;
                        }
                        QJsonObject jsonPurpose = data1["使用性质"].toObject();
                        if (!jsonPurpose.isEmpty()) {
                            QString purpose = jsonPurpose["words"].toString();
                            ui.lineEditDriver2Purpose->setText(purpose);
                            writeContents += "," + purpose;
                        }
                        QJsonObject jsonEngine = data1["发动机号码"].toObject();
                        if (!jsonEngine.isEmpty()) {
                            QString engine = jsonEngine["words"].toString();
                            ui.lineEditDriver2Engine->setText(engine);
                            writeContents += "," + engine;
                        }
                        QJsonObject jsonNum = data1["号牌号码"].toObject();
                        if (!jsonNum.isEmpty()) {
                            QString num = jsonNum["words"].toString();
                            ui.lineEditDriver2num->setText(num);
                            writeContents += "," + num;
                        }
                        QJsonObject jsonOwner = data1["所有人"].toObject();
                        if (!jsonOwner.isEmpty()) {
                            QString owner = jsonOwner["words"].toString();
                            ui.lineEditDriver2Owner->setText(owner);
                            writeContents += "," + owner;
                        }
                        QJsonObject jsonAddr = data1["住址"].toObject();
                        if (!jsonAddr.isEmpty()) {
                            QString addr = jsonAddr["words"].toString();
                            ui.lineEditDriver2Addr->setText(addr);
                            writeContents += "," + addr;
                        }
                        QJsonObject jsonRegistDate = data1["注册日期"].toObject();
                        if (!jsonRegistDate.isEmpty()) {
                            QString registDate = jsonRegistDate["words"].toString();
                            ui.lineEditDriver2RegisterDate->setText(registDate);
                            writeContents += "," + registDate;
                        }
                        QJsonObject jsonCarNum = data1["车辆识别代号"].toObject();
                        if (!jsonCarNum.isEmpty()) {
                            QString num = jsonCarNum["words"].toString();
                            ui.lineEditDriver2CarNum->setText(num);
                            writeContents += "," + num;
                        }
                        QJsonObject jsonCarType = data1["车辆类型"].toObject();
                        if (!jsonCarType.isEmpty()) {
                            QString carType = jsonCarType["words"].toString();
                            ui.lineEditDriverCarType->setText(carType);
                            writeContents += "," + carType;
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
            
            // 写文件
            QFile file("data.csv");
            if (file.open(QIODevice::Append)) {
                file.write(writeContents.toLocal8Bit());
                file.write("\r\n", 2);
                file.close();
            }
        }
        else {
            QMessageBox::information(nullptr, "警告", "网络连接失败");
        }
    } while (0);
}