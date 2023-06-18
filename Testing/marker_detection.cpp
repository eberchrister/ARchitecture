// detects the marker, get ID and pose

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <filesystem>

#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/Testing/MarkerMovie.MP4"
using namespace std;

vector<vector<cv::Point>> findContourAndSquare(cv::Mat frame){
    vector <vector<cv::Point>> candidates;
        cv::Mat frame_copy = frame.clone();

        // apply greyscale filter --> easier to analyze the intensity rather than the color
        cv::Mat frame_grey;
        cv::cvtColor(frame_copy, frame_grey, cv::COLOR_BGR2GRAY);

        // tresholding 
        cv::Mat frame_thresh;
        cv::threshold(frame_grey, frame_thresh, 85, 255, cv::THRESH_BINARY);

        // adaptive thresholding --> not used because it is not as good as the normal thresholding in this case
        // cv::adaptiveThreshold(frame_grey, frame_thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 7);
            
        /* ===================================================== find contours --> find white blobs over black background ========================================== */
        // invert image --> white blobs over black background
        frame_thresh = ~frame_thresh; 
        vector<vector<cv::Point>> contours;

        // use external contour to remove inner contours inside the marker --> only the outer contour is kept becau
        cv::findContours(frame_thresh, contours, cv::RETR_EXTERNAL , cv::CHAIN_APPROX_SIMPLE);
        cv::cvtColor(frame_thresh, frame_thresh, cv::COLOR_GRAY2BGR);

		// drawContours(frame_thresh, contours, -1, cv::Scalar(0, 255, 0), 2);

        /* ===================================================== find squares --> find 4 corners of the marker ===================================================== */
        vector<cv::Point> contour_poly_approx;
        for (int i = 0; i < contours.size(); i++){

            // approximate contour to a polygon
            double epsilon = 0.02 * cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], contour_poly_approx, epsilon, true);

            cv::Rect r = cv::boundingRect(contour_poly_approx);
            // if contour is not a square, continue
            if (contour_poly_approx.size() != 4 || !cv::isContourConvex(contour_poly_approx) || cv::contourArea(contour_poly_approx) < 400
            /* don't include contour if it touches the border of the image */
            || r.x <= 0 || r.y <= 0 || r.x + r.width >= frame_copy.cols || r.y + r.height >= frame_copy.rows){
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

            candidates.push_back(contour_poly_approx);
        }

        // for debugging purposes

        for (int i = 0; i < candidates.size(); i++){
            vector <cv::Point> candidate = candidates[i];
            cout << "Candidate " << i << ": " << candidate << endl;

            cv::Scalar colour = cv::Scalar(255, 0, 255);
            cv::Rect r = cv::boundingRect(candidates[i]);

            cv::polylines(frame_thresh, candidates[i], true, colour, 2);
            cv::circle(frame_thresh, candidates[i][0], 3, cv::Scalar(0, 0, 255), 2);      // red
            cv::circle(frame_thresh, candidates[i][1], 3, cv::Scalar(0, 255, 0), 2);      // green
            cv::circle(frame_thresh, candidates[i][2], 3, cv::Scalar(255, 0, 0), 2);      // blue
            cv::circle(frame_thresh, candidates[i][3], 3, cv::Scalar(0, 255, 255), 2);    // yellow
        }

        // end of debugging purposes

    cv::imshow("Contoured and Squared", frame_thresh);

    return candidates;
}

