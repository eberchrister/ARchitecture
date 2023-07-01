#include "ObjectRender.h"

using namespace std;

cv::Point2f ObjectRender::vectorAddRelative(cv::Point2f a, cv::Point2f b, cv::Point2f origin, float scaleA, float scaleB){
    // translate the points with respect to the relative origin
    float a_x = a.x - origin.x;
    float a_y = a.y - origin.y;
    a_y = -a_y; // flip the y axis, since the y axis is flipped in OpenCV coordinate space

    float b_x = b.x - origin.x;
    float b_y = b.y - origin.y;
    b_y = -b_y;

    // add the vectors together with scaling
    cv::Point2f c_trans = cv::Point2f(scaleA*a_x + scaleB*b_x, scaleA*a_y + scaleB*b_y);
    cv::Point2f c = cv::Point2f(c_trans.x + origin.x, -c_trans.y + origin.y);
    return c;
}

vector<cv::Point2f> ObjectRender::convertToGLCoords(vector<cv::Point2f> projectedPoints, int frame_width, int frame_height){
    vector<cv::Point2f> points2D;
    // convert the points from OpenCV coordinate space to OpenGL coordinate space, x and y are in [-1, 1]
    for (const auto & point : projectedPoints){
        float x = point.x/frame_width * 2.0f - 1.0f;
        float y = point.y/frame_height * 2.0f - 1.0f;
        points2D.push_back(cv::Point2f(x, y));
    }
    return points2D;
}

vector<string> ObjectRender::sortWallMarker(map<string, vector<cv::Point2f>> wallMarkers, int frame_height){
    vector<string> sortedMarkers;

    // calculate metric for sorting based on the Y value of the marker
    float topLeftA = frame_height-wallMarkers["topLeft"][0].y;
    float topRightA = frame_height-wallMarkers["topRight"][0].y;
    float bottomRightA = frame_height-wallMarkers["bottomRight"][0].y;
    float bottomLeftA = frame_height-wallMarkers["bottomLeft"][0].y;

    // sort the markers in ascending order (closest --> farthest)
    vector<pair<string, float>> markerAreas = {{"topLeft", topLeftA}, {"topRight", topRightA}, {"bottomRight", bottomRightA}, {"bottomLeft", bottomLeftA}};
    sort(markerAreas.begin(), markerAreas.end(), [](const pair<string, float> &a, const pair<string, float> &b){
        return a.second < b.second;
    });

    for (const auto & marker : markerAreas){
        sortedMarkers.push_back(marker.first);
    }

    return sortedMarkers;
}

