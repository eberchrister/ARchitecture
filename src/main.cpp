#include "MarkerDetection.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <filesystem>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <GL/glu.h>

using namespace std;

// #define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/MarkerMovie.MP4"
#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/trial_wall.mp4"
#define MARKERPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/markers"


#define CAM_MTX (cv::Mat_<float>(3, 3) << 1000, 0.0, 500, 0.0, 1000, 500, 0.0, 0.0, 1.0)
#define CAM_DIST (cv::Mat_<float>(1, 4) << 0, 0, 0, 0)
float angle = 0;


void setPerspective(float fovY, float aspect, float zNear, float zFar)
{
    float top = tan(fovY * 0.5 * (CV_PI / 180.0)) * zNear;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;

    glFrustum(left, right, bottom, top, zNear, zFar);
}

cv::Point2f vectorAddRelative(cv::Point2f a, cv::Point2f b, cv::Point2f origin, float scaleA, float scaleB){
    float a_x = a.x - origin.x;
    float a_y = a.y - origin.y;
    a_y = -a_y;

    float b_x = b.x - origin.x;
    float b_y = b.y - origin.y;
    b_y = -b_y;

    // cv::Point2f four_trans = cv::Point2f(one_trans.x + two_trans.x, one_trans.y + two_trans.y);
            // cv::Point2f four = cv::Point2f(four_trans.x + zero.x, -four_trans.y + zero.y);

    cv::Point2f c_trans = cv::Point2f(scaleA*a_x + scaleB*b_x, scaleA*a_y + scaleB*b_y);
    cv::Point2f c = cv::Point2f(c_trans.x + origin.x, -c_trans.y + origin.y);
    return c;
}

// returns the order of the marker from (closest --> furthest)
vector<string> sortWallMarker(map<string, vector<cv::Point2f>> wallMarkers, int frame_height){
    vector<string> sortedMarkers;

    // calculate area of each marker relative to (0, frame_height)
    float topLeftA = frame_height-wallMarkers["topLeft"][0].y;
    float topRightA = frame_height-wallMarkers["topRight"][0].y;
    float bottomRightA = frame_height-wallMarkers["bottomRight"][0].y;
    float bottomLeftA = frame_height-wallMarkers["bottomLeft"][0].y;

    // sort the markers based on area
    vector<pair<string, float>> markerAreas = {{"topLeft", topLeftA}, {"topRight", topRightA}, {"bottomRight", bottomRightA}, {"bottomLeft", bottomLeftA}};
    sort(markerAreas.begin(), markerAreas.end(), [](const pair<string, float> &a, const pair<string, float> &b){
        return a.second < b.second;
    });

    // return the sorted markers
    for (const auto & marker : markerAreas){
        sortedMarkers.push_back(marker.first);
    }

    return sortedMarkers;
}

