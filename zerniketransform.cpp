#include "zerniketransform.h"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>


ZernikeTransform::ZernikeTransform()
{
    qDebug() << "ZernikTransform CTOR";
}

void ZernikeTransform::transformFrame()
{
    const int BOARD[2]{6, 9};

    std::vector<std::vector<cv::Point3f>> objPoints;
    std::vector<std::vector<cv::Point2f>> imgPoints;

    std::vector<cv::Point3f> objp;
    for(int i{0}; i < BOARD[1]; i++)
        for(int j{0}; j < BOARD[0]; j++)
            objp.push_back(cv::Point3f(j,i,0));

    std::vector<cv::String> images;
    std::string path = "*.png";

    cv::glob(path, images);
    cv::Mat frame, gray;

    std::vector<cv::Point2f> corner_pts;
    bool success;

    // Looping over all the images in the directory
    for(int i{0}; i<images.size(); i++)
    {
        std::cout << "Load image: " << images[i] << std::endl;
        frame = cv::imread(images[i]);
        cv::cvtColor(frame,gray,cv::COLOR_BGR2GRAY);

        // Finding checker board corners
        // If desired number of corners are found in the image then success = true
        success = cv::findChessboardCorners(gray, cv::Size(BOARD[0], BOARD[1]), corner_pts, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

        /*
         * If desired number of corner are detected,
         * we refine the pixel coordinates and display
         * them on the images of checker board
        */
        if(success)
        {
            std::cout << "Found chessboard in: " << images[i] << std::endl;
            cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 1e-6);

            // refining pixel coordinates for given 2d points.
            cv::cornerSubPix(gray, corner_pts, cv::Size(11,11), cv::Size(-1,-1), criteria);

            // Displaying the detected corner points on the checker board
            cv::drawChessboardCorners(frame, cv::Size(BOARD[0], BOARD[1]), corner_pts, success);

            objPoints.push_back(objp);
            imgPoints.push_back(corner_pts);
        }

    }

    cv::destroyAllWindows();

    cv::Mat cameraMatrix, distCoeffs, R, T;

    /*
    * Performing camera calibration by
    * passing the value of known 3D points (objpoints)
    * and corresponding pixel coordinates of the
    * detected corners (imgpoints)
    */

    cv::Size ImgSize = cv::Size(gray.rows, gray.cols);
    //cv::fisheye::calibrate(objPoints, imgPoints, ImgSize, cameraMatrix, distCoeffs, R, T);
    cv::fisheye::calibrate(objPoints, imgPoints, ImgSize, cameraMatrix, distCoeffs, R, T);
    cv::FileStorage file("cameraCalibration.ext", cv::FileStorage::WRITE);

    cv::Mat newCameraMatrix = cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, ImgSize, 0.7, ImgSize);
    // cv::fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, ImgSize, R, newCameraMatrix);


    file << "cameraMat" << cameraMatrix;
    file << "newCameraMat" << newCameraMatrix;
    file << "distCoeffs" << distCoeffs;
    file << "Rvec" << R;
    file << "Tvec" << T;
    // file << "map1" << objPoints;
    // file << "map2" << imgPoints;
    file.release();
}
