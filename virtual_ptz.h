#ifndef VIRTUAL_PTZ_H
#define VIRTUAL_PTZ_H

#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"

class VirtualPTZ
{
public:
    VirtualPTZ();
    virtual ~VirtualPTZ() = delete;

    void setPanTiltZoom(float pan, float tilt, float zoom);
    
private:
    float m_pan;
    float m_tilt;
    float m_zoom;

    cv::Mat m_camera;
    cv::Mat m_newCamera;
    cv::Mat m_rotation;

};


#endif // VIRTUAL_PTZ_H