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

extern SphereMesh generateSphere(float radius, int stacks, int sectors)
{
    SphereMesh mesh;
    const int vertex_count = 2 + (stacks - 1) * sectors;
    const int triangle_count = sectors + sectors + 2 * sectors * (stacks - 2);

    int north;
    int south;
    int stack;
    int sector;

    int v = 0;
    int t = 0;

    mesh.vertices = NULL;
    mesh.triangles = NULL;
    mesh.vertex_count = 0;
    mesh.triangle_count = 0;

    mesh.vertices = malloc(sizeof(Vec3) * vertex_count);
    mesh.triangles = malloc(sizeof(Tri) * triangle_count);

    if (!mesh.vertices || !mesh.triangles)
    {
        free(mesh.vertices);
        free(mesh.triangles);

        mesh.vertices = NULL;
        mesh.triangles = NULL;

        return mesh;
    }

    /* north pole case */
    north = v++;
    mesh.vertices[north].x = 0.0f;
    mesh.vertices[north].y = 0.0f;
    mesh.vertices[north].z = radius;

    /* rings */
    for (stack = 1; stack < stacks; ++stack)
    {
        const float theta = PI / 2.0f - ((float)stack * PI / (float)stacks);
        const float ring_radius = radius * cos(theta);
        const float z = radius * sin(theta);

        for (sector = 0; sector < sectors; ++sector)
        {
            const float phi = 2.0f * PI * (float)sector / (float)sectors;
            mesh.vertices[v].x = ring_radius * cos(phi);
            mesh.vertices[v].y = ring_radius * sin(phi);
            mesh.vertices[v].z = z;
            ++v;
        }
    }

    /* south pole case */
    south = v++;
    mesh.vertices[south].x = 0.0f;
    mesh.vertices[south].y = 0.0f;
    mesh.vertices[south].z = -radius;

    /* top cap */
    for (sector = 0; sector < sectors; ++sector)
    {
        const int a = 1 + sector;
        const int b = 1 + ((sector + 1) % sectors);
        mesh.triangles[t++] = makeTri(north, a, b);
    }

    /* middle bands */
    for (stack = 0; stack < stacks - 2; ++stack)
    {
        const int ring0 = 1 + stack * sectors;
        const int ring1 = ring0 + sectors;
        for (sector = 0; sector < sectors; ++sector)
        {
            const int next_sector = (sector + 1) % sectors;
            const int a = ring0 + sector;
            const int b = ring0 + next_sector;
            const int c = ring1 + sector;
            const int d = ring1 + next_sector;
            mesh.triangles[t++] = makeTri(a, c, b);
            mesh.triangles[t++] = makeTri(b, c, d);
        }
    }

    /* bottom cap */
    {
        const int last_ring = 1 + (stacks - 2) * sectors;
        for (sector = 0; sector < sectors; ++sector)
        {
            const int a = last_ring + sector;
            const int b = last_ring + ((sector + 1) % sectors);
            mesh.triangles[t++] = makeTri(a, south, b);
        }
    }

    assert(v == vertex_count);
    assert(t == triangle_count);

    mesh.vertex_count = vertex_count;
    mesh.triangle_count = triangle_count;

    return mesh;
}

void freeSphere(SphereMesh mesh) {
    if (mesh.vertices != NULL) free(mesh.vertices);
    if (mesh.triangles != NULL) free(mesh.triangles);
}