// draw two walls, connecting the farthest and its neighbors. color is a triple of (r, g, b) -->  [0]: floor, [1] left, [2]: right, [3]: roof
void ObjectRender::drawWalls(map<string, vector<cv::Point2f>> wallMarkerCorners, vector<string> sortedKeyClosest, vector<vector<GLfloat>> colors, bool outline = true, bool floor = true, float extraHeight = 0.0){
    vector<string> sortedKeyClosest1 = sortedKeyClosest;

    string furthest = sortedKeyClosest[3];
    string neighbor1 = sortedKeyClosest[2];
    string neighbor2 = sortedKeyClosest[1];
    string closest = sortedKeyClosest[0];

    // control the thickness of the walls
    wallMarkerCorners[neighbor1][4] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][4], wallMarkerCorners[neighbor1][0], wallMarkerCorners[neighbor1][0], 0.25, 0.25);
    wallMarkerCorners[neighbor2][4] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][4], wallMarkerCorners[neighbor2][0], wallMarkerCorners[neighbor2][0], 0.25, 0.25);
    wallMarkerCorners[furthest][4] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][4], wallMarkerCorners[furthest][0], wallMarkerCorners[furthest][0], 0.25, 0.25);
    wallMarkerCorners[neighbor1][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][4], wallMarkerCorners[neighbor1][0], 1, 1);
    wallMarkerCorners[neighbor2][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][4], wallMarkerCorners[neighbor2][0], 1, 1);
    wallMarkerCorners[furthest][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][4], wallMarkerCorners[furthest][0], 1, 1);

    // add extra height to the wall
    wallMarkerCorners[neighbor1][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][0], wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][0], 1, 1+extraHeight);
    wallMarkerCorners[neighbor2][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][0], wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][0], 1, 1+extraHeight);
    wallMarkerCorners[furthest][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][0], wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][0], 1, 1+extraHeight);
    wallMarkerCorners[neighbor1][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][4], wallMarkerCorners[neighbor1][0], 1, 1);
    wallMarkerCorners[neighbor2][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][4], wallMarkerCorners[neighbor2][0], 1, 1);
    wallMarkerCorners[furthest][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][4], wallMarkerCorners[furthest][0], 1, 1);
    wallMarkerCorners[neighbor1][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][1], wallMarkerCorners[neighbor1][0], 1, 1);
    wallMarkerCorners[neighbor2][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][1], wallMarkerCorners[neighbor2][0], 1, 1);
    wallMarkerCorners[furthest][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][1], wallMarkerCorners[furthest][0], 1, 1);
    wallMarkerCorners[neighbor1][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][2], wallMarkerCorners[neighbor1][0], 1, 1);
    wallMarkerCorners[neighbor2][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][2], wallMarkerCorners[neighbor2][0], 1, 1);
    wallMarkerCorners[furthest][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][2], wallMarkerCorners[furthest][0], 1, 1);
    

    // draw floor
    if (floor){
        glBegin(GL_QUADS);
        // 0.851,0.725,0.608
        // 0.678,0.58,0.486
        glColor3f(0.678,0.58,0.486);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[closest][0].x, -wallMarkerCorners[closest][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glEnd();
    }

    
    glBegin(GL_QUADS);
    // draw second closest to furthest floor
    glColor3f(colors[0][0], colors[0][1], colors[0][2]);
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(colors[1][0], colors[1][1], colors[1][2]);
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(colors[2][0], colors[2][1], colors[2][2]);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(colors[3][0], colors[3][1], colors[3][2]);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        // draw closest to furthest wall cover
        glColor3f(colors[1][0], colors[1][1], colors[1][2]);
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
    glEnd();

    glBegin(GL_QUADS);
    // draw closest to furthest wall
    glColor3f(colors[0][0], colors[0][1], colors[0][2]);
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(colors[2][0], colors[2][1], colors[2][2]);
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(colors[1][0], colors[1][1], colors[1][2]);
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(colors[3][0], colors[3][1], colors[3][2]);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        // draw closest to furthest wall cover
        glColor3f(colors[1][0], colors[1][1], colors[1][2]);
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
    glEnd();

    if (outline){
        glBegin(GL_LINES);
        // trace inner wall first
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        glEnd();

        glBegin(GL_LINES);
        // trace outer wall second
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        glEnd();

        glBegin(GL_LINES);
        // trace roof first
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glEnd();

        glBegin(GL_LINES);
        // trace roof second
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glEnd();

        glBegin(GL_LINES);
        // trace wall cover first
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glEnd();

        glBegin(GL_LINES);
        // trace wall cover second
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glEnd();
    }
}

