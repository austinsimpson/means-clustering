#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"
#include "MeansCluster.h"

#include <QPointF>

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();

private:
    QVector<QPointF> normalizeData(const QVector<QPointF>& unnormalizedData);
    void loadTrainingPoints(const QString& filePath);
    MeansCluster<QPointF> _cluster;
    QVector<QPointF> _trainingPoints;

};

#endif // MAINWINDOW_H
