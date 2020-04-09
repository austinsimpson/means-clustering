#ifndef MEANSCLUSTER_H
#define MEANSCLUSTER_H

#include <functional>
#include <algorithm>

#include <QVector>

#include <QDebug>

const double kConvergenceThreshold = 0.00001;

template <typename T>
class MeansCluster
{
public:
    MeansCluster(): _numberOfClusters(1), _oldestError(-1.0), _middleError(-1.0), _currentError(-1.0), _shouldStop(false)
    {

    }

    void setPoints(const QVector<T>& points)
    {
        _dataPoints = points;
    }

    const QVector<T>& points() const
    {
        return _dataPoints;
    }

    void setDistanceFunction(std::function < qreal(const T&, const T&)> distanceFunction)
    {
        _distanceFunction = distanceFunction;
    }

    void learn()
    {
        _shouldStop = false;
        _currentError = -1.0;
        _middleError = -1.0;
        _oldestError = -1.0;
        while (_numberOfClusters <= _dataPoints.count() && !_shouldStop)
        {
            //First, we start off with an initial guess for the centroids, "random".
            QVector<T> previousCentroids = seedInitialCentroids();
            QVector<T> nextCentroids;
            QMap<unsigned int, QVector<unsigned int>> classifications;

            do
            {
                if (nextCentroids.count() > 0)
                {
                    previousCentroids = nextCentroids;
                }
                classifications = classifyByCentroids(previousCentroids);
                nextCentroids = findCentroids(classifications);
                
            } while (maxCentroidDifference(previousCentroids, nextCentroids) > kConvergenceThreshold);

            updateErrorMetrics(classifications, nextCentroids);
            _numberOfClusters++;
        }
    }


    QMap<unsigned int, QVector<unsigned int>> classifyByCentroids(const QVector<T>& centroids)
    {
        QMap<unsigned int, QVector<unsigned int>> classifications;
        for (unsigned int pointIndex = 0; pointIndex < _dataPoints.count(); ++pointIndex)
        {
            const T& point = _dataPoints[pointIndex];

            unsigned int minimalDistanceIndex = 0;
            qreal minDistance = std::numeric_limits<qreal>::max();

            unsigned int centroidIndex = 0;
            for (const T& centroid : centroids)
            {
                qreal distance = _distanceFunction(centroid, point);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    minimalDistanceIndex = centroidIndex;
                }
                centroidIndex++;
            }
            classifications[minimalDistanceIndex].push_back(pointIndex);
        }
        return classifications;
    }

    QVector<T> findCentroids(const QMap<unsigned int, QVector<unsigned int>>& classifications)
    {
        QVector<T> newCentroids;

        QMap<unsigned int, QVector<unsigned int>>::const_key_value_iterator category = classifications.constKeyValueBegin();
        while (category != classifications.constKeyValueEnd())
        {
            T centroid;

            QVector<unsigned int>::const_iterator pointIndex = (*category).second.cbegin();
            while (pointIndex != (*category).second.cend())
            {
                centroid += _dataPoints[*pointIndex];

                pointIndex++;
            }

            centroid /= (*category).second.count();
            newCentroids.push_back(centroid);

            category++;
        }

        return newCentroids;
    }

    //Allows us to animate whats happening
    QMap<unsigned int, QVector<unsigned int>> learnCoroutinized(QVector<T>& centroids)
    {
        if (_shouldStop)
            return classifyByCentroids(_previousCentroids);

        if (_previousCentroids.size() == 0)
        {
            _previousCentroids = seedInitialCentroids();
        }

        auto classifications = classifyByCentroids(_previousCentroids);
        centroids = findCentroids(classifications);
        if (maxCentroidDifference(centroids, _previousCentroids) < 0.00001)
        {
            _numberOfClusters++;
            updateErrorMetrics(classifications, centroids);
            if (!_shouldStop)
            {
                _previousCentroids.clear();
            }
        }
        else
        {
            _previousCentroids = centroids;
        }
        return classifications;
    }

    qreal findError(const QMap<unsigned int, QVector<unsigned int>>& classifications, const QVector<T>& centroids)
    {
        qreal result = 0.0;
        auto cluster = classifications.constKeyValueBegin();
        while (cluster != classifications.constKeyValueEnd())
        {
            const T& centroid = centroids[(*cluster).first];
            auto pointIndex = (*cluster).second.cbegin();
            while (pointIndex != (*cluster).second.cend())
            {
                result += _distanceFunction(centroid, _dataPoints[*pointIndex]);
                pointIndex++;
            }
            cluster++;
        }
        return result;
    }

private:
    QVector<T> seedInitialCentroids()
    {
        srand(time(nullptr));
        QVector<T> centroids;
        QVector<T> dataPoints = _dataPoints;

        if (dataPoints.count() > 0)
        {
            unsigned int potentialIndex = rand() % _dataPoints.count();
            centroids.push_back(_dataPoints[potentialIndex]);
            dataPoints.remove(potentialIndex);
        }
        while (centroids.count() < _numberOfClusters)
        {
            qreal previousMax = std::numeric_limits<qreal>::min();

            unsigned int maximalPointIndex;
            for (int pointIndex = 0; pointIndex < dataPoints.count(); ++pointIndex)
            {
                const auto& point = dataPoints[pointIndex];
                qreal cumulativeSquaredDistance = 0.0;
                for (const auto& centroid : centroids)
                {
                    cumulativeSquaredDistance += _distanceFunction(point, centroid);
                }
                if (previousMax < cumulativeSquaredDistance)
                {
                    previousMax = cumulativeSquaredDistance;
                    maximalPointIndex = pointIndex;
                }
            }
            centroids.push_back(dataPoints[maximalPointIndex]);
            dataPoints.remove(maximalPointIndex);
        }
        return centroids;
    }

    void debugClassifications(const QMap<unsigned int, QVector<unsigned int>>& classifications) const
    {
        QMap<unsigned int, QVector<unsigned int>>::const_key_value_iterator category = classifications.constKeyValueBegin();
        while (category != classifications.constKeyValueEnd())
        {
            QVector<unsigned int>::const_iterator categoryPointIndex = (*category).second.cbegin();
            while (categoryPointIndex != (*category).second.end())
            {
                categoryPointIndex++;
            }
            category++;
        }
    }

    qreal maxCentroidDifference(const QVector<T>& firstCentroids, const QVector<T>& secondCentroids) const
    {
        Q_ASSERT(firstCentroids.count() == secondCentroids.count());

        qreal result = std::numeric_limits<qreal>::min();

        for (int i = 0; i < firstCentroids.count(); ++i)
        {
            qreal distance = _distanceFunction(firstCentroids[i], secondCentroids[i]);
            if (distance > result)
            {
                result = distance;
            }
        }

        return result;
    }

    void updateErrorMetrics(const QMap<unsigned int, QVector<unsigned int>>& classifications, const QVector<T>& centroids)
    {
        _oldestError = _middleError;
        _middleError = _currentError;
        _currentError = findError(classifications, centroids);
        if (_oldestError > 0)
        {
            _errorDeceleration = -2 * _middleError + _oldestError + _currentError;
            if (abs(_errorDeceleration) <= 3.0)
            {
                _shouldStop = true;
            }
        }
    }

    unsigned int _numberOfClusters;

    std::function<qreal(const T&, const T&)> _distanceFunction;
    QVector<T> _dataPoints;

    QVector<T> _previousCentroids;
    QVector<qreal> _errorForK;

    qreal _oldestError;
    qreal _middleError;
    qreal _currentError;
    qreal _errorDeceleration;
    bool _shouldStop;
};

#endif // MEANSCLUSTER_H
