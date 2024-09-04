#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include "powermeterdevice.h"
#include "customchartview.h"
#include "dataloader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    // 遍历串口信息列表，将串口名字添加到ComboBox中
    for(const QSerialPortInfo &info : portList) {
        ui->serialPortsComboBox->addItem(info.portName());
    }

    // 创建定时器
    m_timer = new QTimer();
    //m_timer->setSingleShot(true);

    // 连接定时器的timeout信号到槽函数，用于处理定时器触发时的操作
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onReadCurrentData()));

    PowerMeterDevice *device = PowerMeterDevice::getInstance();
    if (device) {
        connect(device, &PowerMeterDevice::deviceConnected, this, &MainWindow::onDeviceConnected);
    }

    m_chart = new QChart();
    m_currentSeries = new QLineSeries();
    m_chart->addSeries(m_currentSeries);

    currentAxisY = new QValueAxis;
    currentAxisY->setLabelFormat("%i mA");
    currentAxisY->setRange(0,60);

    // 将轴添加到图表中
    axisX = new QValueAxis;
    axisX->setRange(0,50);

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(currentAxisY, Qt::AlignRight);

    m_currentSeries->attachAxis(axisX);
    m_currentSeries->attachAxis(currentAxisY);

    m_currentSeries->setName("电流");
    //ui->chartView->setChart(m_chart);

    CustomChartView *chartView = new CustomChartView(m_chart, ui->chartWidget);
    chartView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *layout = new QVBoxLayout(ui->chartWidget);
    layout->addWidget(chartView);

    connect(chartView, &CustomChartView::calculateAverageDone, this, &MainWindow::onCalculateSelectAreaAverageDone);

    connect(this, &MainWindow::updateCurrentCurve, this, &MainWindow::onUpdateCurrentCurve);

    onUnitChanged();

}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow" ;
    m_timer->stop();
    delete  m_timer;
    delete ui;

    qDebug() << "~MainWindow" ;
}


void MainWindow::onDeviceConnected(bool isConnected)
{

    if (isConnected) {
        ui->startPushButton->setEnabled(true);
    } else {
        ui->startPushButton->setText("Start");
        m_timer->stop();
        ui->startPushButton->setEnabled(false);
    }
}

void MainWindow::on_openComPortPushButton_clicked()
{
    PowerMeterDevice *device = PowerMeterDevice::getInstance();

    if (device) {
        bool openOK = false;

        if (ui->openComPortPushButton->text() == "打开通信端口") {

                QString portName = ui->serialPortsComboBox->currentText();
                int baurdRate = ui->serialBaurdRateComboBox->currentText().toInt();
                openOK = device->comPortOpen(PowerMeterDevice::SerialBus, portName, baurdRate);


            if (openOK) {
                ui->openComPortPushButton->setText("关闭通信端口");
                device->pingDevice();

                // connect(device, &PowerMeterDevice::busErrorOccurred, this, &serialsetting::handleSerialError);
            }
            else {
                QMessageBox::information(nullptr, "提示", "通信端口打开失败 ");

                qDebug() << "通信端口打开失败 !" ;
            }
        }
        else if (ui->openComPortPushButton->text() == "关闭通信端口") {
            //disconnect(device, &PowerMeterDevice::busErrorOccurred, this, &serialsetting::handleSerialError);
            device->comPortClose();
            ui->openComPortPushButton->setText("打开通信端口");
            //checkPortsAvailableTimer.start(1000);
        }


    }
}

void MainWindow::onUpdateCurrentCurve(float current)
{
    currentRecordList.append(current);

    if (!currentRecordList.isEmpty()) {
        float sum = 0.0;
            for (float value : currentRecordList) {
                sum += value;
            }

            float average = sum / currentRecordList.size();
            QString averageString = QString::number(average, 'f', 3);
            ui->avgCurrentLabel->setText(averageString);

            QString unit = ui->timeUnitComboBox->currentText();
            if (unit == "秒") {
                m_currentSeries->append((currentRecordList.size()-1) * 0.5, current);
            } else if (unit == "分") {
                m_currentSeries->append((currentRecordList.size()-1) * 0.5 / 60.0, current);
            } else if (unit == "时") {
                m_currentSeries->append((currentRecordList.size()-1) * 0.5 / 3600.0, current);
            }
    }

}

