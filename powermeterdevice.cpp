#include "powermeterdevice.h"
#include "serialcommunication.h"
#include "protocol.h"
#include <QDebug>

class ICommunicationInterface;

PowerMeterDevice* PowerMeterDevice::instance = nullptr;

PowerMeterDevice *PowerMeterDevice::getInstance()
{
    if (!instance)
    {
        instance = new PowerMeterDevice();
    }
    return instance;
}

bool PowerMeterDevice::comPortOpen(PowerMeterDevice::BusType type, const QString &portName, int baudRate, const QString &vendorName)
{


    if (instance->communicationInterface) {
        instance->communicationInterface->close();
        delete instance->communicationInterface;
        instance->communicationInterface = nullptr;
    }

    switch (type) {
    case SerialBus:
        instance->communicationInterface = new SerialCommunication();
        break;

    }

    isBusOpened = instance->communicationInterface->open(portName, baudRate, vendorName);
    connect(instance->communicationInterface, &ICommunicationInterface::errorOccurred,
            this, &PowerMeterDevice::busErrorOccurred);
    return isBusOpened;
}

bool PowerMeterDevice::comPortClose()
{


    if (instance->communicationInterface) {
        disconnect(instance->communicationInterface, &ICommunicationInterface::errorOccurred,
                   this, &PowerMeterDevice::busErrorOccurred);
        instance->communicationInterface->close();
        delete instance->communicationInterface;
        instance->communicationInterface = nullptr;
    }

    if (isDeviceConnected == true) {
        emit deviceConnected(false);
    }

    isDeviceConnected = false;
    isBusOpened = false;
    isDeviceUpgrading = false;

    return true;
}

bool PowerMeterDevice::isComPortOpened()
{
    return isBusOpened;
}

bool PowerMeterDevice::isBMSDeviceConnected()
{
    return isDeviceConnected;
}

bool PowerMeterDevice::pingDevice()
{
    if (isBusOpened == false) {
        qDebug() << "DeviceErrorCode::BusNotOpened";
        return  false;
    }

    if (DeviceErrorCode::NoError == protocol->pingDevice(communicationInterface)) {
        isDeviceConnected = true;
    }
    else {
        isDeviceConnected = false;
    }

    emit deviceConnected(isDeviceConnected);

    return  isDeviceConnected;
}

DeviceErrorCode PowerMeterDevice::getRealTimeCurr(float &current)
{
    if (isBusOpened == false) {
        return  DeviceErrorCode::BusNotOpened;
    }

    if (isDeviceConnected == false) {
        return  DeviceErrorCode::DeviceNotConnected;
    }
    if (isDeviceUpgrading) {
        return DeviceErrorCode::InUpgrading;
    }

    enum DeviceErrorCode ret = DeviceErrorCode::NoError;
    if (mutex.try_lock())
    {
        ret = protocol->getRealTimeCurr(communicationInterface, current);
        mutex.unlock();
    }
    else
    {
        return DeviceErrorCode::DeviceBusy;
    }

    return ret;
}

PowerMeterDevice::PowerMeterDevice(QObject *parent): QObject(parent),communicationInterface(nullptr)
{
    protocol = new Protocol(this);
    isDeviceConnected = false;
    isBusOpened = false;
    isDeviceUpgrading = false;
}
