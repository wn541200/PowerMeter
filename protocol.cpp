#include "protocol.h"
#include <QDebug>
#include <QDataStream>
#include <QEventLoop>
#include <QTimer>
#include <QtEndian>
#include <QDateTime>
#include <QBitmap>


#define RESPONDTIMEOUT 2000

Protocol::Protocol(QObject *parent): QObject(parent)
{

}

static unsigned char auchCRCHi[] = {0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
0x41,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00,
0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80,
0x41, 0x00, 0xC1, 0x81, 0x40,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00,
0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40} ;
/* CRC 低位字节值表*/
static unsigned char auchCRCLo[] = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,0xC6, 0x06, 0x07, 0xC7, 0x05,
0xC5, 0xC4, 0x04,0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA,
0x1A,0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17,
0x16, 0xD6,0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,0xF0, 0x30, 0x31, 0xF1, 0x33,
0xF3, 0xF2, 0x32,0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,0x3C, 0xFC, 0xFD, 0x3D,
0xFF, 0x3F, 0x3E, 0xFE,0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC,
0x2C,0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21,
0x20, 0xE0,0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,0x66, 0xA6, 0xA7, 0x67, 0xA5,
0x65, 0x64, 0xA4,0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,0xAA, 0x6A, 0x6B, 0xAB,
0x69, 0xA9, 0xA8, 0x68,0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,0xBE, 0x7E, 0x7F,
0xBF, 0x7D, 0xBD, 0xBC, 0x7C,0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,0x72, 0xB2,
0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,0x96,
0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E,
0x5E,0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A,0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40} ;

unsigned short Protocol::calculateCRC16(const QByteArray &data)
{
    unsigned char uchCRCHi = 0xFF ; /* 高CRC 字节初始化*/
    unsigned char uchCRCLo = 0xFF ; /* 低CRC 字节初始化*/
    unsigned uIndex;

    for (int byteIdx = 0; byteIdx < data.size(); byteIdx++)
    {
    uIndex = uchCRCHi ^ static_cast<unsigned char>(data.at(byteIdx)) ; /* 计算CRC */
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
    uchCRCLo = auchCRCLo[uIndex] ;
    }
    return (uchCRCHi << 8 | uchCRCLo) ;

}


DeviceErrorCode Protocol::pingDevice(ICommunicationInterface *comPort)
{
    QString model;
    DeviceErrorCode ret= DeviceErrorCode::OtherError;

    ret = getDeviceModel(comPort, model);
    if (ret != DeviceErrorCode::NoError) {
        qDebug() << "pingDevice 出错，ErrorCode:" << ret;
    }

    return  ret;
}

QByteArray Protocol::generateReadRequestFrame(CommandCode command, uint16_t startAddr, uint16_t len, QByteArray data)
{
    QByteArray requestFrame;

    // 添加仪表地址
    requestFrame.append(0x01);

    // 添加指令码、起始地址、长度
    requestFrame.append(static_cast<uint8_t>(command));
    requestFrame.append(static_cast<uint8_t>(startAddr >> 8));
    requestFrame.append(static_cast<uint8_t>(startAddr & 0xff));
    requestFrame.append(static_cast<uint8_t>(len >> 8));
    requestFrame.append(static_cast<uint8_t>(len & 0xff));

    if (data.size() > 0) {
        requestFrame.append(data);
    }

    // 计算并添加校验，不包括帧头
    unsigned short crc = calculateCRC16(requestFrame);
    requestFrame.append(static_cast<uint8_t>(crc >> 8));
    requestFrame.append(static_cast<uint8_t>(crc & 0xff));

    // qDebug() << "requestFrame:" << requestFrame.toHex();

    return requestFrame;
}

DeviceErrorCode Protocol::getDeviceModel(ICommunicationInterface *comPort, QString &model)
{
    QByteArray outdata;
    DeviceErrorCode ret= DeviceErrorCode::OtherError;

    uint16_t reg_addr = 0x2;
    uint16_t len = 0x3;

    QByteArray senddata = generateReadRequestFrame(CommandCode::ReadRegs, reg_addr, len, QByteArray());
    ret = sendAndWaitResponse(comPort, senddata, outdata);
    if (NoError == ret) {
        model = QString::fromLatin1(outdata.mid(3, outdata.size() - 3 - 2));
        qDebug() << "HardwareVersion:" << model;
    }

    return  ret;
}

