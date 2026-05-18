#ifndef RENDER_H
#define RENDER_H

#include "utils.h"

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

#endif /* RENDER_H */
