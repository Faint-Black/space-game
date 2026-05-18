#include "render.h"
#include "stars.h"
#include "utils.h"
#include <GL/gl.h>

extern void renderTriFaces(const TriangleFace* tf_array, int count) {
    int v_index;
    int f_index;

    glBegin(GL_TRIANGLES);
    for (f_index = 0; f_index < count; f_index++) {
        for (v_index = 0; v_index < 3; v_index++) {
            const Vertex vert = tf_array[f_index].v[v_index];
            glColor3f(vert.albedo.x, vert.albedo.y, vert.albedo.z);
            glNormal3f(vert.normal.x, vert.normal.y, vert.normal.z);
            glTexCoord2f(vert.texture.x, vert.texture.y);
            glVertex3f(vert.position.x, vert.position.y, vert.position.z);
        }
    }
    glEnd();
}

extern void renderStars(void) {
    int i;
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPointSize(2.0F);

    glBegin(GL_POINTS);
    for (i = 0; i < getStarCount(); i++) {
        const Star star = getStars()[i];
        glColor3f(star.brightness, star.brightness, star.brightness * 0.95F);
        glVertex3f(star.position.x, star.position.y, star.position.z);
    }
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
