#include "zerniketransform.h"
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>



ZernikeTransform::ZernikeTransform()
{
    qDebug() << "ZernikTransform CTOR";


}


void ZernikeTransform::transformFrame()
{
    // Distortion_coefficients
    // Camera_Matrix
    cv::Mat image;
    cv::Mat undistortedImage;

    cv::undistort(image, undistortedImage, Camera_Matrix, Distortion_coefficients);
}

