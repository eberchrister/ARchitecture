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

#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/MarkerMovie.MP4"
#define MARKERPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/markers"


#define CAM_MTX (cv::Mat_<float>(3, 3) << 1000, 0.0, 500, 0.0, 1000, 500, 0.0, 0.0, 1.0)
#define CAM_DIST (cv::Mat_<float>(1, 4) << 0, 0, 0, 0)


void setPerspective(float fovY, float aspect, float zNear, float zFar)
{
    float top = tan(fovY * 0.5 * (3.14159265358979323846 / 180.0)) * zNear;
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

    cout << "=========================================" << endl;
    cout << "[CV] Video Metadata: " << endl;
    cout << "\tFrame Count: " << cap.get(cv::CAP_PROP_FRAME_COUNT) << endl;
    cout << "\tFrame Rate: " << cap.get(cv::CAP_PROP_FPS) << endl;
    cout << "\tFrame Dimension: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << endl;
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

    GLFWwindow* window = glfwCreateWindow(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT), "render", NULL, NULL);
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
        
        cv::Point2f p1_bottom, p1_top;
        cv::Point2f p2_bottom, p2_top;
        cv::Point2f p3_bottom, p3_top;
        cv::Point2f p4_bottom, p4_top;
        
        GLfloat scalingFactor = 2.0f;
    
        
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
                // draw here
                    /*
                    glPushMatrix();
                    glTranslatef(-1.0, 0.0, 0.0);
                    glBegin(GL_TRIANGLES);
                    glColor3f(1.0f, 0.0f, 0.0f);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);  // Vertex 1
                   
                    glVertex2f(projectedPoints[1].x/frame_render.cols * scalingFactor, 1 - projectedPoints[1].y/frame_render.rows * scalingFactor);  // Vertex 2
                    
                    
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);  // Vertex 3
                    glEnd();

                    
                    glBegin(GL_TRIANGLES);
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glVertex2f(projectedPoints[0].x/frame_render.cols * scalingFactor, 1 - projectedPoints[0].y/frame_render.rows * scalingFactor);  // Vertex 1
                    glVertex2f(projectedPoints[2].x/frame_render.cols * scalingFactor, 1 - projectedPoints[2].y/frame_render.rows * scalingFactor);  // Vertex 2
                    glVertex2f(projectedPoints[3].x/frame_render.cols * scalingFactor, 1 - projectedPoints[3].y/frame_render.rows * scalingFactor);  // Vertex 3
                    glEnd();
                     
                    glPopMatrix();
                    */
                    
                    //outer
                    p1_bottom = projectedPoints[0];
                    p1_top = projectedPoints[3];
                    
                    //inner
                    //comming soon... @_@
                    
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    // outer
                    p2_bottom = projectedPoints[0];
                    p2_top = projectedPoints[3];
                     
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    // outer
                    p3_bottom = projectedPoints[0];
                    p3_top = projectedPoints[3];
                    break;
                case 12:
                case 13:
                case 14:
                case 15:
                    // outer
                    p4_bottom = projectedPoints[0];
                    p4_top = projectedPoints[3];
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
        

        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p1_top.x/frame_render.cols * scalingFactor, 1 - p1_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_bottom.x/frame_render.cols * scalingFactor, 1 - p1_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_bottom.x/frame_render.cols * scalingFactor, 1 - p2_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_top.x/frame_render.cols * scalingFactor, 1 - p2_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p2_top.x/frame_render.cols * scalingFactor, 1 - p2_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p2_bottom.x/frame_render.cols * scalingFactor, 1 - p2_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_bottom.x/frame_render.cols * scalingFactor, 1 - p3_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_top.x/frame_render.cols * scalingFactor, 1 - p3_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p3_top.x/frame_render.cols * scalingFactor, 1 - p3_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_bottom.x/frame_render.cols * scalingFactor, 1 - p3_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_bottom.x/frame_render.cols * scalingFactor, 1 - p4_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_top.x/frame_render.cols * scalingFactor, 1 - p4_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p4_top.x/frame_render.cols * scalingFactor, 1 - p4_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_bottom.x/frame_render.cols * scalingFactor, 1 - p4_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_bottom.x/frame_render.cols * scalingFactor, 1 - p1_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_top.x/frame_render.cols * scalingFactor, 1 - p1_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        /*
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p3_top.x/frame_render.cols * scalingFactor, 1 - p3_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p3_bottom.x/frame_render.cols * scalingFactor, 1 - p3_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_bottom.x/frame_render.cols * scalingFactor, 1 - p4_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_top.x/frame_render.cols * scalingFactor, 1 - p4_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-1.0, 0.0, 0.0);
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(p4_top.x/frame_render.cols * scalingFactor, 1 - p4_top.y/frame_render.rows * scalingFactor);
        glVertex2f(p4_bottom.x/frame_render.cols * scalingFactor, 1 - p4_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_bottom.x/frame_render.cols * scalingFactor, 1 - p1_bottom.y/frame_render.rows * scalingFactor);
        glVertex2f(p1_top.x/frame_render.cols * scalingFactor, 1 - p1_top.y/frame_render.rows * scalingFactor);
        glEnd();
        glPopMatrix();
        */
        /*
        cv::line(frame_pose, p1_bottom, p1_top, cv::Scalar(0, 0, 255), 2);
        cv::line(frame_pose, p2_bottom, p2_top, cv::Scalar(0, 0, 255), 2);
        cv::line(frame_pose, p1_bottom, p2_bottom, cv::Scalar(0, 0, 255), 2);
        cv::line(frame_pose, p1_top, p2_top, cv::Scalar(0, 0, 255), 2);
       */
        //glPopMatrix(); // wait, what was this for?

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