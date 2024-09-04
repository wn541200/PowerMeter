// icommunicationinterface.h
#ifndef ICOMMUNICATIONINTERFACE_H
#define ICOMMUNICATIONINTERFACE_H

#include <QObject>

class ICommunicationInterface : public QObject
{
    Q_OBJECT

public:
    explicit ICommunicationInterface(QObject *parent = nullptr);
    virtual ~ICommunicationInterface();

    // 公共方法
    virtual bool open(const QString &portName, int baudRate=0, const QString &vendorName="none") = 0;
    virtual bool close() = 0;
    virtual bool sendData(const QByteArray &data) = 0;
    virtual QByteArray receiveData() = 0;

signals:
    void readyRead();
    void errorOccurred(const QString &errorMessage);
};

#endif // ICOMMUNICATIONINTERFACE_H
