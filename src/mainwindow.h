#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QGraphicsScene>
#include <imageprocessing.h>
#include <QThread>
#include <QMessageBox>
#include <oddslider.h>
#include <oddspinbox.h>
#include <settings.h>
#include <exportdialog.h>
#include <aboutdialog.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_File_triggered();

    void on_LayersList_currentRowChanged(int currentRow);

    void on_StackButton_clicked();

    void focusStackingComplete(cv::Mat result);

    void on_action_Save_File_triggered();

    void showImageInScene(const QImage& image, QGraphicsScene* scene, int size = 1080);

    void renderImage(cv::Mat image, bool grayscale);

    void progress(QString label, int value, int max);

    void on_SaveParams_clicked();

    void on_LoadParams_clicked();

    void on_RestoreDefault_clicked();

    void on_action_How_to_use_triggered();

    void resizeImages();

    void on_action_About_triggered();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MainWindow *ui;
    QGraphicsScene *previewScene;
    QGraphicsScene *resultScene;
    QGraphicsScene *renderScene;
    ImageProcessing *imageProcessor;
    QImage stackresult;
    QImage layer;
    QImage render;

signals:
    void focusStackImages(const std::vector<cv::Mat>& unalignedImages,int laplaceKernelSize, int smoothKernelSize, int smoothStrength, int smoothIterations, bool blendLayers);
};
#endif // MAINWINDOW_H
