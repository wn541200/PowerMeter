// serialcommunication.h
#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include "icommunicationinterface.h"

#include <QSerialPort>

class SerialCommunication : public ICommunicationInterface
{
    Q_OBJECT

public:
    explicit SerialCommunication(QObject *parent = nullptr);
    ~SerialCommunication() override;

    bool open(const QString &portName, int baudRate=0, const QString &vendorName="none") override;
    bool close() override;
    bool sendData(const QByteArray &data) override;
    QByteArray receiveData() override;


private:
    QSerialPort *m_serialPort;

public slots:
    void handleSerialError(QSerialPort::SerialPortError error);

};

#endif // SERIALCOMMUNICATION_H
