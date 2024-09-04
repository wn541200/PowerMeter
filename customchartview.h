#ifndef CUSTOMCHARTVIEW_H
#define CUSTOMCHARTVIEW_H


#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QtCharts>
#include <QGraphicsRectItem>
#include <QMouseEvent>

using namespace QtCharts;

class CustomChartView : public QChartView
{
    Q_OBJECT

public:
    explicit CustomChartView(QWidget *parent = nullptr)
    {
        CustomChartView(nullptr, parent);
    }

    explicit CustomChartView(QChart *chart = nullptr, QWidget *parent = nullptr) : QChartView(chart, parent), selecting(false) {
        rubberBand = new QGraphicsRectItem();
        rubberBand->setBrush(QColor(0, 0, 255, 50)); // 半透明蓝色
        chart->scene()->addItem(rubberBand);
        rubberBand->hide();
    }

signals:
    void calculateAverageDone(double average);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            if (rubberBand->isVisible()) {
                 rubberBand->hide();
             } else {
                 selecting = true;
                 origin = event->pos();
                 rubberBand->setRect(QRectF( QPoint(origin.x(), 0), QSize()));
                 rubberBand->show();
             }
        }
        QChartView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (selecting) {
            QPointF current = event->pos();
            QRectF rect = QRectF(QRectF( QPoint(origin.x(), 0), QPoint(current.x(), this->height()))).normalized();

            QValueAxis *axisY = qobject_cast<QValueAxis *>(chart()->axisY());
            if (axisY) {
                QPointF topLeft = chart()->mapToPosition(QPointF(0, axisY->max()));
                QPointF bottomRight = chart()->mapToPosition(QPointF(0, axisY->min()));
                rect.setTop(topLeft.y());
                rect.setBottom(bottomRight.y());
            }
            rubberBand->setRect(rect);
        }
        QChartView::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            selecting = false;
            calculateAverage(rubberBand->rect());
        }
        QChartView::mouseReleaseEvent(event);
    }

private:
    void calculateAverage(const QRectF &rect) {
        QChart *chart = this->chart();
        QList<QAbstractSeries *> seriesList = chart->series();
        if (seriesList.isEmpty()) return;

        QLineSeries *series = qobject_cast<QLineSeries *>(seriesList.first());
        if (!series) return;

        QValueAxis *axisX = qobject_cast<QValueAxis *>(chart->axisX());
        QValueAxis *axisY = qobject_cast<QValueAxis *>(chart->axisY());
        if (!axisX || !axisY) return;

        QPointF topLeft = chart->mapToValue(rect.topLeft());
        QPointF bottomRight = chart->mapToValue(rect.bottomRight());

        double xMin = qMin(topLeft.x(), bottomRight.x());
        double xMax = qMax(topLeft.x(), bottomRight.x());

        double sum = 0.0;
        int count = 0;
        for (const QPointF &point : series->points()) {
            if (point.x() >= xMin && point.x() <= xMax) {
                sum += point.y();
                count++;
            }
        }

        double average = count > 0 ? sum / count : 0;
        emit calculateAverageDone(average);
        qDebug() << "Selected region average current:" << average;
    }

    bool selecting;
    QPoint origin;
    QGraphicsRectItem *rubberBand;
};


#endif // CUSTOMCHARTVIEW_H
