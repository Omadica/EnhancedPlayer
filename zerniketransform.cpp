#include "zerniketransform.h"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>


//void saveCameraParams( const std::string& filename,
//                      cv::Size imageSize, cv::Size boardSize,
//                      float squareSize, float aspectRatio, int flags,
//                      const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
//                      const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
//                      const std::vector<float>& reprojErrs,
//                      const std::vector<std::vector<cv::Point2f> >& imagePoints,
//                      double totalAvgErr )
//{
//    cv::FileStorage fs( filename, cv::FileStorage::WRITE );

//    time_t tt;
//    time( &tt );
//    struct tm *t2 = localtime( &tt );
//    char buf[1024];
//    strftime( buf, sizeof(buf)-1, "%c", t2 );

//    fs << "calibration_time" << buf;

//    if( !rvecs.empty() || !reprojErrs.empty() )
//        fs << "nframes" << (int)std::max(rvecs.size(), reprojErrs.size());
//    fs << "image_width" << imageSize.width;
//    fs << "image_height" << imageSize.height;
//    fs << "board_width" << boardSize.width;
//    fs << "board_height" << boardSize.height;
//    fs << "square_size" << squareSize;

//    if( flags & cv::CALIB_FIX_ASPECT_RATIO )
//        fs << "aspectRatio" << aspectRatio;

//    if( flags != 0 )
//    {
//        sprintf( buf, "flags: %s%s%s%s",
//                flags & cv::CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
//                flags & cv::CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
//                flags & cv::CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
//                flags & cv::CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "" );
//        //cvWriteComment( *fs, buf, 0 );
//    }

//    fs << "flags" << flags;

//    fs << "camera_matrix" << cameraMatrix;
//    fs << "distortion_coefficients" << distCoeffs;

//    fs << "avg_reprojection_error" << totalAvgErr;
//    if( !reprojErrs.empty() )
//        fs << "per_view_reprojection_errors" << cv::Mat(reprojErrs);

//    if( !rvecs.empty() && !tvecs.empty() )
//    {
//        CV_Assert(rvecs[0].type() == tvecs[0].type());
//        cv::Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
//        for( int i = 0; i < (int)rvecs.size(); i++ )
//        {
//            cv::Mat r = bigmat(cv::Range(i, i+1), cv::Range(0,3));
//            cv::Mat t = bigmat(cv::Range(i, i+1), cv::Range(3,6));

//            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
//            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
//            //*.t() is MatExpr (not Mat) so we can use assignment operator
//            r = rvecs[i].t();
//            t = tvecs[i].t();
//        }
//        //cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
//        fs << "extrinsic_parameters" << bigmat;
//    }

//    if( !imagePoints.empty() )
//    {
//        cv::Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
//        for( int i = 0; i < (int)imagePoints.size(); i++ )
//        {
//            cv::Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
//            cv::Mat imgpti(imagePoints[i]);
//            imgpti.copyTo(r);
//        }
//        fs << "image_points" << imagePtMat;
//    }
//}

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

//        cv::imshow("Image",frame);
//        cv::waitKey(0);
    }

    cv::destroyAllWindows();

    cv::Mat cameraMatrix, distCoeffs, R, T, newCameraMatrix;

    /*
    * Performing camera calibration by
    * passing the value of known 3D points (objpoints)
    * and corresponding pixel coordinates of the
    * detected corners (imgpoints)
    */

    cv::Size ImgSize = cv::Size(gray.rows, gray.cols);
    cv::fisheye::calibrate(objPoints, imgPoints, ImgSize, cameraMatrix, distCoeffs, R, T);
    cv::FileStorage file("cameraCalibration.ext", cv::FileStorage::WRITE);

    //cv::Mat newCameraMatrix = cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, ImgSize, 0.7, ImgSize);
    //cv::fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, ImgSize, R, newCameraMatrix);


    file << "cameraMat" << cameraMatrix;
    file << "newCameraMat" << newCameraMatrix;
    file << "distCoeffs" << distCoeffs;
    file << "Rvec" << R;
    file << "Tvec" << T;
    file << "map1" << objPoints;
    file << "map2" << imgPoints;

    file.release();


//    cv::FileStorage file_read("cameraCalibration.ext", cv::FileStorage::READ);

//    cv::Mat cameraMat;
//    cv::Mat dcoeff;
//    cv::Mat RMat;
//    cv::Mat TMat;

//    file_read["cameraMat"] >> cameraMat;
//    file_read["distCoeffs"] >> dcoeff;
//    file_read["Rvec"] >> RMat;
//    file_read["Tvec"] >> TMat;

//    file_read.release();

//    cv::Mat new_frame;

//    for(auto &it : images){
//        frame = cv::imread(it);
//        cv::undistort(frame, new_frame, cameraMat, dcoeff, cameraMat);
//        cv::imwrite(cv::String("/tmp/") + it, new_frame);
//        //cv::imshow("Undistorted image", new_frame);
//        //cv::waitKey(0);
//    }

}
