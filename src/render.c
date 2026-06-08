#include "render.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "module.h"
#include "ship.h"
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

    renderExplosionParticles();
}

extern void renderModules(void) {
    const Module* modules  = getModules();
    const int count = getModuleCount();
    const float size = 2.0F;
    const float ring_r = 3.5F;
    const int ring_seg = 32;
    float pulse, angle;
    int i, s;

    /* Pulse: oscillates between 0 and 1 using elapsed time */
    pulse = 0.5F + 0.5F * (float)sin((double)SDL_GetTicks() * 0.003);

    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);

    for (i = 0; i < count; i++) {
        Vec3 p;
        float r, g, b;

        if (!modules[i].active) {
            continue;
        }
        p = modules[i].position;

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);

        /* Slow rotation around Y */
        angle = (float)(SDL_GetTicks() % 360000) * 0.06F;
        glRotatef(angle, 0.0F, 1.0F, 0.0F);

        /* Pulsing gold color */
        r = 0.8F + 0.2F * pulse;
        g = 0.6F + 0.2F * pulse;
        b = 0.0F + 0.1F * pulse;

        /* Octahedron — 8 triangular faces */
        glColor3f(r, g, b);
        glBegin(GL_TRIANGLES);
        /* Top 4 faces */
        glVertex3f( 0, size,  0);  glVertex3f( size, 0,  0);    glVertex3f( 0, 0, size);
        glVertex3f( 0, size,  0);  glVertex3f( 0, 0, size);  glVertex3f(-size, 0, 0);
        glVertex3f( 0, size,  0);  glVertex3f(-size, 0,  0);    glVertex3f( 0, 0, -size);
        glVertex3f( 0, size,  0);  glVertex3f( 0, 0, -size);  glVertex3f( size, 0, 0);
        /* Bottom 4 faces — slightly dimmer */
        glColor3f(r * 0.6F, g * 0.6F, b * 0.6F);
        glVertex3f( 0, -size,  0);  glVertex3f( 0, 0,  size);  glVertex3f( size, 0,  0);
        glVertex3f( 0, -size,  0);  glVertex3f(-size, 0, 0);    glVertex3f( 0, 0,  size);
        glVertex3f( 0, -size,  0);  glVertex3f( 0, 0, -size);  glVertex3f(-size, 0, 0);
        glVertex3f( 0, -size,  0);  glVertex3f( size, 0, 0);    glVertex3f( 0, 0, -size);
        glEnd();

        /* Ring around the equator */
        glColor3f(r, g, b);
        glBegin(GL_LINE_LOOP);
        for (s = 0; s < ring_seg; s++) {
            float a = (float)s / (float)ring_seg * 6.28318F;
            glVertex3f(ring_r * (float)cos(a), 0.0F, ring_r * (float)sin(a));
        }
        glEnd();

        glPopMatrix();
    }

    glColor3f(1.0F, 1.0F, 1.0F);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}

extern void debugRenderPoints(Vec3* points, int count) {
    const Vec3 color = vec3(1.0, 0.0, 0.0);
    int i;

    glBegin(GL_POINTS);
    glColor3f(color.x, color.y, color.z);
    for (i = 0; i < count; i++) {
        const Vec3 pos = points[i];
        glVertex3f(pos.x, pos.y, pos.z);
    }
    glEnd();
}

extern void debugRenderAsteroidBarycenters(void) {
    int i;
    for (i = 0; i < getAsteroidCount(); i++) {
        const Asteroid asteroid = getAsteroids()[i];
        debugRenderPoints(asteroid.barycenter_array, asteroid.barycenter_count);
    }
}

static void debugRenderAABB(AABB box, Vec3 color) {
    glBegin(GL_LINES);
    glColor3f(color.x, color.y, color.z);
    {
        glVertex3f(box.min.x, box.min.y, box.min.z);
        glVertex3f(box.max.x, box.min.y, box.min.z);
        glVertex3f(box.max.x, box.min.y, box.min.z);
        glVertex3f(box.max.x, box.min.y, box.max.z);
        glVertex3f(box.max.x, box.min.y, box.max.z);
        glVertex3f(box.min.x, box.min.y, box.max.z);
        glVertex3f(box.min.x, box.min.y, box.max.z);
        glVertex3f(box.min.x, box.min.y, box.min.z);

        glVertex3f(box.min.x, box.max.y, box.min.z);
        glVertex3f(box.max.x, box.max.y, box.min.z);
        glVertex3f(box.max.x, box.max.y, box.min.z);
        glVertex3f(box.max.x, box.max.y, box.max.z);
        glVertex3f(box.max.x, box.max.y, box.max.z);
        glVertex3f(box.min.x, box.max.y, box.max.z);
        glVertex3f(box.min.x, box.max.y, box.max.z);
        glVertex3f(box.min.x, box.max.y, box.min.z);

        glVertex3f(box.min.x, box.min.y, box.min.z);
        glVertex3f(box.min.x, box.max.y, box.min.z);
        glVertex3f(box.max.x, box.min.y, box.min.z);
        glVertex3f(box.max.x, box.max.y, box.min.z);
        glVertex3f(box.max.x, box.min.y, box.max.z);
        glVertex3f(box.max.x, box.max.y, box.max.z);
        glVertex3f(box.min.x, box.min.y, box.max.z);
        glVertex3f(box.min.x, box.max.y, box.max.z);
    }
    glEnd();
}

