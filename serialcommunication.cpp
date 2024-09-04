#include "serialcommunication.h"
#include <QDebug>
#include <QSerialPort>
#include <QDataStream>

SerialCommunication::SerialCommunication(QObject *parent): ICommunicationInterface(parent), m_serialPort(nullptr)
{
    m_serialPort = new QSerialPort();

    m_serialPort->setDataBits(QSerialPort::Data8); // 设置数据位数
    m_serialPort->setParity(QSerialPort::NoParity); // 设置校验位
    m_serialPort->setStopBits(QSerialPort::OneStop); // 设置停止位


    connect(m_serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
            this, SLOT(handleSerialError(QSerialPort::SerialPortError)));
}

SerialCommunication::~SerialCommunication()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        delete m_serialPort;
    }
}

bool SerialCommunication::open(const QString &portName, int baudRate, const QString &vendorName)
{
    Q_UNUSED(vendorName);

    if (m_serialPort->isOpen()) {
        return false; // Serial port is already open
    }

    m_serialPort->setPortName(portName); // 设置串口名称
    m_serialPort->setBaudRate(baudRate); // 设置波特率

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port:" << portName;
        return false;
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunication::readyRead);
    return true;
}

bool SerialCommunication::close()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        disconnect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunication::readyRead);
        m_serialPort->close();
        return true;
    }
    return false;
}

bool SerialCommunication::sendData(const QByteArray &data)
{
    if (m_serialPort && m_serialPort->isOpen()) {
        qint64 bytesWritten = m_serialPort->write(data);
        return bytesWritten == data.size();
    }
    return false;
}

QByteArray SerialCommunication::receiveData()
{
    if (m_serialPort && m_serialPort->isOpen() && m_serialPort->bytesAvailable() > 0) {
        return m_serialPort->readAll();
    }
    return QByteArray();
}

void SerialCommunication::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qDebug() << "Serial port error occurred: " << m_serialPort->errorString();
        qDebug() << "error:" << error;

        if (error == QSerialPort::ResourceError) {
            emit errorOccurred("请检查串口是否被拔出！！！");
        }
    }
}
