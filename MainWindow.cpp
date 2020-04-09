#include "MainWindow.h"

#include <QFile>
#include <QTextStream>

#include <QDebug>

#include <QFileDialog>
#include <QPointF>

#include <cmath>

MainWindow::MainWindow
(
    QWidget* parent
):
    QMainWindow(parent),
    Ui::MainWindow()
{
    setupUi(this);

    loadTrainingPoints(":/data/test_data_1.csv");
    _clusterWidget->setPoints(normalizeData(_trainingPoints));
}

MainWindow::~MainWindow()
{
}

QVector<QPointF> MainWindow::normalizeData(const QVector<QPointF>& unnormalizedData)
{
    float meanX = 0.0;
    float expectationXSquared = 0.0;
    

    float meanY = 0.0;
    float expectationYSquared = 0.0;

    for (auto point : unnormalizedData)
    {
        meanX += point.x();
        meanY += point.y();

        expectationXSquared += pow(point.x(), 2);
        expectationYSquared += pow(point.y(), 2);
    }

    meanX /= unnormalizedData.size();
    meanY /= unnormalizedData.size();

    expectationXSquared /= unnormalizedData.size();
    expectationYSquared /= unnormalizedData.size();

    float standardDeviationX = sqrt(expectationXSquared - pow(meanX, 2.0));
    float standardDeviationY = sqrt(expectationYSquared - pow(meanY, 2.0));

    QVector<QPointF> result;

    for (auto point : unnormalizedData)
    {
        QPointF toAdd = point;
        toAdd.rx() -= meanX;
        toAdd.ry() -= meanY;

        toAdd.rx() /= standardDeviationX;
        toAdd.ry() /= standardDeviationY;
        result.append(toAdd);
    }
    return result;
}

void MainWindow::loadTrainingPoints
(
    const QString& filePath
)
{
    _trainingPoints.clear();

    QFile dataPointsFile(filePath);
    if (dataPointsFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&dataPointsFile);
        while (stream.atEnd() == false)
        {
            QString line = stream.readLine();
            QStringList pointAsList = line.split(",");
            if (pointAsList.count() == 2)
            {
                bool xOk = false;
                bool yOk = false;
                double x = pointAsList[0].toDouble(&xOk);
                double y = pointAsList[1].toDouble(&yOk);
                if (xOk && yOk)
                {
                    _trainingPoints.append(QPointF(x, y));
                }
            }
        }
        dataPointsFile.close();
    }
}

void MainWindow::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.isNull() == false)
    {
        loadTrainingPoints(fileName);
        _clusterWidget->setPoints(normalizeData(_trainingPoints));
    }
}
