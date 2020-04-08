#ifndef MEANSCLUSTERWIDGET_H
#define MEANSCLUSTERWIDGET_H

#include <QWidget>
#include <QPointF>

#include <QTimer>

#include "MeansCluster.h"

class MeansClusterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MeansClusterWidget(QWidget* parent = nullptr);

    void setPoints(const QVector<QPointF>& points);

    void paintEvent(QPaintEvent* event) override;

private:
    QVector<QColor> generateColors(int numberOfColors);

    QPointF interpolate(const QPointF& start, const QPointF& finish, qreal time);

    QVector<QPointF> _dataPoints;
    MeansCluster<QPointF> _cluster;

    QMap<unsigned int, QVector<unsigned int>> _classifications;
    QVector<QPointF> _centroids;

    float _xMin;
    float _xMax;

    float _yMin;
    float _yMax;

    QTransform _painterTransform;
    QTimer _updateTimer;
    
    QVector<QPointF> _previousCentroids;
    bool _isAnimating;
    int _frame;
};

#endif // MEANSCLUSTERWIDGET_H