vector<int> getIds(cv::Mat frame, vector<cv::Point> square_contour, int bits){
    int numPixels = sqrt(bits);

    // clockwise order from top left corner
    vector<cv::Point2f> corners{cv::Point2f{0, 0}, cv::Point2f{bits, 0}, cv::Point2f{bits, bits}, cv::Point2f{0, bits}};

    // get transformation matrix to warp the image into a straightened square with the size bits x bits
        // convert square contour to float
    vector<cv::Point2f> square_contour_float;
    for (int i = 0; i < square_contour.size(); i++){
        square_contour_float.push_back(cv::Point2f(square_contour[i].x, square_contour[i].y));
    }

    cv::Mat transformationM = cv::getPerspectiveTransform(square_contour_float, corners);
    
    // warp image into a straightened square with the size bits x bits
    cv::Mat warped;
    cv::warpPerspective(frame, warped, transformationM, cv::Size(bits, bits));

    // now reduce noise by thresholding the image
    cv::Mat warped_thresh;
    cv::cvtColor(warped, warped, cv::COLOR_BGR2GRAY);
        // thresh_otsu scans the image to find the best threshold value
    cv::threshold(warped, warped_thresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    
    // the white blob is still not perfect, so erode it to remove noise
    cv::Mat eroded = warped_thresh.clone();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ERODE , cv::Size(3, 3));
    // cv::dilate(eroded, eroded, kernel);
    cv::erode(eroded, eroded, kernel);
    

    // read the bits from the image per cell
    vector<int> ids;
    for (int row = 0; row < numPixels; row++){
        for (int column = 0; column < numPixels; column++){
            // get center of cell
            int x = column * numPixels + (numPixels / 2);
            int y = row * numPixels + (numPixels / 2);

            // draw circle at center of cell
            cv::circle(eroded, cv::Point{x,y}, 1, cv::Scalar(0, 0, 255), 1);

            if  (eroded.at<uchar>(y, x) >= 128){
                ids.push_back(1);
            } else {
                ids.push_back(0);
            }
        }
    }
    
    cv::cvtColor(eroded, eroded, cv::COLOR_GRAY2BGR);
    // draw grid lines for analysis
    for (int i = 0; i < numPixels; i++){
        cv::line(eroded, cv::Point2f(0, i * (bits / numPixels)), cv::Point2f(bits, i * (bits / numPixels)), cv::Scalar(255, 128, 64), 1);
        cv::line(eroded, cv::Point2f(i * (bits / numPixels), 0), cv::Point2f(i * (bits / numPixels), bits), cv::Scalar(255, 128, 64), 1);
    }

    // make window resizable
    cv::namedWindow("warped", cv::WINDOW_NORMAL);
    cv::imshow("warped", warped_thresh);

    cv::namedWindow("eroded", cv::WINDOW_NORMAL);
    cv::imshow("eroded", eroded);

    // print IDs in a 6x6 grid
    cout << "IDs: ";
    for (int row = 0; row < numPixels; row++){
        for (int column = 0; column < numPixels; column++){
            cout << ids[row * numPixels + column] << " ";
        }
        cout << endl;
    }

    return ids;
}

cv::Mat detectMarker(cv::Mat frame){
    cv::Mat frame_clone = frame.clone();

    // 1. find contours --> find white blobs over black background
    // 2. find squares --> find 4 corners of the marker
    vector<vector<cv::Point>> candidates = findContourAndSquare(frame_clone);

    // 3. find marker
    for (int i = 0; i < candidates.size(); i++){
        vector<cv::Point> square = candidates[i];
        vector<int> ids = getIds(frame_clone, square, 36);
    }

    // 4. get ID and pose
    return frame_clone; // return frame with marker detected and ID and pose on marke
}

int main(int argc, char const *argv[]){

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

    while(cap.read(frame)){

        vector<vector<cv::Point>> candidates = findContourAndSquare(frame);
        for (int i = 0; i < candidates.size(); i++){
            vector<cv::Point> square = candidates[i];
            vector<int> ids = getIds(frame, square, 36);

            // verify if the marker is valid

        }
        

        // cv::namedWindow("Threshold");
        cv::namedWindow("Original");
        // cv::imshow("Threshold", frame_processed);
        cv::imshow("Original", frame);
        if (cv::waitKey(24) == 27){
            break;
        }

    }

    return 0;
}