// draw two walls, connecting the farthest and its neighbors. color is a triple of (r, g, b) --> [0]: outer wall, [1]: inner wall, [2]: roof
void drawWalls(map<string, vector<cv::Point2f>> wallMarkerCorners, vector<string> sortedKeyClosest /*, vector<tuple<GLfloat, GLfloat, GLfloat>> colors*/){
    vector<string> sortedKeyClosest1 = sortedKeyClosest;
    cout << "sortedKeyClosest1: " << sortedKeyClosest1[1] << ", " << sortedKeyClosest1[2] << ", " << sortedKeyClosest1[3] << endl;

    string furthest = sortedKeyClosest[3];
    string neighbor1 = sortedKeyClosest[2];
    string neighbor2 = sortedKeyClosest[1];
    // draw second closest to furthest floor
    glColor3f(0.5f, 0.0f, 0.5f);
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(0.5f, 0.5f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(0.0f, 0.5f, 0.5f);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(0.0, 0.5f, 0.0);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        



    // draw closest to furthest wall
    glColor3f(0.5f, 0.0f, 0.5f);
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(0.5f, 0.5f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(0.0f, 0.5f, 0.5f);
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(0.0, 0.5f, 0.0);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);

}

vector<cv::Point2f> convertToGLCoords(vector<cv::Point2f> projectedPoints, int frame_width, int frame_height){
    vector<cv::Point2f> points2D;

    for (const auto & point : projectedPoints){
        float x = point.x/frame_width * 2.0f - 1.0f;
        float y = point.y/frame_height * 2.0f - 1.0f;
        points2D.push_back(cv::Point2f(x, y));
    }

    return points2D;
}


int main(int argc, char const *argv[]){
    bool debug = false;
    if (argc >= 2){
        if (atoi(argv[1]) == 1){
            debug = true;
            cout << "[prog] Debug mode enabled" << endl;
        }
    }

    cv::Mat frame;
    cv::VideoCapture cap("http://192.168.0.7:4747/video", cv::CAP_FFMPEG);

    if (!cap.isOpened()){
        cout << "[CV] No Webcam detected, searching for video file" << endl;
        cap.open(VIDEOPATH, cv::CAP_FFMPEG);
        if (!cap.isOpened()){
            cout << "[CV] No video file detected, exiting" << endl;
            exit(0);
        }
        cout << "[CV] Video file detected" << endl;
    }

    int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int frame_rate = cap.get(cv::CAP_PROP_FPS);
    cout << "=========================================" << endl;
    cout << "[CV] Video Metadata: " << endl;
    cout << "\tFrame Count: " << cap.get(cv::CAP_PROP_FRAME_COUNT) << endl;
    cout << "\tFrame Rate: " << frame_rate << endl;
    cout << "\tFrame Dimension: " << frame_width << "x" << frame_height << endl;
    cout << "=========================================" << endl;

    // read the files in a directory
    vector<string> markerPaths;
    for (const auto & entry : filesystem::directory_iterator(MARKERPATH)){
        markerPaths.push_back(entry.path());
    }
    // sort the files in ascending order (marker0, marker1, marker2, ...)
    std::sort(markerPaths.begin(), markerPaths.end());

    // construct dictionary
    cout << "=========================================" << endl;
    MarkerDict dict = MarkerDetection::constructMarkerDictionary(markerPaths);
    cout << "=========================================" << endl;

    // Initialize GLFW
    if (!glfwInit()){
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    cout << "[GLFW] GLFW initialized" << endl;

    GLFWwindow* window = glfwCreateWindow(frame_width, frame_height, "render", NULL, NULL);
    if (!window)
    {
        std::cerr << "[GLFW] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    

    while(cap.read(frame)){
        cv::Mat frame_clone = frame.clone();
        cv::Mat frame_pose = frame.clone();
        cv::Mat frame_render = frame.clone();

        vector<MarkerResult> results = MarkerDetection::detectMarker(frame_clone, dict, 0, debug);

        for (int i = 0; i < results.size(); i++){
            cv::putText(frame_clone, to_string(results[i].index), results[i].corners[0], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(32, 32, 255), 2);
            cv::drawContours(frame_clone, vector<vector<cv::Point>>{results[i].corners}, 0, cv::Scalar(0, 255, 32), 1);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Convert the frame to OpenGL texture format
        cv::flip(frame_render, frame_render, 0);  // Flip vertically
        cv::cvtColor(frame_render, frame_render, cv::COLOR_BGR2RGB);

        // Set up the texture mapping
        glEnable(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_render.data);

        // draw a rectangle that covers the entire screen, overlaying the webcam feed as a texture
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
            glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
            glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
            glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
        glEnd();    

        // Set up the camera
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        
        // setPerspective(45, 1, 1, 10);

        // store wall marker corners
        map<string, vector<cv::Point2f>> wallMarkerCorners;
        
        // Pose Estimation        
        for (MarkerResult& res : results){
            vector<cv::Point2f> projectedPoints = MarkerDetection::poseEstimation(dict.orientations[res.index], res.corners, CAM_MTX, CAM_DIST);

            cv::Point2f zero = projectedPoints[0];
            cv::Point2f one = projectedPoints[1];
            cv::Point2f two = projectedPoints[2];
            cv::Point2f three = projectedPoints[3];
            cv::Point2f four = projectedPoints[4];
            cv::Point2f five = projectedPoints[5];
            cv::Point2f six = projectedPoints[6];
            cv::Point2f seven = projectedPoints[7];

            // draw axis lines
            cv::line(frame_pose, zero, one, cv::Scalar(0, 0, 255), 1);
            cv::line(frame_pose, two, zero, cv::Scalar(0, 255, 0), 1);
            cv::line(frame_pose, zero, three, cv::Scalar(255, 0, 0), 1);
            cv::line(frame_pose, one, four, cv::Scalar(255, 0, 255), 1);
            cv::line(frame_pose, four, two, cv::Scalar(255, 0, 255), 1);
            cv::line(frame_pose, one, six, cv::Scalar(255, 255, 0), 1);
            cv::line(frame_pose, four, five, cv::Scalar(255, 255, 0), 1);
            cv::line(frame_pose, two, seven, cv::Scalar(255, 255, 0), 1);
            cv::line(frame_pose, five, seven, cv::Scalar(0, 255, 255), 1);
            cv::line(frame_pose, three, seven, cv::Scalar(0, 255, 255), 1);
            cv::line(frame_pose, five, six, cv::Scalar(0, 255, 255), 1);
            cv::line(frame_pose, three, six, cv::Scalar(0, 255, 255), 1);

            /*======================================== MATH IMPLEMENTATION FOR COORDINATE FINDING, will be deleted later ========================================*/
            // float one_x = projectedPoints[1].x - projectedPoints[0].x;
            // float one_y = projectedPoints[1].y - projectedPoints[0].y;
            // one_y = -one_y;
            // cv::Point2f one_trans = cv::Point2f(one_x, one_y);
            // float two_x = projectedPoints[2].x - projectedPoints[0].x;
            // float two_y = projectedPoints[2].y - projectedPoints[0].y;
            // two_y = -two_y;
            // cv::Point2f two_trans = cv::Point2f(two_x, two_y);
            // float three_x = projectedPoints[3].x - projectedPoints[0].x;
            // float three_y = projectedPoints[3].y - projectedPoints[0].y;
            // three_y = -three_y;
            // cv::Point2f three_trans = cv::Point2f(three_x, three_y);

            // // cv::Point2f r = cv::Point2f(b.x + c.x, b.y + c.y);
            // // cv::Point2f r_trans = cv::Point2f(r.x + projectedPoints[0].x, r.y + projectedPoints[0].y);
            // cout << "zero: " << projectedPoints[0] << endl;
            // cout << "one: " << projectedPoints[1] << endl;
            // cout << "two: " << projectedPoints[2] << endl;
            // cout << "three: " << projectedPoints[3] << endl;
            // cout << "one_trans: " << one_trans << endl;
            // cout << "two_trans: " << two_trans << endl;
            // cout << "three_trans" << three_trans << endl;

            
            // cv::Point2f four_trans = cv::Point2f(one_trans.x + two_trans.x, one_trans.y + two_trans.y);
            // cv::Point2f four = cv::Point2f(four_trans.x + zero.x, -four_trans.y + zero.y);

            // cv::Point2f five_trans = cv::Point2f(one_trans.x + three_trans.x, one_trans.y + three_trans.y);
            // cv::Point2f five = cv::Point2f(five_trans.x + zero.x, -five_trans.y + zero.y);

            // cv::Point2f six_trans = cv::Point2f(two_trans.x + three_trans.x, two_trans.y + three_trans.y);
            // cv::Point2f six = cv::Point2f(six_trans.x + zero.x, -six_trans.y + zero.y);

            // cv::Point2f seven_trans = cv::Point2f(four_trans.x + three_trans.x, four_trans.y + three_trans.y);
            // cv::Point2f seven = cv::Point2f(seven_trans.x + zero.x, -seven_trans.y + zero.y);
            // projectedPoints = vector<cv::Point2f>{zero, one, two, three, four, five, six, seven};
            /*===================================================================================================================================================*/

            cv::circle(frame_pose, zero, 3, cv::Scalar(75, 25, 230), -1);       // red      - lower top left
            cv::circle(frame_pose, one, 3, cv::Scalar(48, 130, 245), -1);       // orange   - lower top right
            cv::circle(frame_pose, two, 3, cv::Scalar(25, 255, 255), -1);       // yellow   - lower bottom left
            cv::circle(frame_pose, three, 3, cv::Scalar(60, 245, 210), -1);     // lime     - upper top left
            cv::circle(frame_pose, four, 3, cv::Scalar(75, 180, 60), -1);       // green    - lower bottom right
            cv::circle(frame_pose, five, 3, cv::Scalar(240, 240, 70), -1);      // cyan     - upper bottom right
            cv::circle(frame_pose, six, 3, cv::Scalar(200, 130, 0), -1);        // blue     - upper top left
            cv::circle(frame_pose, seven, 3, cv::Scalar(180, 30, 145), -1);     // purple   - upper bottom left

            cv::circle(frame_pose, vectorAddRelative(cv::Point2f(one.x, one.y), cv::Point2f(two.x, two.y), zero, 0.5,0.5), 3, cv::Scalar(48, 130, 245), -1);

            // and put a text on the image for the projected points
            if (debug){
                cv::putText(frame_pose, "0", zero, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(75, 25, 230), 1);
                cv::putText(frame_pose, "1", one, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(48, 130, 245), 1);
                cv::putText(frame_pose, "2", two, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(25, 255, 255), 1);
                cv::putText(frame_pose, "3", three, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(60, 245, 210), 1);
                cv::putText(frame_pose, "4", four, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(75, 180, 60), 1);
                cv::putText(frame_pose, "5", five, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(240, 240, 70), 1);
                cv::putText(frame_pose, "6", six, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 130, 0), 1);
                cv::putText(frame_pose, "7", seven, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180, 30, 145), 1);
            }
            //GLfloat scalingFactor = 2.0f;
        
            //GLfloat x0,x1,x2,x3;
            //GLfloat y0,y1,y2,y3;
            /*
            x0 = projectedPoints[0].x/frame_render.cols * scalingFactor;
            x1 = projectedPoints[1].x/frame_render.cols * scalingFactor;
            x2 = projectedPoints[2].x/frame_render.cols * scalingFactor;
            x3 = projectedPoints[3].x/frame_render.cols * scalingFactor;
            
            y0 = 1 - projectedPoints[0].y/frame_render.rows * scalingFactor;
            y1 = 1 - projectedPoints[1].y/frame_render.rows * scalingFactor;
            y2 = 1 - projectedPoints[2].y/frame_render.rows * scalingFactor;
            y3 = 1 - projectedPoints[3].y/frame_render.rows * scalingFactor;
         */
           // GLfloat x1;
           // GLfloat x3;

            vector<cv::Point2f> projectedGLPoints = convertToGLCoords(projectedPoints, frame_width, frame_height);
            
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            switch(res.index) {
                // top left wall
                case 0:
                case 1:
                case 2:
                case 3:
                    wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("topLeft", projectedGLPoints));
                    break;
                // top right wall
                case 4:
                case 5:
                case 6:
                case 7:               
                    wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("topRight", projectedGLPoints));
                    break;
                // bottom right wall
                case 8:
                case 9:
                case 10:
                case 11:
                    wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("bottomRight", projectedGLPoints));
                    break;
                // bottom left wall
                case 12:
                case 13:
                case 14:
                case 15:
                    wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("bottomLeft", projectedGLPoints));
                    break;
                case 16:
                glPushMatrix();
                    
                    glDisable(GL_TEXTURE_2D);
                    glBegin(GL_QUADS);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    // cube bottom
                    glVertex2f(projectedGLPoints[0].x, -projectedGLPoints[0].y);
                    glVertex2f(projectedGLPoints[1].x, -projectedGLPoints[1].y);
                    glVertex2f(projectedGLPoints[4].x, -projectedGLPoints[4].y);
                    glVertex2f(projectedGLPoints[2].x, -projectedGLPoints[2].y);
                    // cube back
                    glColor3f(0.0f, 0.0f, 1.0f);
                    glVertex2f(projectedGLPoints[0].x, -projectedGLPoints[0].y);
                    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
                    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
                    glVertex2f(projectedGLPoints[2].x, -projectedGLPoints[2].y);
                    // cube right
                    glColor3f(0.0f, 1.0f, 1.0f);
                    glVertex2f(projectedGLPoints[1].x, -projectedGLPoints[1].y);
                    glVertex2f(projectedGLPoints[0].x, -projectedGLPoints[0].y);
                    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
                    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
                    // cube left
                    glColor3f(1.0f, 0.0f, 1.0f);
                    glVertex2f(projectedGLPoints[4].x, -projectedGLPoints[4].y);
                    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
                    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
                    glVertex2f(projectedGLPoints[2].x, -projectedGLPoints[2].y);
                    // cube front
                    glColor3f(1.0f, 1.0f, 0.0f);
                    glVertex2f(projectedGLPoints[1].x, -projectedGLPoints[1].y);
                    glVertex2f(projectedGLPoints[4].x, -projectedGLPoints[4].y);
                    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
                    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
                    // cube top
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
                    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
                    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
                    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);

                    glEnd();
                    glPopMatrix();
                    break;
                case 17:
                case 18:
                case 19:
                    // draw here
                    break;
                case 20:
                case 21:
                case 22:
                case 23:
                    // draw here
                    break;
                default:
                // just dummy stuff bc it can't be empty
                    glPushMatrix();
                    glPopMatrix();
            }

            if (wallMarkerCorners.size() == 4) {
                vector<string> sortedWallName = sortWallMarker(wallMarkerCorners, frame_height);

                
                cout << "[prog] all wall markers found" << endl;
                glPushMatrix();
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);

                // TODO: CREATE FUNCTION FOR THIS ============================================================================
                // tuple<GLfloat, GLfloat,GLfloat> wallColor{0.0f, 0.0f, 0.0f};
                // vector<tuple<GLfloat, GLfloat,GLfloat>> wallColors{wallColor, wallColor, wallColor, wallColor};
                drawWalls(wallMarkerCorners, sortedWallName);
                // top left - top right
                // glColor3f(0.5f, 0.0f, 0.5f);
                // glVertex2f(wallMarkerCorners["topLeft"][0].x, -wallMarkerCorners["topLeft"][0].y);
                // glVertex2f(wallMarkerCorners["topRight"][0].x, -wallMarkerCorners["topRight"][0].y);
                // glVertex2f(wallMarkerCorners["topRight"][4].x, -wallMarkerCorners["topRight"][4].y);
                // glVertex2f(wallMarkerCorners["topLeft"][4].x, -wallMarkerCorners["topLeft"][4].y);
                //     // inner wall
                //     glColor3f(0.0f, 0.5f, 0.5f);
                //     glVertex2f(wallMarkerCorners["topLeft"][4].x, -wallMarkerCorners["topLeft"][4].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][5].x, -wallMarkerCorners["topLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topRight"][5].x, -wallMarkerCorners["topRight"][5].y);
                //     glVertex2f(wallMarkerCorners["topRight"][4].x, -wallMarkerCorners["topRight"][4].y);
                //     // outer wall
                //     glColor3f(0.5f, 0.5f, 0.0f);
                //     glVertex2f(wallMarkerCorners["topLeft"][0].x, -wallMarkerCorners["topLeft"][0].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][3].x, -wallMarkerCorners["topLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["topRight"][3].x, -wallMarkerCorners["topRight"][3].y);
                //     glVertex2f(wallMarkerCorners["topRight"][0].x, -wallMarkerCorners["topRight"][0].y);
                //     // upper wall
                //     glColor3f(0.0f, 0.0f, 0.5f);
                //     glVertex2f(wallMarkerCorners["topLeft"][3].x, -wallMarkerCorners["topLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][5].x, -wallMarkerCorners["topLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topRight"][5].x, -wallMarkerCorners["topRight"][5].y);
                //     glVertex2f(wallMarkerCorners["topRight"][3].x, -wallMarkerCorners["topRight"][3].y);
                    


                // // top right - bottom right
                // glColor3f(0.5f, 0.0f, 0.5f);
                // glVertex2f(wallMarkerCorners["topRight"][0].x, -wallMarkerCorners["topRight"][0].y);
                // glVertex2f(wallMarkerCorners["bottomRight"][0].x, -wallMarkerCorners["bottomRight"][0].y);
                // glVertex2f(wallMarkerCorners["bottomRight"][4].x, -wallMarkerCorners["bottomRight"][4].y);
                // glVertex2f(wallMarkerCorners["topRight"][4].x, -wallMarkerCorners["topRight"][4].y);
                //     // inner wall
                //     glColor3f(0.0f, 0.5f, 0.5f);
                //     glVertex2f(wallMarkerCorners["topRight"][4].x, -wallMarkerCorners["topRight"][4].y);
                //     glVertex2f(wallMarkerCorners["topRight"][5].x, -wallMarkerCorners["topRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][5].x, -wallMarkerCorners["bottomRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][4].x, -wallMarkerCorners["bottomRight"][4].y);
                //     // outer wall
                //     glColor3f(0.5f, 0.5f, 0.0f);
                //     glVertex2f(wallMarkerCorners["topRight"][0].x, -wallMarkerCorners["topRight"][0].y);
                //     glVertex2f(wallMarkerCorners["topRight"][3].x, -wallMarkerCorners["topRight"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][3].x, -wallMarkerCorners["bottomRight"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][0].x, -wallMarkerCorners["bottomRight"][0].y);
                //     // upper wall
                //     glColor3f(0.0f, 0.0f, 0.5f);
                //     glVertex2f(wallMarkerCorners["topRight"][3].x, -wallMarkerCorners["topRight"][3].y);
                //     glVertex2f(wallMarkerCorners["topRight"][5].x, -wallMarkerCorners["topRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][5].x, -wallMarkerCorners["bottomRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][3].x, -wallMarkerCorners["bottomRight"][3].y);

                // // bottom right - bottom left
                // glColor3f(0.5f, 0.0f, 0.5f);
                // glVertex2f(wallMarkerCorners["bottomRight"][0].x, -wallMarkerCorners["bottomRight"][0].y);
                // glVertex2f(wallMarkerCorners["bottomLeft"][0].x, -wallMarkerCorners["bottomLeft"][0].y);
                // glVertex2f(wallMarkerCorners["bottomLeft"][4].x, -wallMarkerCorners["bottomLeft"][4].y);
                // glVertex2f(wallMarkerCorners["bottomRight"][4].x, -wallMarkerCorners["bottomRight"][4].y);
                //     // inner wall
                //     glColor3f(0.0f, 0.5f, 0.5f);
                //     glVertex2f(wallMarkerCorners["bottomRight"][4].x, -wallMarkerCorners["bottomRight"][4].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][5].x, -wallMarkerCorners["bottomRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][5].x, -wallMarkerCorners["bottomLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][4].x, -wallMarkerCorners["bottomLeft"][4].y);
                //     // outer wall
                //     glColor3f(0.5f, 0.5f, 0.0f);
                //     glVertex2f(wallMarkerCorners["bottomRight"][0].x, -wallMarkerCorners["bottomRight"][0].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][3].x, -wallMarkerCorners["bottomRight"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][3].x, -wallMarkerCorners["bottomLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][0].x, -wallMarkerCorners["bottomLeft"][0].y);
                //     // upper wall
                //     glColor3f(0.0f, 0.0f, 0.5f);
                //     glVertex2f(wallMarkerCorners["bottomRight"][3].x, -wallMarkerCorners["bottomRight"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomRight"][5].x, -wallMarkerCorners["bottomRight"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][5].x, -wallMarkerCorners["bottomLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][3].x, -wallMarkerCorners["bottomLeft"][3].y);
                

                // // bottom left - top left
                // glColor3f(0.5f, 0.0f, 0.5f);
                // glVertex2f(wallMarkerCorners["bottomLeft"][0].x, -wallMarkerCorners["bottomLeft"][0].y);
                // glVertex2f(wallMarkerCorners["topLeft"][0].x, -wallMarkerCorners["topLeft"][0].y);
                // glVertex2f(wallMarkerCorners["topLeft"][4].x, -wallMarkerCorners["topLeft"][4].y);
                // glVertex2f(wallMarkerCorners["bottomLeft"][4].x, -wallMarkerCorners["bottomLeft"][4].y);
                //     // inner wall
                //     glColor3f(0.0f, 0.5f, 0.5f);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][4].x, -wallMarkerCorners["bottomLeft"][4].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][5].x, -wallMarkerCorners["bottomLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][5].x, -wallMarkerCorners["topLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][4].x, -wallMarkerCorners["topLeft"][4].y);
                //     // outer wall
                //     glColor3f(0.5f, 0.5f, 0.0f);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][0].x, -wallMarkerCorners["bottomLeft"][0].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][3].x, -wallMarkerCorners["bottomLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][3].x, -wallMarkerCorners["topLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][0].x, -wallMarkerCorners["topLeft"][0].y);
                //     // upper wall
                //     glColor3f(0.0f, 0.0f, 0.5f);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][3].x, -wallMarkerCorners["bottomLeft"][3].y);
                //     glVertex2f(wallMarkerCorners["bottomLeft"][5].x, -wallMarkerCorners["bottomLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][5].x, -wallMarkerCorners["topLeft"][5].y);
                //     glVertex2f(wallMarkerCorners["topLeft"][3].x, -wallMarkerCorners["topLeft"][3].y);
                // ===================================================================================================
                glEnd();
                glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }
        }

        glMatrixMode(GL_PROJECTION);
            glPopMatrix();


        // slow down video
        cv::waitKey(12);

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
        glfwSwapBuffers(window);
        
        
        //renderScene(window,frame_pose);
        glfwPollEvents();
    }
    cap.release();

    glfwTerminate();

    return 0;
}