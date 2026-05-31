#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "sphere.h"

static void addNeighbor(AdjacentTable *adj, int neighbor)
{
    int i;
    for (i = 0; i < adj->count; ++i)
    {
        if (adj->neighbors[i] == neighbor) return;
    }

    if (adj->count == adj->capacity)
    {
        const int new_capacity = (adj->capacity == 0) ? 8 : adj->capacity * 2;
        adj->neighbors = realloc(adj->neighbors, sizeof(int) * new_capacity);
        adj->capacity = new_capacity;
    }

    adj->neighbors[adj->count++] = neighbor;
}

static void addEdge(SphereMesh *mesh, int a, int b)
{
    addNeighbor(&mesh->adjacents[a], b);
    addNeighbor(&mesh->adjacents[b], a);
}

static Tri makeTri(int a, int b, int c, SphereMesh* sm) {
    Tri result;

    result.a = a;
    result.b = b;
    result.c = c;

    addEdge(sm, a, c);
    addEdge(sm, c, b);
    addEdge(sm, b, a);

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
    mesh.adjacents = NULL;
    mesh.vertex_count = 0;
    mesh.triangle_count = 0;

    mesh.vertices = malloc(sizeof(Vec3) * vertex_count);
    mesh.triangles = malloc(sizeof(Tri) * triangle_count);
    mesh.adjacents = calloc(vertex_count, sizeof(AdjacentTable));

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
        mesh.triangles[t++] = makeTri(north, a, b, &mesh);
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
            mesh.triangles[t++] = makeTri(a, c, b, &mesh);
            mesh.triangles[t++] = makeTri(b, c, d, &mesh);
        }
    }

    /* bottom cap */
    {
        const int last_ring = 1 + (stacks - 2) * sectors;
        for (sector = 0; sector < sectors; ++sector)
        {
            const int a = last_ring + sector;
            const int b = last_ring + ((sector + 1) % sectors);
            mesh.triangles[t++] = makeTri(a, south, b, &mesh);
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
    if (mesh.adjacents != NULL) {
        int i;
        for (i = 0; i < mesh.vertex_count; i++) {
            free(mesh.adjacents[i].neighbors);
        }
        free(mesh.adjacents);
    }
}
