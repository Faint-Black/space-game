#include "utils.h"

typedef struct Tri {
    int a, b, c;
} Tri;

typedef struct AdjacentTable {
    int *neighbors;
    int count;
    int capacity;
} AdjacentTable;

typedef struct SphereMesh {
    AdjacentTable *adjacents;
    Vec3 *vertices;
    int vertex_count;
    Tri *triangles;
    int triangle_count;

    int south_pole_v_index;
    int north_pole_v_index;
} SphereMesh;

extern SphereMesh generateSphere(float radius, int stacks, int sectors);

extern void freeSphere(SphereMesh mesh);
