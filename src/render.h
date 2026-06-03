#ifndef RENDER_H
#define RENDER_H

#include "utils.h"
#include <GL/gl.h>

typedef struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 texture; /* TODO: z unused */
    Vec3 albedo;
} Vertex;

typedef struct TriangleFace {
    Vertex v[3];
} TriangleFace;

typedef struct Mesh {
    TriangleFace* faces;
    int face_count;
} Mesh;

/**
 * @brief Loads an OpenGL texture through an image.
 */
extern GLuint loadTexture(const char* filepath);

/**
 * @brief Tells OpenGL to render in bulk the data of a mesh.
 */
extern void renderMesh(Mesh mesh);

/**
 * @brief Render horizon stars.
 */
extern void renderStars(void);

/**
 * @brief Render asteroids.
 */
extern void renderAsteroids(void);

/**
 * @brief (For debug use only) Renders an array of points as red dots.
 */
extern void debugRenderPoints(Vec3* points, int count);

/**
 * @brief (For debug use only) Renders the barycenters of the asteroids.
 */
extern void debugRenderAsteroidBarycenters(void);

/**
 * @brief (For debug use only) Renders the BVH of the asteroids.
 */
extern void debugRenderAsteroidBVHs(void);

/**
 * @brief (For debug use only) Renders the BVH of the asteroids. Only the leaf nodes.
 */
extern void debugRenderAsteroidLeafBVHs(void);

/**
 * @brief (For debug use only) Renders only leaf BVH nodes of every asteroid.
 * Yellow = leaf colliding with the ship AABB. Red = no collision.
 */
extern void debugRenderAsteroidCollisions(void);

/**
 * @brief Renders all currently live explosion particles as fading colored sparks.
 *        Called automatically by renderAsteroids() — no need to call manually.
 */
extern void renderExplosionParticles(void);

#endif /* RENDER_H */