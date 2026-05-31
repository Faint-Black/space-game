#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

typedef struct Tri {
    int a, b, c;
} Tri;

typedef struct {
    Vec3 *vertices;
    int vertex_count;
    Tri *triangles;
    int triangle_count;
} SphereMesh;

static Tri makeTri(int a, int b, int c) {
    Tri result;
    result.a = a;
    result.b = b;
    result.c = c;
    return result;
}

extern SphereMesh generateSphere(float radius, int stacks, int sectors) {
    SphereMesh mesh = {0};
    const int vertex_count = (stacks + 1) * (sectors + 1);
    const int triangle_count = 2 * sectors * (stacks - 1);
    int i = 0;
    int j = 0;
    int v = 0;
    int t = 0;

    mesh.vertices = (Vec3 *)malloc(sizeof(Vec3) * vertex_count);
    for (i = 0; i <= stacks; ++i) {
        float theta = PI / 2.0f - i * PI / stacks;
        float xy = radius * cos(theta);
        float z = radius * sin(theta);
        for (j = 0; j <= sectors; ++j, ++v) {
            float sectorAngle = j * 2.0f * PI / sectors;
            mesh.vertices[v].x = xy * cos(sectorAngle);
            mesh.vertices[v].y = xy * sin(sectorAngle);
            mesh.vertices[v].z = z;
        }
    }
    assert(v == vertex_count);

    mesh.triangles = (Tri *)malloc(sizeof(Tri) * triangle_count);
    for (i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for (j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) mesh.triangles[t++] = makeTri(k1, k2, k1 + 1);
            if (i != (stacks - 1)) mesh.triangles[t++] = makeTri(k1 + 1, k2, k2 + 1);
        }
    }
    assert(t == triangle_count);

    mesh.triangle_count = triangle_count;
    mesh.vertex_count = vertex_count;
    return mesh;
}

void freeSphere(SphereMesh mesh) {
    if (mesh.vertices != NULL) free(mesh.vertices);
    if (mesh.triangles != NULL) free(mesh.triangles);
}
