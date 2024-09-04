#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChart>
#include <QChartView>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDeviceConnected(bool isConnected);
    void onReadCurrentData();
    void onUnitChanged();
    void onCalculateSelectAreaAverageDone(double average);

    void on_openComPortPushButton_clicked();

    void on_startPushButton_clicked();

    void on_timeUnitComboBox_currentIndexChanged(const QString &arg1);

    void on_timeUnitPerTickComboBox_currentIndexChanged(const QString &arg1);

    void on_timeTickComboBox_currentIndexChanged(const QString &arg1);

    void on_currentUnitComboBox_currentIndexChanged(const QString &arg1);

    void on_currentUnitPerTickComboBox_currentIndexChanged(const QString &arg1);

    void on_currentTickComboBox_currentIndexChanged(const QString &arg1);

    void on_savePushButton_clicked();

    void on_loadPushButton_clicked();


    void onUpdateCurrentCurve(float current);

signals:
    void updateCurrentCurve(float current);

private:
    Ui::MainWindow *ui;
    QTimer *m_timer;
    QList<float> currentRecordList;
    QChart *m_chart;
    QLineSeries *m_currentSeries;
    QValueAxis *axisX;
    QValueAxis *currentAxisY;
};
#endif // MAINWINDOW_H
