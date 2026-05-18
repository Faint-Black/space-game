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

/**
 * @brief Tells OpenGL to render an array of triangle faces (with GL_TRIANGLES).
 */
extern void renderTriFaces(const TriangleFace* tf_array, int count);

#endif /* RENDER_H */
