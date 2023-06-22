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
        glTranslatef(0.0, 0.0, -5.0); //correct value
        
        
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
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-2.5, -2.5, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f(2.5, -2.5, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f(2.5, 2.5, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-2.5, 2.5, 0.0);
        glEnd();

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
            GLfloat scalingFactor = 2.0f;
        
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