#include <opencv2/opencv.hpp>

using namespace std;

struct MarkerDict{
    vector<vector<int>> ids;
    vector<vector<cv::Point3f>> orientations;
};

struct MarkerResult{
    vector<cv::Point> corners;
    int index = -1;
};


class MarkerDetection{
    public:
        /**
         * Finds the contour of any 4 sided shape in the frame and returns the coordinates of the corners 
         * 
         * This already includes filtering out any contours that are not 4 sided, too small, too large, 
         * not convex, and touching the edge of the frame. Furthermore, it also sorts the corners in a 
         * clockwise order.
         * 
         * @param frame The frame (an image/a single frame of a video) to find the contour in
         * @return a vector of candidate markers. Each marker is a vector of 4 points (squares).
         */
        static vector<vector<cv::Point>> findContourAndSquare(cv::Mat frame, bool debug);

        /**
         * Find the IDs of all the markers in the frame
         * 
         * Each marker is warped into a bits*bits image and is divded into sqrt(bits)*sqrt(bits) cells.
         * The center value of each cell (black -> 0, white -> 1) makes up the ID of each marker.
         * 
         * @param frame The image containing the marker
         * @param square_contour The contour of the marker
         * @param bits The number of rows/columns in the square marker
         * @return an ID vector of the marker
         */
        static vector<int> getIds(cv::Mat frame, vector<cv::Point> square_contour, int bits, bool debug);

        /**
         * Constructs a dictionary of markers
         * 
         * Reads in all the marker images and obtains their IDs for each orientation.
         * Serves as a lookup table for the marker detection algorithm.
         * 
         * @param markerPaths A vector of paths to the marker images
         * @return a dictionary of markers
        */
        static MarkerDict constructMarkerDictionary(vector<string> markerPaths);

        /**
         * Detects the markers in the frame
         * 
         * @param frame The frame to detect the markers in
         * @param dict The dictionary of markers
         * @param error_threshold The maximum number of errors allowed when comparing the marker to the dictionary
         * @return a vector of detected markers
         */
        static vector<MarkerResult> detectMarker(cv::Mat frame, MarkerDict dict, int error_threshold, bool debug);

        /**
         * Estimates the pose of a single marker
         * 
         * @param orientations The orientations of the marker
         * @param corners The corners of the marker
         * @param cameraMatrix The camera matrix
         * @param distCoeffs The distortion coefficients
         * @return vector of projected points of the marker 
         */
        static vector<cv::Point2f> poseEstimation(vector<cv::Point3f> orientations, vector<cv::Point> corners, cv::Mat cameraMatrix, cv::Mat distCoeffs);
        static vector<cv::Point2f> pointsEstimation(vector<cv::Point3f> orientations, vector<cv::Point> corners, cv::Mat cameraMatrix, cv::Mat distCoeffs, vector<cv::Point3f> axis);
};