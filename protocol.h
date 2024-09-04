#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <icommunicationinterface.h>
#include <QStringList>
#include "powermeterdevice.h"


enum class CommandCode
{
    ReadRegs = 3,       // 读取仪表寄存器内容
};

class Protocol: public QObject
{
    Q_OBJECT
public:
    Protocol(QObject *parent = nullptr);

    static unsigned short calculateCRC16(const QByteArray &data);
    enum DeviceErrorCode pingDevice(ICommunicationInterface *comPort);
    enum DeviceErrorCode getDeviceModel(ICommunicationInterface *comPort, QString &model);
    enum DeviceErrorCode getRealTimeCurr(ICommunicationInterface *comPort, float &current);

private:
    QByteArray generateReadRequestFrame(CommandCode command, uint16_t startAddr, uint16_t len, QByteArray data);
    enum DeviceErrorCode sendAndWaitResponse(ICommunicationInterface* port, const QByteArray& request, QByteArray& outdata);
    enum DeviceErrorCode parseReceivedData(QByteArray& receivedData, QByteArray& outdata, const QByteArray& request, bool& receivedRequestedData);

};

#endif // PROTOCOL_H
