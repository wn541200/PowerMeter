#ifndef DATALOADER_H
#define DATALOADER_H

#include <QThread>
#include <QFile>
#include <QTextStream>

class DataLoader : public QThread {
    Q_OBJECT

public:
    DataLoader(const QString& filePath, QObject* parent = nullptr)
        : QThread(parent), filePath(filePath) {}

signals:
    void dataLoaded(float value);
    void loadingFinished();

protected:
    void run() override {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                float value = line.toFloat();
                emit dataLoaded(value);
                QThread::usleep(10); // 让出cpu,给界面响应
            }
            file.close();
        }
        emit loadingFinished();
    }

private:
    QString filePath;
};
#endif // DATALOADER_H
