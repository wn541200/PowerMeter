#ifndef POWERMETERDEVICE_H
#define POWERMETERDEVICE_H

#include <QObject>
#include <QTimer>
#include <QMutex>

class ICommunicationInterface;
class Protocol;

enum DeviceErrorCode {
    NoError = 0,                 // 无错误
    BusError = -1,               // 总线错误
    NotSupport = -2,             // 不支持该操作
    IncorrectResponse = -3,       // 设备回应不正确的指令
    Timeout = -4,                 // 通信超时
    CRCError = -5,                // CRC校验错误
    IncompleteData = -6,          // 接收到数据不完整还需要继续接收
    InvalidArgument = -7,         // 传递的参数无效
    BusNotOpened = -8,            // 通信端口还未打开
    DeviceNotConnected = -9,      // 设备未连接
    InUpgrading = -10,               // 正在升级固件
    DeviceBusy = -11,           // 设备忙碌，需再次尝试
    OtherError = -12               // 其他错误
};

class PowerMeterDevice: public QObject
{
    Q_OBJECT
public:
    enum BusType {
        SerialBus,
        CANBus,
        VirtualBus
    };

    static PowerMeterDevice* getInstance();
    bool comPortOpen(BusType type, const QString &portName, int baudRate=0, const QString &vendorName="none");
    bool comPortClose();
    bool isComPortOpened();
    bool isBMSDeviceConnected();
    bool pingDevice();

    enum DeviceErrorCode getRealTimeCurr(float &current);

signals:
    void deviceConnected(bool isConnected);
    void busErrorOccurred(const QString &errorMessage);

private:
    explicit PowerMeterDevice(QObject *parent = nullptr);


private:
    static PowerMeterDevice* instance;
    ICommunicationInterface *communicationInterface;
    Protocol *protocol;
    bool isDeviceConnected;
    bool isBusOpened;
    bool isDeviceUpgrading;
    QMutex mutex;
};

#endif // POWERMETERDEVICE_H
