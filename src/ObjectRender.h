#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

using namespace std;

class ObjectRender{
    public:
        /**
         * Adds two vectors together relative to an origin point
         * 
         * Since the relative origin position of each marker is not (0, 0), the relative origin must be
         * translated to (0, 0) before adding the two vectors together. After adding the two vectors, the
         * result is translated back to the original relative origin.
         * 
         * @param a The first vector
         * @param b The second vector
         * @param origin The relative origin
         * @param scaleA The scale of the first vector
         * @param scaleB The scale of the second vector
         * @return the sum of the two (scaled) vectors
        */
        static cv::Point2f vectorAddRelative(cv::Point2f a, cv::Point2f b, cv::Point2f origin, float scaleA, float scaleB);

        /**
         * Converts the projected points from OpenCV coordinate space to OpenGL coordinate space
         * 
         * OpenCV coordinate space has origin at the top left corner of the frame, with the x axis
         * pointing right and the y axis pointing down ([0, frame_width] * [0, frame_height]). OpenGL coordinate 
         * space has origin at the center of the frame, with the x axis pointing right and the y axis 
         * pointing up ([-1, 1] * [-1, 1]).
         * 
         * @param projectedPoints The 2D points in OpenCV coordinate space
         * @param frame_width The width of the frame
         * @param frame_height The height of the frame
         * @return vector of converted 2D projected points in OpenGL coordinate space
        */
        static vector<cv::Point2f> convertToGLCoords(vector<cv::Point2f> projectedPoints, int frame_width, int frame_height);

        /**
         * Sorts the wall markers based on how close they are to the camera (higher y value = closer to camera), 
         * since the OpenCV coordinate space has the y axis pointing down
         * 
         * @param wallMarkers the map of wall markers
         * @param frame_height the height of the frame
         * @return vector of keys for the sorted wall markers map
        */
        static vector<string> sortWallMarker(map<string, vector<cv::Point2f>> wallMarkers, int frame_height);

        /**
         * Draws a dynamic wall of the room based on the wall markers
         * 
         * Only two walls are being drawn at a time. Depending on the camera's position, the closest two walls  
         * to the camera are not drawn. This is to improve visibility of the room. 
         * 
         * @param wallMarkerCorners The map of wall markers
         * @param sortedKeyClosest The sorted keys of the wall markers
         * @param colors The colors of the walls
         * @param outline Whether or not to draw the outline of the walls
         * @param floor Whether or not to draw the floor
         * @param extraHeight The extra height of the walls
        */
        static void drawWalls(map<string, vector<cv::Point2f>> wallMarkerCorners, vector<string> sortedKeyClosest, vector<vector<GLfloat>> colors, bool outline, bool floor, float extraHeight);


    // the following functions share the same parameters
        /**
         * @param projectedGLPoints The GL projected points
         * @param babyBlue The baby blue color
         * @param orangeSalmon The orange salmon color
         * @param scale The scale of the table
        */
    
        /* Draws a 1x1 table based on the GL projected points */
        static void drawTable1x1(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);
    
        /* Draws a 1x2 table based on the GL projected points */
        static void drawTable1x2(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a basic chair (accompanying the table) based on the GL projected points*/
        static void drawBasicChair(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);
    
        /* Draws a bed based on the GL projected points */
        static void drawBed(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a small sofa based on the GL projected points */
        static void drawSmallSofa(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a long sofa based on the GL projected points */
        static void drawLongSofa(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a 1x1 table for the sofa based on the GL projected points */
        static void drawTableForSofa(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a stylized dining table based on the GL projected points */
        static void drawDiningTable(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a stylized dining chair based on the GL projected points */
        static void drawDiningChair(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a TV object based on the projected GL points */        
        static void drawTV(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);
    
        /* Draws a flat circular carpet on the marker based on the marker (GL) coordinates */
        static void drawCarpet(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);

        /* Draws a bookshelf based on the GL projected points */
        static void drawBookshelf(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> babyBlue, vector<vector<GLfloat>> orangeSalmon, float scale);
};