// TODO: Scaling
// TODO: Outline
void ObjectRender::drawTable(vector<cv::Point2f> projectedGLPoints, vector<vector<GLfloat>> colorLegs, vector<vector<GLfloat>> colorTable, float scale, bool outline){
    // leg0
    vector<cv::Point2f> projectClone = projectedGLPoints;
    projectedGLPoints[3] = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectClone[3], projectedGLPoints[0], 0, scale);
    projectedGLPoints[6] = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectClone[6], projectedGLPoints[1], 0, scale);
    projectedGLPoints[7] = ObjectRender::vectorAddRelative(projectedGLPoints[2], projectClone[7], projectedGLPoints[2], 0, scale);
    projectedGLPoints[5] = ObjectRender::vectorAddRelative(projectedGLPoints[4], projectClone[5], projectedGLPoints[4], 0, scale);

    projectedGLPoints[0] = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectClone[4], projectedGLPoints[0], 0, (1-scale)/2);
    projectedGLPoints[1] = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectClone[2], projectedGLPoints[1], 0, (1-scale)/2);
    projectedGLPoints[2] = ObjectRender::vectorAddRelative(projectedGLPoints[2], projectClone[1], projectedGLPoints[2], 0, (1-scale)/2);
    projectedGLPoints[3] = ObjectRender::vectorAddRelative(projectedGLPoints[3], projectClone[5], projectedGLPoints[3], 0, (1-scale)/2);
    projectedGLPoints[4] = ObjectRender::vectorAddRelative(projectedGLPoints[4], projectClone[0], projectedGLPoints[4], 0, (1-scale)/2);
    projectedGLPoints[5] = ObjectRender::vectorAddRelative(projectedGLPoints[5], projectClone[3], projectedGLPoints[5], 0, (1-scale)/2);
    projectedGLPoints[6] = ObjectRender::vectorAddRelative(projectedGLPoints[6], projectClone[7], projectedGLPoints[6], 0, (1-scale)/2);
    projectedGLPoints[7] = ObjectRender::vectorAddRelative(projectedGLPoints[7], projectClone[6], projectedGLPoints[7], 0, (1-scale)/2);


    cv::Point2f leg00 = ObjectRender::vectorAddRelative(projectedGLPoints[0], projectedGLPoints[4], projectedGLPoints[0], 0, 0);
    cv::Point2f leg01 = ObjectRender::vectorAddRelative(leg00, projectedGLPoints[1], leg00, 0, (float)(scale/6));
    cv::Point2f leg04 = ObjectRender::vectorAddRelative(leg00, projectedGLPoints[4], leg00, 0, (float)(scale/6));
    cv::Point2f leg02 = ObjectRender::vectorAddRelative(leg00, projectedGLPoints[2], leg00, 0, (float)(scale/6));
    cv::Point2f leg03 = ObjectRender::vectorAddRelative(leg00, projectedGLPoints[3], leg00, 0, (float)5/6);
    cv::Point2f leg05 = ObjectRender::vectorAddRelative(leg03, leg04, leg00, 1, 1);
    cv::Point2f leg06 = ObjectRender::vectorAddRelative(leg03, leg01, leg00, 1, 1);
    cv::Point2f leg07 = ObjectRender::vectorAddRelative(leg03, leg02, leg00, 1, 1);
    
    // leg0
    glBegin(GL_QUADS);
    // 0142
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);;
    glVertex2f(leg00.x, -leg00.y);
    glVertex2f(leg01.x, -leg01.y);
    glVertex2f(leg04.x, -leg04.y);
    glVertex2f(leg02.x, -leg02.y);
    // 0163
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg00.x, -leg00.y);
    glVertex2f(leg01.x, -leg01.y);
    glVertex2f(leg06.x, -leg06.y);
    glVertex2f(leg03.x, -leg03.y);
    // 2037
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg02.x, -leg02.y);
    glVertex2f(leg00.x, -leg00.y);
    glVertex2f(leg03.x, -leg03.y);
    glVertex2f(leg07.x, -leg07.y);
    // 1456
    glColor3f(colorLegs[0][0], colorLegs[0][1], colorLegs[0][2]);
    glVertex2f(leg01.x, -leg01.y);
    glVertex2f(leg04.x, -leg04.y);
    glVertex2f(leg05.x, -leg05.y);
    glVertex2f(leg06.x, -leg06.y);
    // 2457
    glColor3f(colorLegs[1][0], colorLegs[1][1], colorLegs[1][2]);
    glVertex2f(leg04.x, -leg04.y);
    glVertex2f(leg02.x, -leg02.y);
    glVertex2f(leg07.x, -leg07.y);
    glVertex2f(leg05.x, -leg05.y);
    // 3657
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg03.x, -leg03.y);
    glVertex2f(leg06.x, -leg06.y);
    glVertex2f(leg05.x, -leg05.y);
    glVertex2f(leg07.x, -leg07.y);
    glEnd();
    // leg1
    cv::Point2f leg11 = ObjectRender::vectorAddRelative(projectedGLPoints[1], projectedGLPoints[2], projectedGLPoints[1], 0, 0);
    cv::Point2f leg10 = ObjectRender::vectorAddRelative(leg11, projectedGLPoints[0], leg11, 0, (float)(scale/6));
    cv::Point2f leg12 = ObjectRender::vectorAddRelative(leg11, projectedGLPoints[2], leg11, 0, (float)(scale/6));
    cv::Point2f leg14 = ObjectRender::vectorAddRelative(leg11, projectedGLPoints[4], leg11, 0, (float)(scale/6));
    cv::Point2f leg16 = ObjectRender::vectorAddRelative(leg11, projectedGLPoints[6], leg11, 0, (float)5/6);
    cv::Point2f leg15 = ObjectRender::vectorAddRelative(leg16, leg14, leg11, 1, 1);
    cv::Point2f leg17 = ObjectRender::vectorAddRelative(leg16, leg12, leg11, 1, 1);
    cv::Point2f leg13 = ObjectRender::vectorAddRelative(leg16, leg10, leg11, 1, 1);
    glBegin(GL_QUADS);
    // 0142
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg10.x, -leg10.y);
    glVertex2f(leg11.x, -leg11.y);
    glVertex2f(leg14.x, -leg14.y);
    glVertex2f(leg12.x, -leg12.y);
    // 0163
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg10.x, -leg10.y);
    glVertex2f(leg11.x, -leg11.y);
    glVertex2f(leg16.x, -leg16.y);
    glVertex2f(leg13.x, -leg13.y);
    // 2037
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg12.x, -leg12.y);
    glVertex2f(leg10.x, -leg10.y);
    glVertex2f(leg13.x, -leg13.y);
    glVertex2f(leg17.x, -leg17.y);
    // 1456
    glColor3f(colorLegs[0][0], colorLegs[0][1], colorLegs[0][2]);
    glVertex2f(leg11.x, -leg11.y);
    glVertex2f(leg14.x, -leg14.y);
    glVertex2f(leg15.x, -leg15.y);
    glVertex2f(leg16.x, -leg16.y);
    // 2457
    glColor3f(colorLegs[1][0], colorLegs[1][1], colorLegs[1][2]);
    glVertex2f(leg14.x, -leg14.y);
    glVertex2f(leg12.x, -leg12.y);
    glVertex2f(leg17.x, -leg17.y);
    glVertex2f(leg15.x, -leg15.y);
    // 3657
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg13.x, -leg13.y);
    glVertex2f(leg16.x, -leg16.y);
    glVertex2f(leg15.x, -leg15.y);
    glVertex2f(leg17.x, -leg17.y);
    glEnd();
    // leg4
    cv::Point2f leg44 = ObjectRender::vectorAddRelative(projectedGLPoints[4], projectedGLPoints[0], projectedGLPoints[4], 0, 0);
    cv::Point2f leg40 = ObjectRender::vectorAddRelative(leg44, projectedGLPoints[0], leg44, 0, (float)(scale/6));
    cv::Point2f leg41 = ObjectRender::vectorAddRelative(leg44, projectedGLPoints[1], leg44, 0, (float)(scale/6));
    cv::Point2f leg42 = ObjectRender::vectorAddRelative(leg44, projectedGLPoints[2], leg44, 0, (float)(scale/6));
    cv::Point2f leg45 = ObjectRender::vectorAddRelative(leg44, projectedGLPoints[5], leg44, 0, (float)5/6);
    cv::Point2f leg46 = ObjectRender::vectorAddRelative(leg45, leg41, leg44, 1, 1);
    cv::Point2f leg47 = ObjectRender::vectorAddRelative(leg45, leg42, leg44, 1, 1);
    cv::Point2f leg43 = ObjectRender::vectorAddRelative(leg45, leg40, leg44, 1, 1);
    glBegin(GL_QUADS);
    // 0142
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg40.x, -leg40.y);
    glVertex2f(leg41.x, -leg41.y);
    glVertex2f(leg44.x, -leg44.y);
    glVertex2f(leg42.x, -leg42.y);
    // 0163
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg40.x, -leg40.y);
    glVertex2f(leg41.x, -leg41.y);
    glVertex2f(leg46.x, -leg46.y);
    glVertex2f(leg43.x, -leg43.y);
    // 2037
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg42.x, -leg42.y);
    glVertex2f(leg40.x, -leg40.y);
    glVertex2f(leg43.x, -leg43.y);
    glVertex2f(leg47.x, -leg47.y);
    // 1456
    glColor3f(colorLegs[0][0], colorLegs[0][1], colorLegs[0][2]);
    glVertex2f(leg41.x, -leg41.y);
    glVertex2f(leg44.x, -leg44.y);
    glVertex2f(leg45.x, -leg45.y);
    glVertex2f(leg46.x, -leg46.y);
    // 2457
    glColor3f(colorLegs[1][0], colorLegs[1][1], colorLegs[1][2]);
    glVertex2f(leg44.x, -leg44.y);
    glVertex2f(leg42.x, -leg42.y);
    glVertex2f(leg47.x, -leg47.y);
    glVertex2f(leg45.x, -leg45.y);
    // 3657
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg43.x, -leg43.y);
    glVertex2f(leg46.x, -leg46.y);
    glVertex2f(leg45.x, -leg45.y);
    glVertex2f(leg47.x, -leg47.y);
    glEnd();
    // leg2
    cv::Point2f leg22 = ObjectRender::vectorAddRelative(projectedGLPoints[2], projectedGLPoints[1], projectedGLPoints[2], 0, 0);
    cv::Point2f leg20 = ObjectRender::vectorAddRelative(leg22, projectedGLPoints[0], leg22, 0, (float)(scale/6));
    cv::Point2f leg21 = ObjectRender::vectorAddRelative(leg22, projectedGLPoints[1], leg22, 0, (float)(scale/6));
    cv::Point2f leg24 = ObjectRender::vectorAddRelative(leg22, projectedGLPoints[4], leg22, 0, (float)(scale/6));
    cv::Point2f leg27 = ObjectRender::vectorAddRelative(leg22, projectedGLPoints[7], leg22, 0, (float)5/6);
    cv::Point2f leg23 = ObjectRender::vectorAddRelative(leg27, leg20, leg22, 1, 1);
    cv::Point2f leg25 = ObjectRender::vectorAddRelative(leg27, leg24, leg22, 1, 1);
    cv::Point2f leg26 = ObjectRender::vectorAddRelative(leg27, leg21, leg22, 1, 1);
    glBegin(GL_QUADS);
    // 0142
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg20.x, -leg20.y);
    glVertex2f(leg21.x, -leg21.y);
    glVertex2f(leg24.x, -leg24.y);
    glVertex2f(leg22.x, -leg22.y);
    // 0163
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg20.x, -leg20.y);
    glVertex2f(leg21.x, -leg21.y);
    glVertex2f(leg26.x, -leg26.y);
    glVertex2f(leg23.x, -leg23.y);
    // 2037
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg22.x, -leg22.y);
    glVertex2f(leg20.x, -leg20.y);
    glVertex2f(leg23.x, -leg23.y);
    glVertex2f(leg27.x, -leg27.y);
    // 1456
    glColor3f(colorLegs[0][0], colorLegs[0][1], colorLegs[0][2]);
    glVertex2f(leg21.x, -leg21.y);
    glVertex2f(leg24.x, -leg24.y);
    glVertex2f(leg25.x, -leg25.y);
    glVertex2f(leg26.x, -leg26.y);
    // 2457
    glColor3f(colorLegs[1][0], colorLegs[1][1], colorLegs[1][2]);
    glVertex2f(leg24.x, -leg24.y);
    glVertex2f(leg22.x, -leg22.y);
    glVertex2f(leg27.x, -leg27.y);
    glVertex2f(leg25.x, -leg25.y);
    // 3657
    glColor3f(colorLegs[2][0], colorLegs[2][1], colorLegs[2][2]);
    glVertex2f(leg23.x, -leg23.y);
    glVertex2f(leg26.x, -leg26.y);
    glVertex2f(leg25.x, -leg25.y);
    glVertex2f(leg27.x, -leg27.y);
    glEnd();
    // table top
    // table board 3636
    glBegin(GL_QUADS);
    glColor3f(colorTable[3][0], colorTable[3][1], colorTable[3][2]);
    glVertex2f(leg03.x, -leg03.y);
    glVertex2f(leg16.x, -leg16.y);
    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
    glEnd();
    // table board 3737
    glBegin(GL_QUADS);
    glColor3f(colorTable[3][0], colorTable[3][1], colorTable[3][2]);
    glVertex2f(leg27.x, -leg27.y);
    glVertex2f(leg03.x, -leg03.y);
    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
    glEnd();
    // table board 5656
    glBegin(GL_QUADS);
    glColor3f(colorTable[1][0], colorTable[1][1], colorTable[1][2]);
    glVertex2f(leg16.x, -leg16.y);
    glVertex2f(leg45.x, -leg45.y);
    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
    glEnd();
    // table board 5757
    glBegin(GL_QUADS);
    glColor3f(colorTable[2][0], colorTable[2][1], colorTable[2][2]);
    glVertex2f(leg45.x, -leg45.y);
    glVertex2f(leg27.x, -leg27.y);
    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
    glEnd();
    // table board top
    glBegin(GL_QUADS);
    glColor3f(colorTable[0][0], colorTable[0][1], colorTable[0][2]);
    glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
    glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
    glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
    glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
    glEnd();

    if(outline){
        // outline leg0
        glBegin(GL_LINES);
        // 0163
        glColor3f(0, 0, 0);
        glVertex2f(leg00.x, -leg00.y);
        glVertex2f(leg01.x, -leg01.y);
        glVertex2f(leg06.x, -leg06.y);
        glVertex2f(leg03.x, -leg03.y);
        glEnd();
        
        glBegin(GL_LINES);
        // 2037
        glColor3f(0, 0, 0);
        glVertex2f(leg02.x, -leg02.y);
        glVertex2f(leg00.x, -leg00.y);
        glVertex2f(leg03.x, -leg03.y);
        glVertex2f(leg07.x, -leg07.y);
        glEnd();
        
        glBegin(GL_LINES);
        // 1456
        glColor3f(0, 0, 0);
        glVertex2f(leg01.x, -leg01.y);
        glVertex2f(leg04.x, -leg04.y);
        glVertex2f(leg05.x, -leg05.y);
        glVertex2f(leg06.x, -leg06.y);
        glEnd();
        
        glBegin(GL_LINES);
        // 2457
        glColor3f(0, 0, 0);
        glVertex2f(leg04.x, -leg04.y);
        glVertex2f(leg02.x, -leg02.y);
        glVertex2f(leg07.x, -leg07.y);
        glVertex2f(leg05.x, -leg05.y);
        glEnd();
        
        glBegin(GL_LINES);
        // 3657
        glColor3f(0, 0, 0);
        glVertex2f(leg03.x, -leg03.y);
        glVertex2f(leg06.x, -leg06.y);
        glVertex2f(leg05.x, -leg05.y);
        glVertex2f(leg07.x, -leg07.y);
        glEnd();
    
        // outline leg1
        glBegin(GL_LINES);
        // 0163
        glColor3f(0, 0, 0);
        glVertex2f(leg10.x, -leg10.y);
        glVertex2f(leg11.x, -leg11.y);
        glVertex2f(leg16.x, -leg16.y);
        glVertex2f(leg13.x, -leg13.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2037
        glColor3f(0, 0, 0);
        glVertex2f(leg12.x, -leg12.y);
        glVertex2f(leg10.x, -leg10.y);
        glVertex2f(leg13.x, -leg13.y);
        glVertex2f(leg17.x, -leg17.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 1456
        glColor3f(0, 0, 0);
        glVertex2f(leg11.x, -leg11.y);
        glVertex2f(leg14.x, -leg14.y);
        glVertex2f(leg15.x, -leg15.y);
        glVertex2f(leg16.x, -leg16.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2457
        glColor3f(0, 0, 0);
        glVertex2f(leg14.x, -leg14.y);
        glVertex2f(leg12.x, -leg12.y);
        glVertex2f(leg17.x, -leg17.y);
        glVertex2f(leg15.x, -leg15.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 3657
        glColor3f(0, 0, 0);
        glVertex2f(leg13.x, -leg13.y);
        glVertex2f(leg16.x, -leg16.y);
        glVertex2f(leg15.x, -leg15.y);
        glVertex2f(leg17.x, -leg17.y);
        glEnd();
    
    
        // outline leg4
        glBegin(GL_LINES);
        // 0163
        glColor3f(0, 0, 0);
        glVertex2f(leg40.x, -leg40.y);
        glVertex2f(leg41.x, -leg41.y);
        glVertex2f(leg46.x, -leg46.y);
        glVertex2f(leg43.x, -leg43.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2037
        glColor3f(0, 0, 0);
        glVertex2f(leg42.x, -leg42.y);
        glVertex2f(leg40.x, -leg40.y);
        glVertex2f(leg43.x, -leg43.y);
        glVertex2f(leg47.x, -leg47.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 1456
        glColor3f(0, 0, 0);
        glVertex2f(leg41.x, -leg41.y);
        glVertex2f(leg44.x, -leg44.y);
        glVertex2f(leg45.x, -leg45.y);
        glVertex2f(leg46.x, -leg46.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2457
        glColor3f(0, 0, 0);
        glVertex2f(leg44.x, -leg44.y);
        glVertex2f(leg42.x, -leg42.y);
        glVertex2f(leg47.x, -leg47.y);
        glVertex2f(leg45.x, -leg45.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 3657
        glColor3f(0, 0, 0);
        glVertex2f(leg43.x, -leg43.y);
        glVertex2f(leg46.x, -leg46.y);
        glVertex2f(leg45.x, -leg45.y);
        glVertex2f(leg47.x, -leg47.y);
        glEnd();
    
        // outline leg2
        glBegin(GL_LINES);
        // 0163
        glColor3f(0, 0, 0);
        glVertex2f(leg20.x, -leg20.y);
        glVertex2f(leg21.x, -leg21.y);
        glVertex2f(leg26.x, -leg26.y);
        glVertex2f(leg23.x, -leg23.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2037
        glColor3f(0, 0, 0);
        glVertex2f(leg22.x, -leg22.y);
        glVertex2f(leg20.x, -leg20.y);
        glVertex2f(leg23.x, -leg23.y);
        glVertex2f(leg27.x, -leg27.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 1456
        glColor3f(0, 0, 0);
        glVertex2f(leg21.x, -leg21.y);
        glVertex2f(leg24.x, -leg24.y);
        glVertex2f(leg25.x, -leg25.y);
        glVertex2f(leg26.x, -leg26.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 2457
        glColor3f(0, 0, 0);
        glVertex2f(leg24.x, -leg24.y);
        glVertex2f(leg22.x, -leg22.y);
        glVertex2f(leg27.x, -leg27.y);
        glVertex2f(leg25.x, -leg25.y);
        glEnd();
    
        glBegin(GL_LINES);
        // 3657
        glColor3f(0, 0, 0);
        glVertex2f(leg23.x, -leg23.y);
        glVertex2f(leg26.x, -leg26.y);
        glVertex2f(leg25.x, -leg25.y);
        glVertex2f(leg27.x, -leg27.y);
        glEnd();
    
        // outlint table top
         // table top
        // table board 3636
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(leg03.x, -leg03.y);
        glVertex2f(leg16.x, -leg16.y);
        glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
        glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
        glEnd();
        // table board 3737
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(leg27.x, -leg27.y);
        glVertex2f(leg03.x, -leg03.y);
        glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
        glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
        glEnd();
        // table board 5656
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(leg16.x, -leg16.y);
        glVertex2f(leg45.x, -leg45.y);
        glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
        glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
        glEnd();
        // table board 5757
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(leg45.x, -leg45.y);
        glVertex2f(leg27.x, -leg27.y);
        glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
        glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
        glEnd();
        // table board top
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(projectedGLPoints[3].x, -projectedGLPoints[3].y);
        glVertex2f(projectedGLPoints[6].x, -projectedGLPoints[6].y);
        glVertex2f(projectedGLPoints[5].x, -projectedGLPoints[5].y);
        glVertex2f(projectedGLPoints[7].x, -projectedGLPoints[7].y);
        glEnd();
    }

}