static void debugRenderBVH(BVHNode* node, int depth) {
    const int color_cycle_size = 4;
    Vec3 color_cycle[4];
    color_cycle[0] = vec3(1, 1, 1); /* white */
    color_cycle[1] = vec3(1, 0, 0); /* red */
    color_cycle[2] = vec3(0, 1, 0); /* green */
    color_cycle[3] = vec3(0, 0, 1); /* blue */

    if (node == NULL) return;

    debugRenderAABB(node->aabb, color_cycle[depth % color_cycle_size]);
    if (node->left != NULL) debugRenderBVH(node->left, depth + 1);
    if (node->right != NULL) debugRenderBVH(node->right, depth + 1);
}

extern void debugRenderAsteroidBVHs(void) {
    int i;
    for (i = 0; i < getAsteroidCount(); i++) {
        const Asteroid asteroid = getAsteroids()[i];
        debugRenderBVH(asteroid.bvh, 0);
    }
}

static void debugRenderLeafBVH(BVHNode* node, const BVHNode* ship_bvh) {
    const Vec3 red    = vec3(1, 0, 0);
    const Vec3 yellow = vec3(1, 1, 0);

    if (node == NULL) {
        return;
    }
    if (bvhNodeIsLeaf(node)) {
        if (ship_bvh && aabbVsAABB(ship_bvh->aabb, node->aabb)) {
            debugRenderAABB(node->aabb, yellow);
        } else {
            debugRenderAABB(node->aabb, red);
        }
    }
    if (node->left != NULL) {
        debugRenderLeafBVH(node->left, ship_bvh);
    }
    if (node->right != NULL) {
        debugRenderLeafBVH(node->right, ship_bvh);
    }
}

extern void debugRenderAsteroidLeafBVHs(void) {
    const BVHNode* ship_bvh = getShipBVH();
    int i;
    for (i = 0; i < getAsteroidCount(); i++) {
        const Asteroid asteroid = getAsteroids()[i];
        debugRenderLeafBVH(asteroid.bvh, ship_bvh);
    }
}


extern void debugRenderAsteroidCollisions(void) {
    const BVHNode* ship_bvh = getShipBVH();
    int i;

    glDisable(GL_LIGHTING);
    glLineWidth(1.5F);

    /* Draw asteroid leaf BVH nodes: yellow if overlapping ship BVH root, red otherwise */
    for (i = 0; i < getAsteroidCount(); i++) {
        const Asteroid* ast = &getAsteroids()[i];
        const Vec3 yellow = { 1.0F, 1.0F, 0.0F };
        const Vec3 red    = { 1.0F, 0.0F, 0.0F };

        if (!ast->bvh) {
            continue;
        }

        if (ship_bvh && aabbVsAABB(ship_bvh->aabb, ast->bvh->aabb)) {
            debugRenderAABB(ast->bvh->aabb, yellow);
        } else {
            debugRenderAABB(ast->bvh->aabb, red);
        }
    }

    glLineWidth(1.0F);
    glEnable(GL_LIGHTING);
}


extern void renderExplosionParticles(void) {
    const ExplosionParticle* particles = getExplosionParticles();
    const int capacity = getExplosionParticleCapacity();
    GLfloat saved_color[4];
    GLfloat saved_point_size;
    GLboolean blend_was_enabled;
    GLboolean light_was_enabled;
    int i;

    {
        int any_live = 0;
        for (i = 0; i < capacity; i++) {
            if (particles[i].lifetime > 0.0F) { any_live = 1; break; }
        }
        if (!any_live) return;
    }

    glGetFloatv(GL_CURRENT_COLOR, saved_color);
    glGetFloatv(GL_POINT_SIZE,    &saved_point_size);
    blend_was_enabled = glIsEnabled(GL_BLEND);
    light_was_enabled = glIsEnabled(GL_LIGHTING);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3.0F);

    glBegin(GL_POINTS);
    for (i = 0; i < capacity; i++) {
        const ExplosionParticle* p = &particles[i];
        float alpha;

        if (p->lifetime <= 0.0F) continue;

        alpha = p->lifetime / p->max_lifetime;

        glColor4f(p->color.x, p->color.y, p->color.z, alpha);
        glVertex3f(p->position.x, p->position.y, p->position.z);
    }
    glEnd();

    glColor4fv(saved_color);
    glPointSize(saved_point_size);
    if (!blend_was_enabled) glDisable(GL_BLEND);
    if (light_was_enabled)  glEnable(GL_LIGHTING);
}