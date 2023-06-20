
//#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>


#include <math.h>


/* PI */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


void drawSphere(double r, int lats, int longs) {
	int i, j;
	for(i = 0; i <= lats; i++) {
		double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
		double z0  = r * sin(lat0);
		double zr0 = r *  cos(lat0);

		double lat1 = M_PI * (-0.5 + (double) i / lats);
		double z1  = r * sin(lat1);
		double zr1 = r * cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for(j = 0; j <= longs; j++) {
			double lng = 2 * M_PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
}


void drawCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{

	// draw the upper part of the cone
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, height);
	for (int angle = 0; angle < 360; angle++) {
		glVertex3f(sin((double)angle) * base, cos((double)angle) * base, 0.f);
	}
	glEnd();

	// draw the base of the cone
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);
	for (int angle = 0; angle < 360; angle++) {
		// normal is just pointing down
		glNormal3f(0, -1, 0);
		glVertex3f(sin((double)angle) * base, cos((double)angle) * base, 0.f);
	}
	glEnd();
}

void drawBlock(GLfloat width, GLfloat height, GLfloat depth) {
    // Define the vertices of the block
    GLfloat vertices[8][3] = {
        { -width / 2, -height / 2, -depth / 2 },  // Bottom Left Back
        { width / 2, -height / 2, -depth / 2 },   // Bottom Right Back
        { width / 2, -height / 2, depth / 2 },    // Bottom Right Front
        { -width / 2, -height / 2, depth / 2 },   // Bottom Left Front
        { -width / 2, height / 2, -depth / 2 },   // Top Left Back
        { width / 2, height / 2, -depth / 2 },    // Top Right Back
        { width / 2, height / 2, depth / 2 },     // Top Right Front
        { -width / 2, height / 2, depth / 2 },    // Top Left Front
    };

    // Define the faces of the block
    GLint faces[6][4] = {
        { 0, 1, 2, 3 },     // Bottom Face
        { 4, 5, 6, 7 },     // Top Face
        { 0, 4, 7, 3 },     // Left Face
        { 1, 5, 6, 2 },     // Right Face
        { 0, 1, 5, 4 },     // Back Face
        { 3, 2, 6, 7 },     // Front Face
    };

    // Draw the block
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            glVertex3fv(vertices[faces[i][j]]);
        }
    }
    glEnd();
}
