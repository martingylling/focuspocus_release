#include "imageprocessing.h"

/****************************************************************************
** File Name:   imageprocessing.cpp
**
** Description:
**     The ImageProcessing class is used for image processing operations such as
**     aligning images, computing depth maps, and focus stacking images.
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


ImageProcessing::ImageProcessing(QObject *parent)
    : QObject{parent}
{}

/// Computes the local variance of an image
/// \param laplacian The input image
/// \param varianceMap The output variance map
/// \param windowSize The size of the window for computing the local variance
void ImageProcessing::compute_local_variance(const cv::Mat& laplacian, cv::Mat& varianceMap, int windowSize) {
    // Compute the squared Laplacian
    cv::Mat laplacianSquared;
    cv::pow(laplacian, 2, laplacianSquared);

    // Compute the mean of the Laplacian
    cv::Mat mean, meanSquare;
    cv::boxFilter(laplacian, mean, CV_64F, cv::Size(windowSize, windowSize));

    // Compute the mean of the squared Laplacian
    cv::boxFilter(laplacianSquared, meanSquare, CV_64F, cv::Size(windowSize, windowSize));

    // Compute the variance: variance = meanSquare - mean^2
    varianceMap = meanSquare - mean.mul(mean); // mul is element-wise multiplication
}

/// Aligns images using SIFT feature matching and homography estimation
/// \param images The images to align
/// \return The aligned images
std::vector<cv::Mat> ImageProcessing::align_images(const std::vector<cv::Mat>& images) {
    if (images.empty()) {
        std::cerr << "No images provided for alignment." << std::endl;
        return {};
    }

    cv::Ptr<SIFT> detector = cv::SIFT::create( );

    //Assume image 0 is base image
    std::vector<cv::Mat> outImages;
    outImages.push_back(images[0]); // Base image

    // Process base image
    cv::Mat baseGray;
    cv::cvtColor(images[0], baseGray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(baseGray, baseGray);
    std::vector<cv::KeyPoint> baseKeypoints;
    cv::Mat baseDescriptors;
    detector->detectAndCompute(baseGray, cv::noArray(), baseKeypoints, baseDescriptors);

    emit progress("Aligning images.",0,images.size()-1);
    // Process remaining images
    for (size_t i = 1; i < images.size(); ++i) {
        std::cout << "Aligning image " << i << std::endl;

        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        cv::Mat gray;
        cv::cvtColor(images[i], gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        detector->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

        // Match descriptors using FLANN
        FlannBasedMatcher matcher;
        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher.knnMatch(baseDescriptors, descriptors, knnMatches, 2); // Find the 2 nearest neighbors

        // Filter good matches using Lowe's ratio test
        std::vector<cv::DMatch> goodMatches;
        const float ratioThresh = 0.75f; // Lowe's ratio test threshold
        for (const auto& knnMatch : knnMatches) {
            if (knnMatch.size() >= 2 && knnMatch[0].distance < ratioThresh * knnMatch[1].distance) {
                goodMatches.push_back(knnMatch[0]);
            }
        }

        // Extract location of good matches
        std::vector<Point2f> pointsRef, pointsCur;
        for (const auto& match : goodMatches) {
            pointsRef.push_back(baseKeypoints[match.queryIdx].pt);
            pointsCur.push_back(keypoints[match.trainIdx].pt);
        }

        //Make sure there are enough points to find homography
        if(pointsCur.size() < 4 || pointsRef.size() < 4){
            std::cerr << "Not enough points to find homography" << std::endl;
            continue;
        }


        // Warp the current image to align with the reference
        Mat aligned;
        Mat H = cv::estimateAffinePartial2D(pointsCur, pointsRef, cv::noArray(), cv::RANSAC);
        warpAffine(images[i], aligned, H, images[0].size(), cv::INTER_CUBIC, cv::BORDER_REPLICATE);
        outImages.push_back(aligned);
        emit renderImage(aligned);
        emit progress("Aligning images.",i,images.size()-1);
    }

    return outImages;
}

/// Computes the depth map from a stack of images
/// \param images The images to compute the depth map from
/// \param estimationRadius The radius for the laplacian estimation
/// \param smoothRadius The radius for the gaussian smoothing
/// \param contrastThreshold The threshold for the laplacian variance
///
cv::Mat ImageProcessing::compute_depth_map(const std::vector<cv::Mat>& images, int laplaceKernelSize, int smoothKernelSize, int smoothStrength, int smoothIterations){
    int rows = images[0].rows;
    int cols = images[0].cols;

    cv::Mat depthMap = cv::Mat::zeros(rows, cols, CV_8U);
    cv::Mat sharpnessMax = cv::Mat::zeros(rows, cols, CV_64F);

    emit progress("Generating depth map.",0, images.size());
    //Iterate through each layer in the stack calculating laplacian variance for each pixel and storing the maximum value
    for(int layer = 0; layer < images.size(); layer++){
        std::cout << "Processing layer " << layer << std::endl;
        cv::Mat image = images[layer];

        //Convert to grayscale
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        cv::Mat sharpness,laplacian, gaussian;

        cv::Laplacian(gray, laplacian, CV_64F, 5);

        //Compute the local variance of the laplacian
        compute_local_variance(laplacian, sharpness, laplaceKernelSize);

        for(int r = 0; r < rows; r++){
            for(int c = 0; c < cols; c++){
                double sharpnessValue = sharpness.at<double>(r,c);
                if( sharpnessValue >= sharpnessMax.at<double>(r,c)){
                    sharpnessMax.at<double>(r,c) = sharpnessValue;
                    depthMap.at<uchar>(r,c) = static_cast<uchar>(layer);
                }
            }
        }

        //Render depth map progress
        cv::Mat dMapProgress = depthMap.clone();
        //Normalize the depth map for visualization
        cv::normalize(dMapProgress, dMapProgress, 0, 255, cv::NORM_MINMAX);
        emit renderImage(dMapProgress, true);
        emit progress("Generating depth map.",layer+1, images.size());
    }

    //Smooth the depth map
    //Convert depth map to flaat before smoothing
    depthMap.convertTo(depthMap, CV_32F);

    //SMooth depth map using bilateral filtering
    cv::Mat depthMapSmoothed;
    emit progress("Smoothening depth map.", 0, smoothIterations);
    for(int i = 0; i < smoothIterations; i++){
         cv::bilateralFilter(depthMap, depthMapSmoothed, smoothKernelSize, smoothStrength, smoothStrength);
         depthMapSmoothed.copyTo(depthMap);
         emit progress("Smoothening depth map.", i+1, smoothIterations);
         //Render the soothened depth map
         cv::Mat dMapProgress = depthMapSmoothed.clone();

         //Normalize the depth map for visualization
         cv::normalize(dMapProgress, dMapProgress, 0, 255, cv::NORM_MINMAX);
         emit renderImage(dMapProgress, true);
    }

    return depthMapSmoothed;
}

/// Creates a composite image from a depth map
/// \param images The images to composite
/// \param depthMap The depth map
/// \param blendLayers Whether to blend layers
/// \return The composite image
cv::Mat ImageProcessing::create_composite_image_from_depth_map(const std::vector<cv::Mat>& images, const cv::Mat& depthMap, bool blendLayers){
    //print max value of depthMap
    double min, max;
    cv::minMaxLoc(depthMap, &min, &max);
    std::cout << "Max value of depth map: " << max << std::endl;

    cv::Mat composite = cv::Mat::zeros(images[0].size(), images[0].type());

    int numImages = images.size();

    for(int r = 0; r < depthMap.rows; r++){
        for(int c = 0; c < depthMap.cols; c++){
            float depthValue = depthMap.at<float>(r,c);

            if(blendLayers){
                //Determine the lower and upper layer indices
                int lowerLayer =  static_cast<int>(std::floor(depthValue));
                int upperLayer =  static_cast<int>(std::ceil(depthValue));

                //Clamp layer indices to a valid range
                lowerLayer = std::clamp(lowerLayer, 0, numImages-1);
                upperLayer = std::clamp(upperLayer, 0, numImages-1);

                //Calculate blending weight
                float weight = depthValue - lowerLayer;

                //Retrieve pixel values from the two layers
                cv::Vec3b lowerPixel = images[lowerLayer].at<cv::Vec3b>(r,c);
                cv::Vec3b upperPixel = images[upperLayer].at<cv::Vec3b>(r,c);

                //Blend the two pixel values
                cv::Vec3b blendedPixel;
                for(int i = 0; i < 3; i++){
                    blendedPixel[i] = static_cast<uchar>((1.0f - weight)*lowerPixel[i] + weight*upperPixel[i]);
                }

                //Assign the blended pixel value to the composite image
                composite.at<cv::Vec3b>(r,c) = blendedPixel;
            }
            else{
                //Set layer index to the nearest integer value
                int layer = static_cast<int>(std::round(depthValue));
                composite.at<cv::Vec3b>(r,c) = images[layer].at<cv::Vec3b>(r,c);
            }
        }
    }

    return composite;
}

/// Focus stacks a set of images
/// \param unalignedImages The images to focus stack
/// \param laplaceKernelSize The kernel size for the laplacian
/// \param smoothKernelSize The kernel size for the smoothing
/// \param smoothStrength The strength of the smoothing
/// \param smoothIterations The number of smoothing iterations
/// \param blendLayers Whether to blend layers
void ImageProcessing::focus_stack(const std::vector<cv::Mat>& unalignedImages,int laplaceKernelSize, int smoothKernelSize, int smoothStrength, int smoothIterations, bool blendLayers) {
    std::vector<cv::Mat> images = align_images(unalignedImages);

    //Compute the depth map
    cv::Mat depthMap = compute_depth_map(images,laplaceKernelSize,smoothKernelSize,smoothStrength,smoothIterations);

    //Create the composite image from the depth map
    cv::Mat output = create_composite_image_from_depth_map(images, depthMap, blendLayers);

    // Emit the final output image
    emit focusStackingComplete(output);
}
