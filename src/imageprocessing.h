#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>

using namespace cv;
using namespace std;

class ImageProcessing : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessing(QObject *parent = nullptr);

private:
    std::vector<cv::Mat> align_images(const std::vector<cv::Mat>& images);
    cv::Mat compute_depth_map(const std::vector<cv::Mat>& images, int laplaceKernelSize, int smoothKernelSize, int smoothStrength, int smoothIterations);
    cv::Mat create_composite_image_from_depth_map(const std::vector<cv::Mat>& images, const cv::Mat& depthMap, bool blendLayers);
    void compute_local_variance(const cv::Mat& input, cv::Mat& output, int windowSize);

public slots:
    void focus_stack(const std::vector<cv::Mat>& unalignedImages, int laplaceKernelSize, int smoothKernelSize, int smoothStrength, int smoothIterations, bool blendLayers);

signals:
    void focusStackingComplete(cv::Mat result);
    void renderImage(cv::Mat image, bool grayscale = false);
    void progress(QString label, int value, int max);
};

#endif // IMAGEPROCESSING_H
