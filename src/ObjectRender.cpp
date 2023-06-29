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
void ObjectRender::drawWalls(map<string, vector<cv::Point2f>> wallMarkerCorners, vector<string> sortedKeyClosest, vector<tuple<GLfloat, GLfloat, GLfloat>> colors, bool outline = true, bool floor = true, float extraHeight = 0.0){
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

    // add extra height to the walls
    wallMarkerCorners[neighbor1][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][3], wallMarkerCorners[neighbor1][0], 1, extraHeight);
    wallMarkerCorners[neighbor2][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][3], wallMarkerCorners[neighbor2][0], 1, extraHeight);
    wallMarkerCorners[furthest][3] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][3], wallMarkerCorners[furthest][0], 1, extraHeight);
    wallMarkerCorners[neighbor1][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][5], wallMarkerCorners[neighbor1][5], wallMarkerCorners[neighbor1][0], 1, extraHeight);
    wallMarkerCorners[neighbor2][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][5], wallMarkerCorners[neighbor2][5], wallMarkerCorners[neighbor2][0], 1, extraHeight);
    wallMarkerCorners[furthest][5] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][5], wallMarkerCorners[furthest][5], wallMarkerCorners[furthest][0], 1, extraHeight);
    wallMarkerCorners[neighbor1][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][6], wallMarkerCorners[neighbor1][6], wallMarkerCorners[neighbor1][0], 1, extraHeight);
    wallMarkerCorners[neighbor2][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][6], wallMarkerCorners[neighbor2][6], wallMarkerCorners[neighbor2][0], 1, extraHeight);
    wallMarkerCorners[furthest][6] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][6], wallMarkerCorners[furthest][6], wallMarkerCorners[furthest][0], 1, extraHeight);
    wallMarkerCorners[neighbor1][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor1][7], wallMarkerCorners[neighbor1][7], wallMarkerCorners[neighbor1][0], 1, extraHeight);
    wallMarkerCorners[neighbor2][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[neighbor2][7], wallMarkerCorners[neighbor2][7], wallMarkerCorners[neighbor2][0], 1, extraHeight);
    wallMarkerCorners[furthest][7] = ObjectRender::vectorAddRelative(wallMarkerCorners[furthest][7], wallMarkerCorners[furthest][7], wallMarkerCorners[furthest][0], 1, extraHeight);

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
    glColor3f(get<0>(colors[0]), get<1>(colors[0]), get<2>(colors[0]));
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
    glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(get<0>(colors[1]), get<1>(colors[1]), get<2>(colors[1]));
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(get<0>(colors[2]), get<1>(colors[2]), get<2>(colors[2]));
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(get<0>(colors[3]), get<1>(colors[3]), get<2>(colors[3]));
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        // draw closest to furthest wall cover
        glColor3f(get<0>(colors[1]), get<1>(colors[1]), get<2>(colors[1]));
        glVertex2f(wallMarkerCorners[neighbor1][0].x, -wallMarkerCorners[neighbor1][0].y);
        glVertex2f(wallMarkerCorners[neighbor1][4].x, -wallMarkerCorners[neighbor1][4].y);
        glVertex2f(wallMarkerCorners[neighbor1][5].x, -wallMarkerCorners[neighbor1][5].y);
        glVertex2f(wallMarkerCorners[neighbor1][3].x, -wallMarkerCorners[neighbor1][3].y);
    glEnd();

    glBegin(GL_QUADS);
    // draw closest to furthest wall
    glColor3f(get<0>(colors[0]), get<1>(colors[0]), get<2>(colors[0]));
    glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
    glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
    glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest outer wall
        glColor3f(get<0>(colors[2]), get<1>(colors[2]), get<2>(colors[2]));
        glVertex2f(wallMarkerCorners[neighbor2][0].x, -wallMarkerCorners[neighbor2][0].y);
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        glVertex2f(wallMarkerCorners[furthest][0].x, -wallMarkerCorners[furthest][0].y);
        // draw closest to furthest inner wall
        glColor3f(get<0>(colors[1]), get<1>(colors[1]), get<2>(colors[1]));
        glVertex2f(wallMarkerCorners[neighbor2][4].x, -wallMarkerCorners[neighbor2][4].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][4].x, -wallMarkerCorners[furthest][4].y);
        // draw closest to furthest roof
        glColor3f(get<0>(colors[3]), get<1>(colors[3]), get<2>(colors[3]));
        glVertex2f(wallMarkerCorners[neighbor2][3].x, -wallMarkerCorners[neighbor2][3].y);
        glVertex2f(wallMarkerCorners[neighbor2][5].x, -wallMarkerCorners[neighbor2][5].y);
        glVertex2f(wallMarkerCorners[furthest][5].x, -wallMarkerCorners[furthest][5].y);
        glVertex2f(wallMarkerCorners[furthest][3].x, -wallMarkerCorners[furthest][3].y);
        // draw closest to furthest wall cover
        glColor3f(get<0>(colors[1]), get<1>(colors[1]), get<2>(colors[1]));
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