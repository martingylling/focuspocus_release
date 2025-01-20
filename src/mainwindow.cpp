
/****************************************************************************
** File Name:   mainwindow.cpp
**
** Description:
**     This file contains the implementation of the MainWindow class, which
**     is the main window of the FocusPocus application. The main window
**     provides a user interface for opening images, focus stacking images,
**     and saving the result.
**
** Author:      Martin Gylling
** Created On:  2024-12-16
**
** Last Modified By: Martin Gylling
** Last Modified On: 2025-01-20
**
** License: LGPL (Lesser General Public License)
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->LayersList->setIconSize(QSize(30,30));

    //Hide progressbar and progresslabel
    ui->ProgressBar->hide();
    ui->ProgressLabel->hide();

    //Set window to be maximized
    this->showMaximized();

    //Create the settingsdirectory in the application directory if it does not exist
    QDir settingsDir(qApp->applicationDirPath() + "/settings");
    if(!settingsDir.exists()){
        settingsDir.mkpath(".");
    }

    //Set up QGraphicsScenes for the images
    previewScene = new QGraphicsScene();
    ui->PreviewImage->setScene(previewScene);
    resultScene = new QGraphicsScene();
    ui->ResultImage->setScene(resultScene);
    renderScene = new QGraphicsScene();
    ui->RenderImage->setScene(renderScene);


    //Make sure that images are of high quality
    ui->PreviewImage->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->ResultImage->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->RenderImage->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //register qmetatypes
    qRegisterMetaType<std::vector<cv::Mat>>("std::vector<cv::Mat>");

    imageProcessor = new ImageProcessing();
    connect(this, &MainWindow::focusStackImages, imageProcessor, &ImageProcessing::focus_stack);
    connect(imageProcessor, &ImageProcessing::focusStackingComplete, this, &MainWindow::focusStackingComplete);
    connect(imageProcessor, &ImageProcessing::renderImage, this, &MainWindow::renderImage);
    connect(imageProcessor, &ImageProcessing::progress, this, &MainWindow::progress);

    //Move imageProcessor to another thread to prevent UI from freezing
    QThread *thread = new QThread();
    imageProcessor->moveToThread(thread);
    thread->start();

    // Synchronize the spinboxes and sliders
    connect(ui->LaplacianKernelSlider, &QSlider::valueChanged, [=](int value) {
        int oddValue = (value % 2 == 0) ? value + 1 : value;
        ui->LaplacianKernelSlider->setValue(oddValue);
        ui->LaplacianKernelSpinBox->setValue(oddValue);
    });

    connect(ui->LaplacianKernelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        int oddValue = (value % 2 == 0) ? value + 1 : value;
        ui->LaplacianKernelSlider->setValue(oddValue);
    });

    connect(ui->SmoothKernelSlider, &QSlider::valueChanged, [=](int value) {
        int oddValue = (value % 2 == 0) ? value + 1 : value;
        ui->SmoothKernelSlider->setValue(oddValue);
        ui->SmoothKernelSpinbox->setValue(oddValue);
    });

    connect(ui->SmoothKernelSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        int oddValue = (value % 2 == 0) ? value + 1 : value;
        ui->SmoothKernelSlider->setValue(oddValue);
    });
    connect(ui->SmoothStrengthSlider, &QSlider::valueChanged, ui->SmoothStrengthSpinBox, &QSpinBox::setValue);
    connect(ui->SmoothStrengthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->SmoothStrengthSlider, &QSlider::setValue);
}

MainWindow::~MainWindow()
{
    delete ui;
}


/// Resize the images in the QGraphicsViews when the window is resized
/// \param event The resize event
void MainWindow::resizeImages(){
    // Resize the images in the QGraphicsViews when the window is resized
    showImageInScene(layer, previewScene, std::min(ui->PreviewImage->width(),ui->PreviewImage->height()));
    showImageInScene(stackresult, resultScene, std::max(ui->ResultImage->width(),ui->ResultImage->height()));
    showImageInScene(render, renderScene,std::min(ui->RenderImage->width(),ui->RenderImage->height()));
}

/// Handle the resize event of the main window
/// \param event The resize event
void MainWindow::resizeEvent(QResizeEvent* event) {
    // Resize the images in the QGraphicsViews when the window is resized
    resizeImages();

    // Call the base class's resizeEvent to process other necessary events
    QMainWindow::resizeEvent(event);
}

/// When the user clicks the Open File action in the men
void MainWindow::on_action_Open_File_triggered()
{

    //Prompt the user to select image file or multiple files
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select one or more files to open", "/home", "Images (*.png *.bmp *.jpg *.jpeg)");

    if(fileNames.isEmpty()){
        return;
    }

    ui->LayersList->clear();

    //Iterate through each file selected
    for(QString file: fileNames){
        QString name = file.split("/").last();

        //Add each image to the listWidget with preview image and filename included
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(name);

        //Set icon to the image selected
        item->setIcon(QIcon(file));

        item->setData(Qt::UserRole,file);

        ui->LayersList->addItem(item);
    }
}

/// Displays the selected image from the listWidget in the QGraphicsView
/// \param currentRow The index of the selected item
void MainWindow::on_LayersList_currentRowChanged(int currentRow)
{
    //Get the selected item
    QListWidgetItem *selectedItem = ui->LayersList->currentItem();
    if(selectedItem != nullptr){
        //Get the image path
        QString imagePath = selectedItem->data(Qt::UserRole).toString();

        //Change tab to the first tab
        ui->tabWidget->setCurrentIndex(0);

        // Display the image in PreviewImage
        QImage img(imagePath);
        layer = img.copy();
        showImageInScene(img, previewScene, std::min(ui->PreviewImage->width(),ui->PreviewImage->height()));
    }
}

/// When the user clicks the Stack button
void MainWindow::on_StackButton_clicked()
{
    //Show message box if layers list is empty
    if(ui->LayersList->count() == 0){
        QMessageBox::warning(this,"Error","No images to stack");
        return;
    }

    //Create cv::Mat from each image in LayersList
    std::vector<cv::Mat> images;
    for(int i=0; i<ui->LayersList->count(); i++){
        QListWidgetItem *item = ui->LayersList->item(i);
        QString imagePath = item->data(Qt::UserRole).toString();
        cv::Mat image = cv::imread(imagePath.toStdString());
        images.push_back(image);
    }

    //Make sure that images have the same size
    for(int i=1; i<images.size(); i++){
        if(images[i].size() != images[0].size()){
            QMessageBox::warning(this,"Error","Images must have the same size");
            return;
        }
    }

    //Emit signal to process images
    int laplaceKernelSize = ui->LaplacianKernelSpinBox->value();
    int smoothKernelSize = ui->SmoothKernelSpinbox->value();
    int smoothStrength = ui->SmoothStrengthSpinBox->value();
    int smoothIterations = ui->SmoothIterations->value();
    bool blendLayers = ui->BlendLayers->isChecked();

    emit focusStackImages(images,laplaceKernelSize,smoothKernelSize,smoothStrength,smoothIterations,blendLayers);
    ui->StackButton->setEnabled(false);
    ui->StackButton->setHidden(true);

    //Change tab to the first tab
    ui->tabWidget->setCurrentIndex(0);
}

/// Displays the result of the focus stacking in the QGraphicsView
/// \param focusedImage The result of the focus stacking
void MainWindow::focusStackingComplete(cv::Mat focusedImage){
    ui->StackButton->setEnabled(true);
    ui->StackButton->setHidden(false);

    ui->ProgressBar->setHidden(true);
    ui->ProgressLabel->setHidden(true);

    //Ensure focusedImage is 8-bit and has 3 channels (BGR)
    if (focusedImage.type() != CV_8UC3) {
        focusedImage.convertTo(focusedImage, CV_8UC3);
    }

    // Convert BGR to RGB
    cv::Mat focusedImageRGB;
    cv::cvtColor(focusedImage, focusedImageRGB, cv::COLOR_BGR2RGB);

    // Convert cv::Mat to QImage
    QImage img(focusedImageRGB.data, focusedImageRGB.cols, focusedImageRGB.rows, focusedImageRGB.step, QImage::Format_RGB888);

    //Store this image in variable to later be able to save it.
    stackresult = img.copy();

    //Change tab to the second tab
    ui->tabWidget->setCurrentIndex(1);

    // Display the image in ResultImage scaling the image to fit the size of the QGraphicsView
    showImageInScene(img, resultScene, std::max(ui->ResultImage->width(),ui->ResultImage->height()));
}

/// Display the image in the QGraphicsView
/// \param image The image to display
/// \param scene The QGraphicsScene to display the image in
/// \param size The size of the image
void MainWindow::showImageInScene(const QImage &image, QGraphicsScene *scene, int size){
    QPixmap pixmap = QPixmap::fromImage(image);
    if (!pixmap.isNull()) {
        scene->clear(); // Clear previous items
        //Limit size of pixmap
        if(pixmap.width() > size || pixmap.height() > size){
            pixmap = pixmap.scaled(size,size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }
        scene->addPixmap(pixmap);
        scene->setSceneRect(pixmap.rect()); // Adjust the scene's size to the pixmap
    }
}

/// When the user clicks the Save File action in the menu
void MainWindow::on_action_Save_File_triggered()
{
    //Only ask for save directory if stackresult in not empty, else display dialog.
    if(stackresult.isNull()){
        QMessageBox::warning(this,"Error","No image to save.");
        return;
    }

    //Open export dialog to select image quality
    ExportDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        int quality = dialog.getImageQuality();
        //Ask to save the image
        QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "/stacked-image", "PNG Image (*.png);;JPEG Image (*.jpg)");
        if(!fileName.isEmpty()){
            stackresult.save(fileName,nullptr,quality);
            //Display information dialog when save is complete
            QMessageBox::information(this,"Success","Image saved.");
        }
    }
}

/// Displays the rendered image in the RenderImage QGraphicsView
/// \param image The image to display
/// \param grayscale Whether the image is grayscale or not
void MainWindow::renderImage(cv::Mat image, bool grayscale){
    //Handle grayscale images
    if(grayscale){
        //Convert 1 channel float image to 8-bit
        if(image.type() == CV_32F){
            //cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
            image.convertTo(image, CV_8U);
        }
        QImage img(image.data, image.cols, image.rows, image.step, QImage::Format_Grayscale8);
        render = img.copy();
        showImageInScene(img, renderScene,std::min(ui->RenderImage->width(),ui->RenderImage->height()));
    }
    else{
        // Ensure image is 8-bit and has 3 channels (BGR)
        if (image.type() != CV_8UC3) {
            image.convertTo(image, CV_8UC3);
        }

        // Convert BGR to RGB
        cv::Mat imageRGB;
        cv::cvtColor(image, imageRGB, cv::COLOR_BGR2RGB);

        // Convert cv::Mat to QImage
        QImage img(imageRGB.data, imageRGB.cols, imageRGB.rows, imageRGB.step, QImage::Format_RGB888);
        render = img.copy();
        showImageInScene(img, renderScene,std::min(ui->RenderImage->width(),ui->RenderImage->height()));
    }
}

/// Display progress in the progressbar
/// \param label The label to display
/// \param value The current value
/// \param max The maximum value
void MainWindow::progress(QString label, int value, int max){
    ui->ProgressBar->setMaximum(max);
    ui->ProgressBar->setValue(value);
    ui->ProgressLabel->setText(label);
    ui->ProgressBar->show();
    ui->ProgressLabel->show();
}

/// When the user clicks the Save parameters button
void MainWindow::on_SaveParams_clicked()
{
    // Define parameters
    QMap<QString, QVariant> params;
    params["Laplacian Kernel size"] = ui->LaplacianKernelSpinBox->value();
    params["Smooth Kernel size"] = ui->SmoothKernelSpinbox->value();
    params["Smooth strength"] = ui->SmoothStrengthSpinBox->value();
    params["Smooth iterations"] = ui->SmoothIterations->value();
    params["Blend layers"] = ui->BlendLayers->isChecked();

    //Prompt the user to select a path to save the settingsfile.
    QString filePath = QFileDialog::getSaveFileName(this, "Save Parameters", qApp->applicationDirPath() + "/settings", "Parameters (*.param)");

    if(!filePath.isEmpty()){
        Settings::save(filePath,params);
        QMessageBox::information(this,"Success","Parameters saved.");
    }
}

/// When the user clicks the Load parameters button
void MainWindow::on_LoadParams_clicked()
{
    //Prompt the user to select a file to load parameters from
    QString filePath = QFileDialog::getOpenFileName(this, "Load Parameters", qApp->applicationDirPath() + "/settings", "Parameters (*.param)");

    if(!filePath.isEmpty()){
        bool ok = true;
        QMap<QString, QVariant> params = Settings::load(filePath, ok);
        if(!ok){
            QMessageBox::warning(this,"Error","Could not load parameters.");
            return;
        }
        ui->LaplacianKernelSpinBox->setValue(params["Laplacian Kernel size"].toUInt());
        ui->SmoothKernelSpinbox->setValue(params["Smooth Kernel size"].toUInt());
        ui->SmoothStrengthSpinBox->setValue(params["Smooth strength"].toDouble());
        ui->SmoothIterations->setValue(params["Smooth iterations"].toUInt());
        ui->BlendLayers->setChecked(params["Blend layers"].toBool());
        QMessageBox::information(this,"Success","Parameters loaded.");
    }
}

/// When the user clicks the Restore Default button
void MainWindow::on_RestoreDefault_clicked()
{
    //Restore default parameter values
    ui->LaplacianKernelSpinBox->setValue(3);
    ui->SmoothKernelSpinbox->setValue(17);
    ui->SmoothStrengthSpinBox->setValue(100);
    ui->SmoothIterations->setValue(5);
    ui->BlendLayers->setChecked(true);
}

/// When the How to use action is triggered
void MainWindow::on_action_How_to_use_triggered()
{
    //Pop up a dialog describing how to use the application and what effects the differens parameters have on the result.
    QMessageBox::information(this,"How to use","1. Open the images by clicking File -> Open Files or by pressing Ctrl+O.\n"
                                              "2. Make sure that the layers are in order so that focus goes from background to foreground or vice versa.\n"
                                              "3. Adjust parameters to your liking\n"
                                              "4. Click Stack images to begin the stacking process.\n"
                                              "5. Save result by clicking File -> Save File or by pressing Ctrl+S.\n");
}

/// When the About action is triggered
void MainWindow::on_action_About_triggered()
{
    //Show the about dialog
    AboutDialog dialog(this);
    dialog.exec();


}

