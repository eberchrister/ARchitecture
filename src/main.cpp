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
//#include <GL/glu.h>

using namespace std;

#define VIDEOPATH "MarkerMovie.MP4"
#define MARKERPATH "/Users/jessicasumargo/Documents/AR_Final/ARchitecture-main 3/resources/markers"


#define CAM_MTX (cv::Mat_<float>(3, 3) << 1000, 0.0, 500, 0.0, 1000, 500, 0.0, 0.0, 1.0)
#define CAM_DIST (cv::Mat_<float>(1, 4) << 0, 0, 0, 0)


void setPerspective(float fovY, float aspect, float zNear, float zFar)
{
    float top = tan(fovY * 0.5 * (CV_PI / 180.0)) * zNear;
    float bottom = -top;
    float left = aspect * bottom;
    float right = aspect * top;

    glFrustum(left, right, bottom, top, zNear, zFar);
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
    cv::VideoCapture cap(0);

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
    for (const auto & entry : std::__fs::filesystem::directory_iterator(MARKERPATH)){
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

        // Set up the camera
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        setPerspective(45.0, 1.0, 1.0, 100.0);
        //glOrtho(0.0, frame_render.cols, 0.0, frame_render.rows, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //glTranslatef(0.0, 0.5, -10.0);
        glTranslatef(0.0, 0.0, -2.425); //correct value
        
        
        // Convert the frame to OpenGL texture format
        cv::flip(frame_render, frame_render, 0);  // Flip vertically
        cv::cvtColor(frame_render, frame_render, cv::COLOR_BGR2RGB);

        // Set up the texture mapping
        glEnable(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_render.cols, frame_render.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_render.data);

        // Draw a rectangle with the webcam frame as the background
        glColor3f(1.0, 1.0, 1.0);
        /*glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-2.5, -2.5, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f(2.5, -2.5, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f(2.5, 2.5, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-2.5, 2.5, 0.0);
        glEnd();*/
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-1, -1, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f(1, -1, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f(1, 1, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-1, 1, 0.0);
        glEnd();
        
        /*
        cv::Point2f p1_outer_bottom, p1_outer_top, p1_inner_bottom, p1_inner_top, p1_cover_a, p1_cover_b, p1_cover_c;
        cv::Point2f p2_outer_bottom, p2_outer_top, p2_inner_bottom, p2_inner_top, p2_cover_a, p2_cover_b, p2_cover_c;
        cv::Point2f p3_outer_bottom, p3_outer_top, p3_inner_bottom, p3_inner_top, p3_cover_a, p3_cover_b, p3_cover_c;
        cv::Point2f p4_outer_bottom, p4_outer_top, p4_inner_bottom, p4_inner_top, p4_cover_a, p4_cover_b, p4_cover_c;
        */
        
        // for storing some projectedPoints that will be needed to connect the pillars
        cv::Point2f p1_1, p1_6, p1_4, p1_5, p1_2, p1_7;
        cv::Point2f p2_1, p2_6, p2_4, p2_5, p2_2, p2_7;
        cv::Point2f p3_1, p3_6, p3_4, p3_5, p3_2, p3_7;
        cv::Point2f p4_1, p4_6, p4_4, p4_5, p4_2, p4_7;
        
        
        GLfloat scalingFactor = 2.0f;
        GLfloat eH = 0.3f;
        
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
            
            switch(res.index) {
                case 0:
                case 1:
                case 2:
                case 3:
                    /*
                    //outer
                    p1_outer_bottom = projectedPoints[0];
                    p1_outer_top = projectedPoints[3];
                    //inner
                    p1_inner_bottom = projectedPoints[4];
                    p1_inner_top = projectedPoints[5];
                    //closing the top
                    p1_cover_a = projectedPoints[3];
                    p1_cover_b = projectedPoints[6];
                    p1_cover_c = projectedPoints[7];
                     */
                    
                    //draw pillar
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 0.0f, 1.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[1].x/frame_render.cols * scalingFactor, 1 - projectedPoints[1].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[2].x/frame_render.cols * scalingFactor, 1 - projectedPoints[2].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[5].x/frame_render.cols * scalingFactor, 1 - projectedPoints[5].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    
                    // saving some coordinates that will be used later to connect the pillars
                    p1_1 = projectedPoints[1];
                    p1_6 = projectedPoints[6];
                    p1_4 = projectedPoints[4];
                    p1_5 = projectedPoints[5];
                    p1_2 = projectedPoints[2];
                    p1_7 = projectedPoints[7];
                    
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    /*
                    // outer
                    p2_outer_bottom = projectedPoints[0];
                    p2_outer_top = projectedPoints[3];
                    //inner
                    p2_inner_bottom = projectedPoints[4];
                    p2_inner_top = projectedPoints[5];
                    //closing the top
                    p2_cover_a = projectedPoints[3];
                    p2_cover_b = projectedPoints[6];
                    p2_cover_c = projectedPoints[7];
                     */
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 0.0f, 1.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[1].x/frame_render.cols * scalingFactor, 1 - projectedPoints[1].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[2].x/frame_render.cols * scalingFactor, 1 - projectedPoints[2].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[5].x/frame_render.cols * scalingFactor, 1 - projectedPoints[5].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    // saving some coordinates that will be used later to connect the pillars
                    p2_1 = projectedPoints[1];
                    p2_6 = projectedPoints[6];
                    p2_4 = projectedPoints[4];
                    p2_5 = projectedPoints[5];
                    p2_2 = projectedPoints[2];
                    p2_7 = projectedPoints[7];
                    
                    
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    /*
                    // outer
                    p3_outer_bottom = projectedPoints[0];
                    p3_outer_top = projectedPoints[3];
                    //inner
                    p3_inner_bottom = projectedPoints[4];
                    p3_inner_top = projectedPoints[5];
                    //closing the top
                    p3_cover_a = projectedPoints[3];
                    p3_cover_b = projectedPoints[6];
                    p3_cover_c = projectedPoints[7];
                    */
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 0.0f, 1.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[1].x/frame_render.cols * scalingFactor, 1 - projectedPoints[1].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[2].x/frame_render.cols * scalingFactor, 1 - projectedPoints[2].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[5].x/frame_render.cols * scalingFactor, 1 - projectedPoints[5].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    // saving some coordinates that will be used later to connect the pillars
                    p3_1 = projectedPoints[1];
                    p3_6 = projectedPoints[6];
                    p3_4 = projectedPoints[4];
                    p3_5 = projectedPoints[5];
                    p3_2 = projectedPoints[2];
                    p3_7 = projectedPoints[7];
                    
                    
                    break;
                case 12:
                case 13:
                case 14:
                case 15:
                    /*
                    // outer
                    p4_outer_bottom = projectedPoints[0];
                    p4_outer_top = projectedPoints[3];
                    //inner
                    p4_inner_bottom = projectedPoints[4];
                    p4_inner_top = projectedPoints[5];
                    //closing the top
                    p4_cover_a = projectedPoints[3];
                    p4_cover_b = projectedPoints[6];
                    p4_cover_c = projectedPoints[7];
                    */
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 0.0f, 1.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[1].x/frame_render.cols * scalingFactor, 1 - projectedPoints[1].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[2].x/frame_render.cols * scalingFactor, 1 - projectedPoints[2].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_QUADS);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[7].x/frame_render.cols * scalingFactor, 1 - projectedPoints[7].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[5].x/frame_render.cols * scalingFactor, 1 - projectedPoints[5].y/frame_render.rows * scalingFactor);
                    glVertex2f(projectedPoints[6].x/frame_render.cols * scalingFactor, 1 - projectedPoints[6].y/frame_render.rows * scalingFactor);
                    glEnd();
                    glPopMatrix();
                    
                    // saving some coordinates that will be used later to connect the pillars
                    p4_1 = projectedPoints[1];
                    p4_6 = projectedPoints[6];
                    p4_4 = projectedPoints[4];
                    p4_5 = projectedPoints[5];
                    p4_2 = projectedPoints[2];
                    p4_7 = projectedPoints[7];
                    
                    
                    
                    break;
                case 16:
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
            
        }
        
        
        //connecting the pillars, outer part
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.5f, 0.0f, 0.5f);
        glVertex2f(p1_6.x/frame_render.cols * scalingFactor, 1 - p1_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_1.x/frame_render.cols * scalingFactor, 1 - p1_1.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_2.x/frame_render.cols * scalingFactor, 1 - p2_2.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_7.x/frame_render.cols * scalingFactor, 1 - p2_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.5f, 0.0f, 0.5f);
        glVertex2f(p2_6.x/frame_render.cols * scalingFactor, 1 - p2_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_1.x/frame_render.cols * scalingFactor, 1 - p2_1.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_2.x/frame_render.cols * scalingFactor, 1 - p3_2.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_7.x/frame_render.cols * scalingFactor, 1 - p3_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.5f, 0.0f, 0.5f);
        glVertex2f(p3_6.x/frame_render.cols * scalingFactor, 1 - p3_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_1.x/frame_render.cols * scalingFactor, 1 - p3_1.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_2.x/frame_render.cols * scalingFactor, 1 - p4_2.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_7.x/frame_render.cols * scalingFactor, 1 - p4_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.5f, 0.0f, 0.5f);
        glVertex2f(p4_6.x/frame_render.cols * scalingFactor, 1 - p4_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_1.x/frame_render.cols * scalingFactor, 1 - p4_1.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_2.x/frame_render.cols * scalingFactor, 1 - p1_2.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_7.x/frame_render.cols * scalingFactor, 1 - p1_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        
        
        
        //connecting the pillars, top part
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f(p1_6.x/frame_render.cols * scalingFactor, 1 - p1_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_5.x/frame_render.cols * scalingFactor, 1 - p1_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_5.x/frame_render.cols * scalingFactor, 1 - p2_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_7.x/frame_render.cols * scalingFactor, 1 - p2_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f(p2_6.x/frame_render.cols * scalingFactor, 1 - p2_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_5.x/frame_render.cols * scalingFactor, 1 - p2_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_5.x/frame_render.cols * scalingFactor, 1 - p3_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_7.x/frame_render.cols * scalingFactor, 1 - p3_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f(p3_6.x/frame_render.cols * scalingFactor, 1 - p3_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_5.x/frame_render.cols * scalingFactor, 1 - p3_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_5.x/frame_render.cols * scalingFactor, 1 - p4_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_7.x/frame_render.cols * scalingFactor, 1 - p4_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f(p4_6.x/frame_render.cols * scalingFactor, 1 - p4_6.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_5.x/frame_render.cols * scalingFactor, 1 - p4_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_5.x/frame_render.cols * scalingFactor, 1 - p1_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_7.x/frame_render.cols * scalingFactor, 1 - p1_7.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        
        //connecting the pillars, inner part
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex2f(p1_5.x/frame_render.cols * scalingFactor, 1 - p1_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_4.x/frame_render.cols * scalingFactor, 1 - p1_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_4.x/frame_render.cols * scalingFactor, 1 - p2_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_5.x/frame_render.cols * scalingFactor, 1 - p2_5.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex2f(p2_5.x/frame_render.cols * scalingFactor, 1 - p2_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_4.x/frame_render.cols * scalingFactor, 1 - p2_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_4.x/frame_render.cols * scalingFactor, 1 - p3_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_5.x/frame_render.cols * scalingFactor, 1 - p3_5.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex2f(p3_5.x/frame_render.cols * scalingFactor, 1 - p3_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_4.x/frame_render.cols * scalingFactor, 1 - p3_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_4.x/frame_render.cols * scalingFactor, 1 - p4_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_5.x/frame_render.cols * scalingFactor, 1 - p4_5.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex2f(p4_5.x/frame_render.cols * scalingFactor, 1 - p4_5.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_4.x/frame_render.cols * scalingFactor, 1 - p4_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_4.x/frame_render.cols * scalingFactor, 1 - p1_4.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_5.x/frame_render.cols * scalingFactor, 1 - p1_5.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        
        
        /*
        //extra height
        GLfloat eH = 0.3f;
        
        //drawing outer walls
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p1_outer_top.x/frame_render.cols * scalingFactor, 1 - p1_outer_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p1_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p1_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p2_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_outer_top.x/frame_render.cols * scalingFactor, 1 - p2_outer_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p2_outer_top.x/frame_render.cols * scalingFactor, 1 - p2_outer_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p2_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p2_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p3_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_outer_top.x/frame_render.cols * scalingFactor, 1 - p3_outer_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p3_outer_top.x/frame_render.cols * scalingFactor, 1 - p3_outer_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p3_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p3_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p4_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_outer_top.x/frame_render.cols * scalingFactor, 1 - p4_outer_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p4_outer_top.x/frame_render.cols * scalingFactor, 1 - p4_outer_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p4_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p4_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_outer_bottom.x/frame_render.cols * scalingFactor, 1 - p1_outer_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_outer_top.x/frame_render.cols * scalingFactor, 1 - p1_outer_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
         
   
        
        // drawing inner walls
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(p1_inner_top.x/frame_render.cols * scalingFactor, 1 - p1_inner_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p1_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p1_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p2_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_inner_top.x/frame_render.cols * scalingFactor, 1 - p2_inner_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(p2_inner_top.x/frame_render.cols * scalingFactor, 1 - p2_inner_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p2_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p2_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p3_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_inner_top.x/frame_render.cols * scalingFactor, 1 - p3_inner_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(p3_inner_top.x/frame_render.cols * scalingFactor, 1 - p3_inner_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p3_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p3_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p4_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_inner_top.x/frame_render.cols * scalingFactor, 1 - p4_inner_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(p4_inner_top.x/frame_render.cols * scalingFactor, 1 - p4_inner_top.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p4_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p4_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_inner_bottom.x/frame_render.cols * scalingFactor, 1 - p1_inner_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_inner_top.x/frame_render.cols * scalingFactor, 1 - p1_inner_top.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        
        //closing the top
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(p1_cover_a.x/frame_render.cols * scalingFactor, 1 - p1_cover_a.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p1_cover_c.x/frame_render.cols * scalingFactor, 1 - p1_cover_c.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p2_cover_b.x/frame_render.cols * scalingFactor, 1 - p2_cover_b.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p2_cover_a.x/frame_render.cols * scalingFactor, 1 - p2_cover_a.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(p2_cover_a.x/frame_render.cols * scalingFactor, 1 - p2_cover_a.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p2_cover_c.x/frame_render.cols * scalingFactor, 1 - p2_cover_c.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p3_cover_b.x/frame_render.cols * scalingFactor, 1 - p3_cover_b.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p3_cover_a.x/frame_render.cols * scalingFactor, 1 - p3_cover_a.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(p3_cover_a.x/frame_render.cols * scalingFactor, 1 - p3_cover_a.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p3_cover_c.x/frame_render.cols * scalingFactor, 1 - p3_cover_c.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p4_cover_b.x/frame_render.cols * scalingFactor, 1 - p4_cover_b.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p4_cover_a.x/frame_render.cols * scalingFactor, 1 - p4_cover_a.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(p4_cover_a.x/frame_render.cols * scalingFactor, 1 - p4_cover_a.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p4_cover_c.x/frame_render.cols * scalingFactor, 1 - p4_cover_c.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p1_cover_b.x/frame_render.cols * scalingFactor, 1 - p1_cover_b.y/frame_render.rows * scalingFactor + eH);
        glVertex2f(p1_cover_a.x/frame_render.cols * scalingFactor, 1 - p1_cover_a.y/frame_render.rows * scalingFactor + eH);
        glEnd();
        glPopMatrix();
        
*/
        
        //glPopMatrix(); // wait, what was this for?
        /*
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
         */
        cv::namedWindow("Pose", cv::WINDOW_NORMAL);
        cv::imshow("Pose", frame_pose);
        if (cv::waitKey(12) == 27){
            break;
        }
         
        glDisable(GL_TEXTURE_2D);

        glFlush();
        glfwSwapBuffers(window);
        
        
        //renderScene(window,frame_pose);
        glfwPollEvents();

    }
    cap.release();

    glfwTerminate();

    return 0;
}
