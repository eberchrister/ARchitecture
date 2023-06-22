#include "MarkerDetection.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <filesystem>
#include <iostream>

using namespace std;

#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/MarkerMovie.MP4"
#define MARKERPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/markers"

#define CAM_MTX (cv::Mat_<float>(3, 3) << 1000, 0.0, 500, 0.0, 1000, 500, 0.0, 0.0, 1.0)
#define CAM_DIST (cv::Mat_<float>(1, 4) << 0, 0, 0, 0)

int main(int argc, char const *argv[]){
    bool debug = false;
    if (argc >= 2){
        if (atoi(argv[1]) == 1){
            debug = true;
            cout << "[prog] Debug mode enabled" << endl;
        }
    }

    // read the files in a directory
    vector<string> markerPaths;
    for (const auto & entry : filesystem::directory_iterator(MARKERPATH)){
        markerPaths.push_back(entry.path());
    }

    // construct dictionary
    cout << "=========================================" << endl;
    MarkerDict dict = MarkerDetection::constructMarkerDictionary(markerPaths);
    cout << "=========================================" << endl;

    cv::Mat frame;
    cv::VideoCapture cap("http://192.168.0.7:4747/video", cv::CAP_FFMPEG);

    if (!cap.isOpened()){
        cout << "[prog] No Webcam detected, searching for video file" << endl;
        cap.open(VIDEOPATH, cv::CAP_FFMPEG);
        if (!cap.isOpened()){
            cout << "[prog] No video file detected, exiting" << endl;
            exit(0);
        }
        cout << "[prog] Video file detected" << endl;
    }

    cout << "=========================================" << endl;
    cout << "[prog] Video Metadata: " << endl;
    cout << "\tFrame Count: " << cap.get(cv::CAP_PROP_FRAME_COUNT) << endl;
    cout << "\tFrame Rate: " << cap.get(cv::CAP_PROP_FPS) << endl;
    cout << "\tFrame Dimension: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "=========================================" << endl;

    while(cap.read(frame)){
        cv::Mat frame_clone = frame.clone();
        cv::Mat frame_pose = frame.clone();

        vector<MarkerResult> results = MarkerDetection::detectMarker(frame_clone, dict, 0, debug);

        for (int i = 0; i < results.size(); i++){
            cv::putText(frame_clone, to_string(results[i].index), results[i].corners[0], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(32, 32, 255), 2);
            cv::drawContours(frame_clone, vector<vector<cv::Point>>{results[i].corners}, 0, cv::Scalar(0, 255, 32), 1);
        }

        // Pose Estimation        
        for (MarkerResult& res : results){
            vector<cv::Point2f> projectedPoints = MarkerDetection::poseEstimation(dict.orientations[res.index], res.corners, CAM_MTX, CAM_DIST);

            // draw the axis
            cv::line(frame_pose, projectedPoints[0], projectedPoints[1], cv::Scalar(0, 0, 255), 2);
            cv::line(frame_pose, projectedPoints[0], projectedPoints[2], cv::Scalar(0, 255, 0), 2);
            cv::line(frame_pose, projectedPoints[0], projectedPoints[3], cv::Scalar(255, 0, 0), 2);

            // and put a text on the image for the projected points
            if (debug){
                cv::putText(frame_pose, "x", projectedPoints[1], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1);
                cv::putText(frame_pose, "y", projectedPoints[2], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0), 1);
                cv::putText(frame_pose, "z", projectedPoints[3], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0), 1);
            }
        }


        cv::namedWindow("Original", cv::WINDOW_NORMAL);
        cv::imshow("Original", frame);
        if (cv::waitKey(12) == 27){
            break;
        }

        cv::namedWindow("ID", cv::WINDOW_NORMAL);
        cv::imshow("ID", frame_clone);
        if (cv::waitKey(12) == 27){
            break;
        }

        cv::namedWindow("Pose", cv::WINDOW_NORMAL);
        cv::imshow("Pose", frame_pose);
        if (cv::waitKey(12) == 27){
            break;
        }

    }

    return 0;
}
