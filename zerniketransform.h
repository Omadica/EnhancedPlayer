#ifndef ZERNIKETRANSFORM_H
#define ZERNIKETRANSFORM_H
#include "qtmetamacros.h"
#include <string>
#include <opencv2/opencv.hpp>


extern "C" {
#include <libavcodec/avcodec.h>
}


class ZernikeTransform
{
public:
    ZernikeTransform();
    virtual ~ZernikeTransform() = delete;

    void setLens(std::string lens);
    void setCamera(std::string camera);

public slots:
    void transformFrame();

signals:
    void lensNotFound();
    void cameraNotFound();

private:
    /***
     * Image conversion
     */
//    void imageFromFrame(IplImage* image, AVFrame *frame);
//    void frameFromImage(AVFrame *frame, IplImage* image);

    /***
     * Distorsions
     */
    void distortPoints();
    void rectifyMap();
    void projectPoints();
    void undistortImage();


    /***
     * Calibration
     */
    double calibrate();
    void estimationMatrixRectify();
    cv::Mat Camera_Matrix;
    cv::Mat Distortion_coefficients;

};

#endif // ZERNIKETRANSFORM_H
