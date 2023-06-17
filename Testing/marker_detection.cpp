// detects the marker, get ID and pose

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <filesystem>

#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/Testing/MarkerMovie.MP4"
using namespace std;

vector<vector<cv::Point>> findContourAndSquare(/*cv::Mat frame*/){

    cv::Mat frame;
    cv::VideoCapture cap(0, cv::CAP_FFMPEG);

    if (!cap.isOpened()){
        cout << "No Webcam detected" << endl;
        cap.open(VIDEOPATH, cv::CAP_FFMPEG);
        if (!cap.isOpened()){
            cout << "No video file detected" << endl;
            exit(0);
        }
    }

    vector <vector<cv::Point>> candidates;
    while (cap.read(frame)){
        cv::Mat frame_copy = frame.clone();

        // apply greyscale filter 
        cv::Mat frame_grey;
        cv::cvtColor(frame_copy, frame_grey, cv::COLOR_BGR2GRAY);

        // tresholding 
        cv::Mat frame_thresh;
        cv::threshold(frame_grey, frame_thresh, 85, 255, cv::THRESH_BINARY);
        // cv::adaptiveThreshold(frame_grey, frame_thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 7);
            
        /* find contours --> find white blobs over black background ========================================== */
        frame_thresh = ~frame_thresh; // invert image
        vector<vector<cv::Point>> contours;

        // use external contour to remove inner contours inside the marker
        cv::findContours(frame_thresh, contours, cv::RETR_EXTERNAL , cv::CHAIN_APPROX_SIMPLE);
        cv::cvtColor(frame_thresh, frame_thresh, cv::COLOR_GRAY2BGR);

		// drawContours(frame_thresh, contours, -1, cv::Scalar(0, 255, 0), 2);

        /* find squares --> find 4 corners of the marker ===================================================== */
        vector<cv::Point> contour_poly_approx;
        for (int i = 0; i < contours.size(); i++){

            // approximate contour to a polygon
            double epsilon = 0.02 * cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], contour_poly_approx, epsilon, true);


            // if contour is not a square, continue
            if (contour_poly_approx.size() != 4 || !cv::isContourConvex(contour_poly_approx) || cv::contourArea(contour_poly_approx) < 400){
                continue;
            }

            // order points in clockwise order
            // find center points of each contour
            double cx = (contour_poly_approx[0].x + contour_poly_approx[1].x + contour_poly_approx[2].x + contour_poly_approx[3].x) / 4;
            double cy = (contour_poly_approx[0].y + contour_poly_approx[1].y + contour_poly_approx[2].y + contour_poly_approx[3].y) / 4;

            if (contour_poly_approx[0].x <= cx && contour_poly_approx[0].y <= cy){
                // if its the top left corner, swap diagonals
                cv::swap(contour_poly_approx[1], contour_poly_approx[3]);
            } else {
                // if its the top right corner, swap bottom and top points
                cv::swap(contour_poly_approx[0], contour_poly_approx[1]);
                cv::swap(contour_poly_approx[2], contour_poly_approx[3]);  
            }

            
            cv::Scalar colour = cv::Scalar(0, 255, 0);
            cv::Rect r = cv::boundingRect(contour_poly_approx);

            cv::polylines(frame_thresh, contour_poly_approx, true, colour, 2);
            cv::circle(frame_thresh, contour_poly_approx[0], 3, cv::Scalar(0, 0, 255), 2);      // red
            cv::circle(frame_thresh, contour_poly_approx[1], 3, cv::Scalar(0, 255, 0), 2);      // green
            cv::circle(frame_thresh, contour_poly_approx[2], 3, cv::Scalar(255, 0, 0), 2);      // blue
            cv::circle(frame_thresh, contour_poly_approx[3], 3, cv::Scalar(0, 255, 255), 2);    // yellow

        }

        


        cv::namedWindow("Threshold");
        cv::namedWindow("Original");
        cv::imshow("Threshold", frame_thresh);
        cv::imshow("Original", frame_copy);
        if (cv::waitKey(24) == 27){
            break;
        }
    }
		 
 	
 	cv::destroyWindow("Threshold");



    return vector<vector<cv::Point>>();
}

void detectMarker(/*cv::Mat frame*/){
    findContourAndSquare(/*frame*/);

    // 1. find contours --> find white blobs over black background
    // 2. find squares --> find 4 corners of the marker
    // 3. find marker
    // 4. get ID and pose
}

int main(int argc, char const *argv[])
{
    detectMarker();

    return 0;
}
