#include "MeansClusterWidget.h"

#include <QPainter>

#include <math.h>

MeansClusterWidget::MeansClusterWidget
(
    QWidget* parent
):
    QWidget(parent),
    _isAnimating(false),
    _frame(0)
{
    _cluster.setDistanceFunction([](const QPointF& first, const QPointF& second)
    {
        float xDifference = first.x() - second.x();
        float yDifference = first.y() - second.y();
        return xDifference * xDifference + yDifference * yDifference;
    });

    
    _updateTimer.setInterval(1000 / 60);
    connect(&_updateTimer, &QTimer::timeout, [this]() 
    {
        if (_isAnimating)
        {
            _frame++;
            if (_frame > 60)
            {
                _isAnimating = false;
                _frame = 0;
            }
        }
        else
        {
            _previousCentroids = _centroids;
            _classifications = _cluster.learnCoroutinized(_centroids);
            if (_previousCentroids.count() == _centroids.count())
            {
                _isAnimating = true;
            }
        }
        update();
    });

    _updateTimer.start();
}

void MeansClusterWidget::setPoints
(
    const QVector<QPointF>& points
)
{
    _dataPoints = points;
    _cluster.setPoints(points);

    _xMin = std::numeric_limits<qreal>::max();
    _yMin = _xMin;

    _xMax = std::numeric_limits<qreal>::min();
    _yMax = _xMax;

    for (const auto& point : points)
    {
        if (point.x() > _xMax)
        {
            _xMax = point.x();
        }

        if (point.x() < _xMin)
        {
            _xMin = point.x();
        }

        if (point.y() > _yMax)
        {
            _yMax = point.y();
        }

        if (point.y() < _yMin)
        {
            _yMin = point.y();
        }
    }

    _classifications = _cluster.learnCoroutinized(_centroids);
}


void MeansClusterWidget::paintEvent
(
    QPaintEvent* event
)
{
    Q_UNUSED(event)

    QPainter painter;
    painter.begin(this);
    
    float xRange = _xMax - _xMin;
    float yRange = _yMax - _yMin;

    QTransform reflectTransform;
    reflectTransform.translate(0, height());
    reflectTransform.scale(1.0, -1.0);

    QTransform toOriginTransform;
    toOriginTransform.translate(-_xMin, -_yMin);

    QTransform scaleTransform;
    float max = xRange > yRange ? xRange : yRange;
    scaleTransform.scale(1.0 / xRange, 1.0 / yRange);
    
    float min = width() < height() ? width() : height();
    scaleTransform.scale(min, min);

    _painterTransform = toOriginTransform * scaleTransform * reflectTransform;
    painter.fillRect(_painterTransform.mapRect(QRectF(_xMin, _yMin, xRange, yRange)), Qt::black);

    painter.setPen(Qt::black);
    painter.drawEllipse(_painterTransform.map(QPointF(0.0, 0.0)), 3.0, 3.0);

    auto clusterColors = generateColors(_classifications.count());

    auto classification = _classifications.constKeyValueBegin();
    while (classification != _classifications.constKeyValueEnd())
    {
        unsigned int clusterIndex = (*classification).first;
        QPen pen = painter.pen();
        pen.setColor(clusterColors[clusterIndex]);
        painter.setPen(pen);
        painter.setBrush(QColor(0, 0, 0, 0));

        if (!_isAnimating)
        {
            painter.drawEllipse(_painterTransform.map(_centroids[clusterIndex]), 5, 5);
        }
        else
        {
            QPointF interpolatedPoint = interpolate(_previousCentroids[clusterIndex], _centroids[clusterIndex], (double)_frame / 60);
            painter.drawEllipse(_painterTransform.map(interpolatedPoint), 5, 5);
        }
        auto pointIter = (*classification).second.begin();
        while (pointIter != (*classification).second.end())
        {
            QPointF originalPoint = _dataPoints[*pointIter];
            QPointF mappedPoint = _painterTransform.map(originalPoint);

            painter.drawEllipse(mappedPoint, 3, 3);
            pointIter++;
        }

        classification++;
    }


    painter.end();
}

QVector<QColor> MeansClusterWidget::generateColors
(
    int numberOfColors
)
{
    //Works by traversing the color wheel in increments of 2pi/N starting at 0
    QVector<QColor> result;
    for (int i = 0; i < numberOfColors; ++i)
    {
        double hue = 255 * i / numberOfColors;
        result.append(QColor::fromHsl(hue, 126, 200));
    }
    return result;
}

QPointF MeansClusterWidget::interpolate
(
    const QPointF& start, 
    const QPointF& finish, 
    qreal time
)
{
    return (finish - start) * time + start;
}