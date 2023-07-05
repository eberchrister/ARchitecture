#include "MarkerDetection.h"
#include "ObjectRender.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <filesystem>
#include <iostream>


using namespace std;

#define VIDEOPATH "C:\\Users\\nobis\\augmented\\ARchitecture-Project\\ARchitecture-Project\\resources\\MarkerMovie.MP4"
#define MARKERPATH "C:\\Users\\nobis\\augmented\\ARchitecture-Project\\ARchitecture-Project\\resources\\markers"
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
    cv::VideoCapture cap(0);

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
        markerPaths.push_back(entry.path().u8string());
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
        glOrtho(-1, 1, -1, 1, -1, 10);	
        
        // store wall marker corners in GL coordinates
        map<string, vector<cv::Point2f>> wallMarkerCorners;
        
        vector<cv::Point2f> GLPoints_Table1x1;
        vector<cv::Point2f> GLPoints_Table1x2;
        vector<cv::Point2f> GLPoints_BasicChair;
        vector<cv::Point2f> GLPoints_Bed;
        vector<cv::Point2f> GLPoints_SmallSofa;
        vector<cv::Point2f> GLPoints_LongSofa;
        vector<cv::Point2f> GLPoints_TableForSofa;
        vector<cv::Point2f> GLPoints_DiningTable;
        vector<cv::Point2f> GLPoints_DiningChair;
        vector<cv::Point2f> GLPoints_TV;
        vector<cv::Point2f> GLPoints_Carpet;
        vector<cv::Point2f> GLPoints_Bookshelf;
        
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
            }

            switch (res.index) {
                    /*
                case 16 ... 19:
                    GLPoints_Table1x1 = projectedGLPoints;
                    break;
                case 20 ... 23:
                    GLPoints_Table1x2 = projectedGLPoints;
                    break;
                case 24 ... 27:
                    GLPoints_BasicChair = projectedGLPoints;
                    break;
                case 28 ... 31:
                    GLPoints_Bed = projectedGLPoints;
                    break;
                case 32 ... 35:
                    GLPoints_SmallSofa = projectedGLPoints;
                    break;
                case 36 ... 39:
                    GLPoints_LongSofa = projectedGLPoints;
                    break;
                case 40 ... 43:
                    GLPoints_TableForSofa = projectedGLPoints;
                    break;
                case 44 ... 47:
                    GLPoints_DiningTable = projectedGLPoints;
                    break;
                case 48 ... 51:
                    GLPoints_DiningChair = projectedGLPoints;
                    break;
                case 52 ... 55:
                    GLPoints_TV = projectedGLPoints;
                    break;
                case 56 ... 59:
                    GLPoints_Carpet = projectedGLPoints;
                    break;
                case 60 ... 63:
                    GLPoints_Bookshelf = projectedGLPoints;
                    break;
                     */
            case 16: case 17:case 18:case 19:
                GLPoints_TV = projectedGLPoints;
                break;
            case 20:case 21:case 22:case 23:
                //GLPoints_Carpet = projectedGLPoints;
                break;
            case 24:case 25:case 26:case 27:
                //GLPoints_Bookshelf = projectedGLPoints;
                break;
            case 28:case 29:case 30:case 31:
                //GLPoints_Bed = projectedGLPoints;
                break;
            case 32:case 33:case 34:case 35:
                //GLPoints_SmallSofa = projectedGLPoints;
                break;
            case 36:case 37:case 38:case 39:
                //GLPoints_LongSofa = projectedGLPoints;
                break;
            case 40:case 41:case 42:case 43:
                //GLPoints_TableForSofa = projectedGLPoints;
                break;
            case 44:case 45:case 46:case 47:
                //GLPoints_DiningTable = projectedGLPoints;
                break;
            case 48:case 49:case 50:case 51:
                //GLPoints_DiningChair = projectedGLPoints;
                break;
            }


        }
        // once all four markers are detected, draw the walls
        if (wallMarkerCorners.size() == 4) {
            vector<string> sortedWallName = ObjectRender::sortWallMarker(wallMarkerCorners, frame_height);
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            // beige. 
            vector<GLfloat> floorColor{0.851,0.725,0.608};
            vector<GLfloat> leftColor{1.,0.941,0.859};
            vector<GLfloat> rightColor{0.933,0.851,0.769};
            vector<GLfloat> ceilingColor{0.98,0.941,0.902};
            vector<vector<GLfloat>> wallColors{floorColor, leftColor, rightColor, ceilingColor};
            ObjectRender::drawWalls(wallMarkerCorners, sortedWallName, wallColors, true, true , 1.0f);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        // baby blue
        vector<GLfloat> legColorLeft{0.663,0.847,0.914};
        vector<GLfloat> legColorRight{0.529,0.675,0.729};
        vector<GLfloat> legColorDark{0.396,0.506,0.545};
        vector<vector<GLfloat>> legColors{legColorLeft, legColorRight, legColorDark};
        // orange salmom
        vector<GLfloat> topColorTop{0.937,0.808,0.761};
        vector<GLfloat> topColorLeft{0.914,0.729,0.663};
        vector<GLfloat> topColorRight{0.82,0.655,0.596};
        vector<GLfloat> topColorDark{0.729,0.58,0.529};
        vector<vector<GLfloat>> topColors{topColorTop, topColorLeft, topColorRight, topColorDark};
        
        if (GLPoints_Table1x1.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawTable1x1(GLPoints_Table1x1, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_Table1x2.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawTable1x2(GLPoints_Table1x2, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_BasicChair.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawBasicChair(GLPoints_BasicChair, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_Bed.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawBed(GLPoints_Bed, legColors, topColors, 2.0, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_SmallSofa.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawSmallSofa(GLPoints_SmallSofa, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_LongSofa.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawLongSofa(GLPoints_LongSofa, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_TableForSofa.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawTableForSofa(GLPoints_TableForSofa, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_DiningTable.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawDiningTable(GLPoints_DiningTable, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_DiningChair.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawDiningChair(GLPoints_DiningChair, legColors, topColors, 0.4, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_TV.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawTV(GLPoints_TV, legColors, topColors, 2.0, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_Carpet.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawCarpet(GLPoints_Carpet, legColors, topColors, 2.0, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
        }
        
        if (GLPoints_Bookshelf.size() != 0){
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            ObjectRender::drawBookshelf(GLPoints_Bookshelf, legColors, topColors, 2.0, false);
            glEnable(GL_TEXTURE_2D);
            glPopMatrix();
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