void MainWindow::onReadCurrentData()
{
    PowerMeterDevice *device = PowerMeterDevice::getInstance();

    if (device) {
        float current;
        device->getRealTimeCurr(current);

        emit updateCurrentCurve(current);

    }
}

void MainWindow::onUnitChanged()
{
    static QString lastUnit;
    // 设置X轴标题
    QString unit = ui->timeUnitComboBox->currentText();
    if (unit == "秒") {
        axisX->setTitleText("时间 (秒)");
    } else if (unit == "分") {
        axisX->setTitleText("时间 (分钟)");
    } else if (unit == "时") {
        axisX->setTitleText("时间 (小时)");
    }

    qDebug() << "onUnitChanged repaint start";
    if (unit != lastUnit) {
        m_currentSeries->clear();
        int index = 0;
        for (float current : currentRecordList) {
            if (unit == "秒") {
                m_currentSeries->append(index * 0.5, current);
            } else if (unit == "分") {
                m_currentSeries->append(index * 0.5 / 60.0, current);
            } else if (unit == "时") {
                m_currentSeries->append(index * 0.5 / 3600.0, current);
            }
            QThread::usleep(10);
            index++;
        }
    }

    qDebug() << "onUnitChanged repaint end";

    lastUnit = unit;


    int xTick = ui->timeTickComboBox->currentText().toInt() + 1;
    int xRange = (xTick-1) * ui->timeUnitPerTickComboBox->currentText().toInt();
    axisX->setRange(0, xRange);
    axisX->setTickCount(xTick);

    int yTick = ui->currentTickComboBox->currentText().toInt() + 1;
    int yRange = (yTick-1) * ui->currentUnitPerTickComboBox->currentText().toInt();
    currentAxisY->setRange(0, yRange);
    currentAxisY->setTickCount(yTick);

}

void MainWindow::onCalculateSelectAreaAverageDone(double average)
{
    QString averageString = QString::number(average, 'f', 3);
    ui->selectAreaAvgCurrentLabel->setText(averageString);
}

void MainWindow::on_startPushButton_clicked()
{
    if (ui->startPushButton->text() == "Start") {
        ui->startPushButton->setText("Stop");
        currentRecordList.clear();
        m_currentSeries->clear();
        m_currentSeries->setName("记录实时电流");
        m_timer->start(500);

    } else if (ui->startPushButton->text() == "Stop") {
        ui->startPushButton->setText("Start");
        m_currentSeries->setName("电流记录已停止");
        m_timer->stop();
    }

}


void MainWindow::on_timeUnitComboBox_currentIndexChanged(const QString &arg1)
{
    //onUnitChanged();
    QtConcurrent::run(this, &MainWindow::onUnitChanged);
}


void MainWindow::on_timeUnitPerTickComboBox_currentIndexChanged(const QString &arg1)
{
    onUnitChanged();
}


void MainWindow::on_timeTickComboBox_currentIndexChanged(const QString &arg1)
{
    onUnitChanged();
}


void MainWindow::on_currentUnitComboBox_currentIndexChanged(const QString &arg1)
{
    onUnitChanged();
}


void MainWindow::on_currentUnitPerTickComboBox_currentIndexChanged(const QString &arg1)
{
    onUnitChanged();
}


void MainWindow::on_currentTickComboBox_currentIndexChanged(const QString &arg1)
{
    onUnitChanged();
}


void MainWindow::on_savePushButton_clicked()
{
    if (ui->startPushButton->text() == "Stop") {
        on_startPushButton_clicked();
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Save CSV", "", "CSV Files (*.csv)");
    if (!filePath.isEmpty()) {

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (float value : currentRecordList) {
                out << QString::number(value) << "\n";
            }
            file.close();
            qDebug() << "Data saved to" << filePath;
        } else {
            qDebug() << "Failed to open file" << filePath;
        }
    }
}


void MainWindow::on_loadPushButton_clicked()
{
    if (ui->startPushButton->text() == "Stop") {
        on_startPushButton_clicked();
    }

    QString filePath = QFileDialog::getOpenFileName(this, "Open CSV", "", "CSV Files (*.csv)");
    if (!filePath.isEmpty()) {
        m_currentSeries->clear();
        DataLoader *loader = new DataLoader(filePath, this);
        connect(loader, &DataLoader::dataLoaded, this, &MainWindow::onUpdateCurrentCurve);
        loader->start();

    }

}

