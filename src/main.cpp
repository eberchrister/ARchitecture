#include "MarkerDetection.h"
#include "ObjectRender.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <filesystem>
#include <iostream>


using namespace std;

#define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/MarkerMovie.MP4"
// #define VIDEOPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/trial_wall.mp4"
#define MARKERPATH "/mnt/c/Users/eberc/Desktop/all/Edu/sem6/AR/ARchitecture/resources/markers"
#define CAM_MTX (cv::Mat_<float>(3, 3) << 1000, 0.0, 500, 0.0, 1000, 500, 0.0, 0.0, 1.0)
#define CAM_DIST (cv::Mat_<float>(1, 4) << 0, 0, 0, 0)


int main(int argc, char const *argv[]){

    // check if debug mode is enabled
    bool debug = false;
    if (argc >= 2){
        if (atoi(argv[1]) == 1){
            debug = true;
            cout << "[prog] Debug mode enabled" << endl;
        }
    }

    /* ======================================== INITIALIZATION ======================================== */
    cv::Mat frame;
    cv::VideoCapture cap("http://192.168.0.7:4747/video", cv::CAP_FFMPEG);

    // check if webcam is detected
    if (!cap.isOpened()){
        cout << "[CV] No Webcam detected, searching for video file" << endl;
        cap.open(VIDEOPATH, cv::CAP_FFMPEG);
        if (!cap.isOpened()){
            cout << "[CV] No video file detected, exiting" << endl;
            exit(0);
        }
        cout << "[CV] Video file detected" << endl;
    }

    // print out video metadata
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
    cout << "[prog] constructing dictionary for markers:" << endl;
    MarkerDict dict = MarkerDetection::constructMarkerDictionary(markerPaths);
    cout << "=========================================" << endl;


    // Initialize GLFW
    if (!glfwInit()){
        fprintf( stderr, "Failed to initialize GLFW\n" );
        cout << "=========================================" << endl;
        return -1;
    }
    cout << "[GLFW] GLFW initialized" << endl;
    cout << "=========================================" << endl;

    GLFWwindow* window = glfwCreateWindow(frame_width, frame_height, "render", NULL, NULL);
    if (!window)
    {
        std::cerr << "[GLFW] Failed to create GLFW window" << std::endl;
        cout << "=========================================" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    /* ======================================== MAIN LOOP STARTS HERE ======================================== */
    while(cap.read(frame)){
        cv::Mat frame_clone = frame.clone();
        cv::Mat frame_pose = frame.clone();
        cv::Mat frame_render = frame.clone();

        // detect all markers in the frame
        vector<MarkerResult> results = MarkerDetection::detectMarker(frame_clone, dict, 0, debug);

        // draw the detected markers on the frame and print their IDs
        for (int i = 0; i < results.size(); i++){
            cv::putText(frame_clone, to_string(results[i].index), results[i].corners[0], cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(32, 32, 255), 2);
            cv::drawContours(frame_clone, vector<vector<cv::Point>>{results[i].corners}, 0, cv::Scalar(0, 255, 32), 1);
        }
        
        // Convert the frame to OpenGL texture format
        cv::flip(frame_render, frame_render, 0);  // Flip vertically
        cv::cvtColor(frame_render, frame_render, cv::COLOR_BGR2RGB);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

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
        
        // store wall marker corners in GL coordinates
        map<string, vector<cv::Point2f>> wallMarkerCorners;

        // Pose Estimation        
        for (MarkerResult& res : results){
            vector<cv::Point2f> projectedPoints = MarkerDetection::poseEstimation(dict.orientations[res.index], res.corners, CAM_MTX, CAM_DIST);

            // store wall marker corners for easy access later
            cv::Point2f zero = projectedPoints[0];
            cv::Point2f one = projectedPoints[1];
            cv::Point2f two = projectedPoints[2];
            cv::Point2f three = projectedPoints[3];
            cv::Point2f four = projectedPoints[4];
            cv::Point2f five = projectedPoints[5];
            cv::Point2f six = projectedPoints[6];
            cv::Point2f seven = projectedPoints[7];

            // draw axis lines on the frame for debugging
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

            // draw axis points on the frame for debugging
            cv::circle(frame_pose, zero, 3, cv::Scalar(75, 25, 230), -1);       // red      - lower top left
            cv::circle(frame_pose, one, 3, cv::Scalar(48, 130, 245), -1);       // orange   - lower top right
            cv::circle(frame_pose, two, 3, cv::Scalar(25, 255, 255), -1);       // yellow   - lower bottom left
            cv::circle(frame_pose, three, 3, cv::Scalar(60, 245, 210), -1);     // lime     - upper top left
            cv::circle(frame_pose, four, 3, cv::Scalar(75, 180, 60), -1);       // green    - lower bottom right
            cv::circle(frame_pose, five, 3, cv::Scalar(240, 240, 70), -1);      // cyan     - upper bottom right
            cv::circle(frame_pose, six, 3, cv::Scalar(200, 130, 0), -1);        // blue     - upper top left
            cv::circle(frame_pose, seven, 3, cv::Scalar(180, 30, 145), -1);     // purple   - upper bottom left

            cv::circle(frame_pose, ObjectRender::vectorAddRelative(cv::Point2f(one.x, one.y), cv::Point2f(two.x, two.y), zero, 0.5,0.5), 3, cv::Scalar(48, 130, 245), -1);

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
            // convert projected points to GL coordinates
            vector<cv::Point2f> projectedGLPoints = ObjectRender::convertToGLCoords(projectedPoints, frame_width, frame_height);
            
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            // draw the isometric walls on the four markers
            if (0 <= res.index && res.index <= 3){
                 wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("topLeft", projectedGLPoints));
            } else if (4 <= res.index && res.index <= 7){
                 wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("topRight", projectedGLPoints));
            } else if (8 <= res.index && res.index <= 11){
                 wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("bottomRight", projectedGLPoints));
            } else if (12 <= res.index && res.index <= 15){
                 wallMarkerCorners.insert(pair<string, vector<cv::Point2f>>("bottomLeft", projectedGLPoints));
            } else if (16 <= res.index && res.index <= 19){
                glPushMatrix();
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // table floor
                glBegin(GL_QUADS);
                glColor4f(1., 0., 0., 0.1);
                glVertex2f(projectedGLPoints[0].x, -projectedGLPoints[0].y);
                glVertex2f(projectedGLPoints[1].x, -projectedGLPoints[1].y);
                glVertex2f(projectedGLPoints[4].x, -projectedGLPoints[4].y);
                glVertex2f(projectedGLPoints[2].x, -projectedGLPoints[2].y);
                glEnd();

                cv::Point2f zeroLeg_0 = projectedGLPoints[0];
                cv::Point2f zeroLeg_1 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[1], projectedGLPoints[0], 0, (float)1/6);
                cv::Point2f zeroLeg_2 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[2], projectedGLPoints[0], 0, (float)1/6);
                cv::Point2f zeroLeg_4 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[4], projectedGLPoints[0], 0, (float)1/6);
                cv::Point2f zeroLeg_3 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[3], projectedGLPoints[0], 0, (float)5/6);
                cv::Point2f zeroLeg_5 = ObjectRender::vectorAddRelative(zeroLeg_4, zeroLeg_3, projectedGLPoints[0], 1, (float)1);
                cv::Point2f zeroLeg_6 = ObjectRender::vectorAddRelative(zeroLeg_1, zeroLeg_3, projectedGLPoints[0], 1, (float)1);
                cv::Point2f zeroLeg_7 = ObjectRender::vectorAddRelative(zeroLeg_2, zeroLeg_3, projectedGLPoints[0], 1, (float)1);

                // table leg base
                glBegin(GL_QUADS);
                glColor4f(0., 1., 0., 1);
                glVertex2f(zeroLeg_1.x, -zeroLeg_1.y);
                glVertex2f(zeroLeg_4.x, -zeroLeg_4.y);
                glVertex2f(zeroLeg_2.x, -zeroLeg_2.y);
                glVertex2f(zeroLeg_0.x, -zeroLeg_0.y);
                glEnd();

                glBegin(GL_QUADS);
                // table leg block back
                glColor3f(0.584,0.749,0.863);
                glVertex2f(zeroLeg_2.x, -zeroLeg_2.y);
                glVertex2f(zeroLeg_7.x, -zeroLeg_7.y);
                glVertex2f(zeroLeg_3.x, -zeroLeg_3.y);
                glVertex2f(zeroLeg_0.x, -zeroLeg_0.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.5);
                    glVertex2f(zeroLeg_2.x, -zeroLeg_2.y);
                    glVertex2f(zeroLeg_7.x, -zeroLeg_7.y);
                    glVertex2f(zeroLeg_3.x, -zeroLeg_3.y);
                    glVertex2f(zeroLeg_0.x, -zeroLeg_0.y);

                // table leg block right
                glColor3f(0.584,0.749,0.863);
                glVertex2f(zeroLeg_0.x, -zeroLeg_0.y);
                glVertex2f(zeroLeg_3.x, -zeroLeg_3.y);
                glVertex2f(zeroLeg_6.x, -zeroLeg_6.y);
                glVertex2f(zeroLeg_1.x, -zeroLeg_1.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.5);
                    glVertex2f(zeroLeg_0.x, -zeroLeg_0.y);
                    glVertex2f(zeroLeg_3.x, -zeroLeg_3.y);
                    glVertex2f(zeroLeg_6.x, -zeroLeg_6.y);
                    glVertex2f(zeroLeg_1.x, -zeroLeg_1.y);

                // table leg block front
                glColor3f(0.584,0.749,0.863);
                glVertex2f(zeroLeg_1.x, -zeroLeg_1.y);
                glVertex2f(zeroLeg_6.x, -zeroLeg_6.y);
                glVertex2f(zeroLeg_5.x, -zeroLeg_5.y);
                glVertex2f(zeroLeg_4.x, -zeroLeg_4.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.3);
                    glVertex2f(zeroLeg_1.x, -zeroLeg_1.y);
                    glVertex2f(zeroLeg_6.x, -zeroLeg_6.y);
                    glVertex2f(zeroLeg_5.x, -zeroLeg_5.y);
                    glVertex2f(zeroLeg_4.x, -zeroLeg_4.y);

                // table leg block left
                glColor3f(0.584,0.749,0.863);
                glVertex2f(zeroLeg_2.x, -zeroLeg_2.y);
                glVertex2f(zeroLeg_7.x, -zeroLeg_7.y);
                glVertex2f(zeroLeg_5.x, -zeroLeg_5.y);
                glVertex2f(zeroLeg_4.x, -zeroLeg_4.y);
                glEnd();

                cv::Point2f oneLeg_1 = projectedGLPoints[1];
                cv::Point2f oneLeg_0 = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectedGLPoints[0], projectedGLPoints[1], 0, (float)1/6);
                cv::Point2f oneLeg_4 = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectedGLPoints[4], projectedGLPoints[1], 0, (float)1/6);
                cv::Point2f oneLeg_2 = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectedGLPoints[2], projectedGLPoints[1], 0, (float)1/6);

                cv::Point2f oneLeg_3 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[6], projectedGLPoints[1], 0, (float)5/6);
                cv::Point2f oneLeg_5 = ObjectRender::vectorAddRelative(oneLeg_4, oneLeg_3, projectedGLPoints[1], 1, (float)1);
                cv::Point2f oneLeg_6 = ObjectRender::vectorAddRelative(oneLeg_1, oneLeg_3, projectedGLPoints[1], 1, (float)1);
                cv::Point2f oneLeg_7 = ObjectRender::vectorAddRelative(oneLeg_2, oneLeg_3, projectedGLPoints[1], 1, (float)1);

                // table leg base
                glBegin(GL_QUADS);
                glColor4f(0., 1., 0., 1);
                glVertex2f(oneLeg_1.x, -oneLeg_1.y);
                glVertex2f(oneLeg_4.x, -oneLeg_4.y);
                glVertex2f(oneLeg_2.x, -oneLeg_2.y);
                glVertex2f(oneLeg_0.x, -oneLeg_0.y);
                glEnd();

                // table leg block back
                glBegin(GL_QUADS);
                glColor3f(0.584,0.749,0.863);
                glVertex2f(oneLeg_2.x, -oneLeg_2.y);
                glVertex2f(oneLeg_7.x, -oneLeg_7.y);
                glVertex2f(oneLeg_3.x, -oneLeg_3.y);
                glVertex2f(oneLeg_0.x, -oneLeg_0.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.5);
                    glVertex2f(oneLeg_2.x, -oneLeg_2.y);
                    glVertex2f(oneLeg_7.x, -oneLeg_7.y);
                    glVertex2f(oneLeg_3.x, -oneLeg_3.y);
                    glVertex2f(oneLeg_0.x, -oneLeg_0.y);

                // table leg block right
                glColor3f(0.584,0.749,0.863);
                glVertex2f(oneLeg_0.x, -oneLeg_0.y);
                glVertex2f(oneLeg_3.x, -oneLeg_3.y);
                glVertex2f(oneLeg_6.x, -oneLeg_6.y);
                glVertex2f(oneLeg_1.x, -oneLeg_1.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.3);
                    glVertex2f(oneLeg_0.x, -oneLeg_0.y);
                    glVertex2f(oneLeg_3.x, -oneLeg_3.y);
                    glVertex2f(oneLeg_6.x, -oneLeg_6.y);
                    glVertex2f(oneLeg_1.x, -oneLeg_1.y);

                // table leg block front
                glColor3f(0.584,0.749,0.863);
                glVertex2f(oneLeg_1.x, -oneLeg_1.y);
                glVertex2f(oneLeg_6.x, -oneLeg_6.y);
                glVertex2f(oneLeg_5.x, -oneLeg_5.y);
                glVertex2f(oneLeg_4.x, -oneLeg_4.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.3);
                    glVertex2f(oneLeg_1.x, -oneLeg_1.y);
                    glVertex2f(oneLeg_6.x, -oneLeg_6.y);
                    glVertex2f(oneLeg_5.x, -oneLeg_5.y);
                    glVertex2f(oneLeg_4.x, -oneLeg_4.y);

                // table leg block left
                glColor3f(0.584,0.749,0.863);
                glVertex2f(oneLeg_2.x, -oneLeg_2.y);
                glVertex2f(oneLeg_7.x, -oneLeg_7.y);
                glVertex2f(oneLeg_5.x, -oneLeg_5.y);
                glVertex2f(oneLeg_4.x, -oneLeg_4.y);
                    // table leg block shadow
                    glColor4f(0, 0, 0, 0.3);
                    glVertex2f(oneLeg_2.x, -oneLeg_2.y);
                    glVertex2f(oneLeg_7.x, -oneLeg_7.y);
                    glVertex2f(oneLeg_5.x, -oneLeg_5.y);
                    glVertex2f(oneLeg_4.x, -oneLeg_4.y);

                glEnd();






                glDisable(GL_BLEND);
                glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            } else if (20 <= res.index && res.index <= 23){
            }

            // once all four markers are detected, draw the walls
            if (wallMarkerCorners.size() == 4) {
                vector<string> sortedWallName = ObjectRender::sortWallMarker(wallMarkerCorners, frame_height);
                glPushMatrix();
                glDisable(GL_TEXTURE_2D);
                // beige. floor = 0.851,0.725,0.608, left = 1.,0.941,0.859, right = 0.933,0.851,0.769, ceiling = 0.98,0.941,0.902
                vector<tuple<GLfloat, GLfloat,GLfloat>> wallColors{make_tuple(0.851,0.725,0.608), make_tuple(1.,0.941,0.859), make_tuple(0.933,0.851,0.769), make_tuple(0.98,0.941,0.902)};
                ObjectRender::drawWalls(wallMarkerCorners, sortedWallName, wallColors, true, true , 0.0f);
                glEnable(GL_TEXTURE_2D);
                glPopMatrix();
            }

        }
        glMatrixMode(GL_PROJECTION);
            glPopMatrix();

        // cv::namedWindow("Original", cv::WINDOW_NORMAL);
        // cv::imshow("Original", frame);
        // if (cv::waitKey(12) == 27){
            // break;
        // }

        cv::namedWindow("ID", cv::WINDOW_NORMAL);
        cv::imshow("ID", frame_clone);
        cv::namedWindow("Pose", cv::WINDOW_NORMAL);
        cv::imshow("Pose", frame_pose);

        if (cv::waitKey(frame_rate) == 27){
            break;
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cap.release();

    glfwTerminate();

    return 0;
}