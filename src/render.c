#include "render.h"
#include "asteroid.h"
#include "stars.h"
#include "utils.h"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

extern GLuint loadTexture(const char* filepath) {
    SDL_Surface* surface = IMG_Load(filepath);
    GLenum format;
    GLuint texture_id;

    if (!surface) {
        SDL_Log("ERROR: IMG_Load failed: %s", IMG_GetError());
        return 0;
    }

    format = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    SDL_FreeSurface(surface);

    return texture_id;
}

extern void renderMesh(Mesh mesh) {
    int v_index;
    int f_index;

    glBegin(GL_TRIANGLES);
    for (f_index = 0; f_index < mesh.face_count; f_index++) {
        for (v_index = 0; v_index < 3; v_index++) {
            const Vertex vert = mesh.faces[f_index].v[v_index];
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

extern void renderAsteroids(void) {
    int i;
    for (i = 0; i < getAsteroidCount(); i++) {
        const Asteroid asteroid = getAsteroids()[i];
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
            glTranslatef(asteroid.position.x, asteroid.position.y, asteroid.position.z);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, asteroid.texture_id);
            renderMesh(asteroid.mesh);
            glDisable(GL_TEXTURE_2D);
        }
        glPopMatrix();
    }
}