DeviceErrorCode Protocol::getRealTimeCurr(ICommunicationInterface *comPort, float &current)
{
    QByteArray outdata;
    DeviceErrorCode ret= DeviceErrorCode::OtherError;

    uint16_t reg_addr = 0x102;
    uint16_t len = 0x2;

    QByteArray senddata = generateReadRequestFrame(CommandCode::ReadRegs, reg_addr, len, QByteArray());
    ret = sendAndWaitResponse(comPort, senddata, outdata);
    if (NoError == ret) {
        //model = QString::fromLatin1(outdata.mid(3, outdata.size() - 3 - 2));
        // 定义一个包含16进制数据的 QByteArray
        QByteArray byteArray;
        byteArray.append(outdata[6]);
        byteArray.append(outdata[5]);
        byteArray.append(outdata[4]);
        byteArray.append(outdata[3]);


        float floatValue = *reinterpret_cast<float*>(byteArray.data());

        current = floatValue * 1000;
        qDebug() << "current:" << current;
    }

    return  ret;
}

enum DeviceErrorCode Protocol::parseReceivedData(QByteArray& receivedData, QByteArray& outdata, const QByteArray& request, bool& receivedRequestedData) {

    enum DeviceErrorCode ret = DeviceErrorCode::NoError;

    // 数据长度至少有1字节设备地址，1字节功能码，2字节crc
    if (receivedData.length() < 5) {
        qDebug() << "DeviceErrorCode::IncompleteData" << receivedData.toHex().toUpper();
        return DeviceErrorCode::IncompleteData;
    }

    int startIndex = receivedData.indexOf(request.mid(0,2));
    if (startIndex == -1) {
        receivedData.clear(); // 清空缓冲区，未找到头
        return DeviceErrorCode::IncompleteData;
    }

    if (receivedData.size() < startIndex + 5) {
        return DeviceErrorCode::IncompleteData;
    }

    quint8 length = receivedData[startIndex + 2];
    int frameSize = 3 + length + 2; // 固定头(3) + 数据长度 + CRC校验(2)

    if (receivedData.size() < startIndex + frameSize) {
        return DeviceErrorCode::IncompleteData;
    }

    uint16_t calc_crc =calculateCRC16( receivedData.mid(0, frameSize - 2));
    //qDebug() << "xxx:" << calc_crc;

    uint16_t rec_crc = static_cast<uint8_t>(receivedData[frameSize-1]) | (static_cast<uint8_t>(receivedData[frameSize-2])<<8);
    //qDebug() << "yyy:" << rec_crc;

    if (rec_crc != calc_crc) {
        receivedData.remove(0, startIndex + frameSize); // 移除已处理的数据
        return DeviceErrorCode::CRCError;
    }

    QByteArray frame = receivedData.mid(startIndex, frameSize);
    receivedData.remove(0, startIndex + frameSize); // 移除已处理的数据

    outdata = frame;
    receivedRequestedData = true;
    //qDebug() << "receivedData:" << frame.toHex();

    return  DeviceErrorCode::NoError;
}

// 等待回应的函数
enum DeviceErrorCode Protocol::sendAndWaitResponse(ICommunicationInterface* port, const QByteArray& request, QByteArray& outdata) {
    static QByteArray receivedData;
    QEventLoop loop;

    //qDebug() << "sendAndWaitResponse";

    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    bool incorrectResponse = false;
    bool crcError = false;

    // 用于标识是否已收到所请求的数据
    bool receivedRequestedData = false;
    bool needWait = true;

    QMetaObject::Connection lambdaConnection;


        // 在 lambda 表达式中捕获请求数据和标志位
        lambdaConnection = connect(port, &ICommunicationInterface::readyRead, [&]() {
            QByteArray data = port->receiveData();
            receivedData.append(data);

            DeviceErrorCode parseResult = parseReceivedData(receivedData, outdata, request, receivedRequestedData);

            if (parseResult == DeviceErrorCode::IncorrectResponse) {
                incorrectResponse = true;
                needWait = false;
                loop.quit();
            } else if (parseResult == DeviceErrorCode::CRCError) {
                crcError = true;
                needWait = false;
                loop.quit();
            } else if (receivedRequestedData) {
                //qDebug() << "Receive a frame:" << outdata.toHex();
                needWait = false;
                loop.quit();
            }
        });

        port->sendData(request);



    timer.start(RESPONDTIMEOUT);
    if (needWait) {
        loop.exec();
    }

    if (!timer.isActive()) {
        qDebug() << "waitResponse timeout";
        disconnect(lambdaConnection);
        return DeviceErrorCode::Timeout;
    }

    if (incorrectResponse) {
        qDebug() << "Incorrect response received";
        disconnect(lambdaConnection);
        return DeviceErrorCode::IncorrectResponse;
    }

    if (crcError) {
        qDebug() << "CRC error received";
        disconnect(lambdaConnection);
        return DeviceErrorCode::CRCError;
    }

    disconnect(lambdaConnection);
    return DeviceErrorCode::NoError;
}


