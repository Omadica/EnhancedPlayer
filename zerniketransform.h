#ifndef ZERNIKETRANSFORM_H
#define ZERNIKETRANSFORM_H
#include "qtmetamacros.h"
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2//imgproc/imgproc.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <opencv2/core/types_c.h>
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
    void imageFromFrame(IplImage* image, AVFrame *frame);
    void frameFromImage(AVFrame *frame, IplImage* image);

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
    Mat Camera_Matrix;
    Mat Distortion_coefficients;

};

#endif // ZERNIKETRANSFORM_H
