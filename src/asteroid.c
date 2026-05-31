#include "asteroid.h"
#include "aabb_bvh.h"
#include "render.h"
#include "utils.h"
#include <GL/gl.h>
#include <stddef.h>
#include <stdlib.h>
#include "sphere.h"

#define MAX_ASTEROID_VELOCITY 5.0F
#define MAX_POINTS_PER_LEAF 4
#define ASTEROID_BASE_RADIUS 5.0F

/* internal global variables */
static Asteroid* asteroid_array = NULL;
static int asteroid_count = 0;
static GLuint asteroid_texture_id = 0;

static void distortVert(Vec3 *position, float height) {
    const Vec3 normal = vec3Normalize(*position);
    *position = vec3AddVector(*position, vec3MulScalar(normal, height));
}

static void distortSelfAndNeighbours(const SphereMesh *sphere, int v_index, float delta) {
    const float epsilon = 0.25F;
    int i;
    if (delta > epsilon) {
        const int neighbor_count = sphere->adjacents[v_index].count;
        if (v_index != sphere->north_pole_v_index && v_index != sphere->south_pole_v_index) {
            distortVert(&sphere->vertices[v_index], delta);
        }
        for (i = 0; i < neighbor_count; i++) {
            const int neighbour = sphere->adjacents[v_index].neighbors[i];
            distortSelfAndNeighbours(sphere, neighbour, delta / 3.0F);
        }
    }
}

static SphereMesh distortSphereMesh(SphereMesh sphere) {
    int i;
    for (i = 0; i < sphere.vertex_count; i++) {
        distortSelfAndNeighbours(&sphere, i, randNormalizedFloat() * 3.0F);
    }
    return sphere;
}

static Mesh parseSphereMesh(SphereMesh sphere) {
    Mesh result;
    int i;

    result.faces = malloc(sphere.triangle_count * sizeof(TriangleFace));
    result.face_count = sphere.triangle_count;
    for (i = 0; i < sphere.triangle_count; i++) {
        result.faces[i].v[0].position = sphere.vertices[sphere.triangles[i].a];
        result.faces[i].v[1].position = sphere.vertices[sphere.triangles[i].b];
        result.faces[i].v[2].position = sphere.vertices[sphere.triangles[i].c];
    }

    return result;
}

static Mesh resolveMiscMeshData(Mesh mesh) {
    int i;

    /* resolve normals */
    for (i = 0; i < mesh.face_count; i++) {
        const Vec3 v1 = vec3SubVector(mesh.faces[i].v[1].position, mesh.faces[i].v[0].position);
        const Vec3 v2 = vec3SubVector(mesh.faces[i].v[2].position, mesh.faces[i].v[0].position);
        const Vec3 normal = vec3Cross(v1, v2);
        mesh.faces[i].v[0].normal = normal;
        mesh.faces[i].v[1].normal = normal;
        mesh.faces[i].v[2].normal = normal;
    }

    /* resolve albedo */
    for (i = 0; i < mesh.face_count; i++) {
        mesh.faces[i].v[0].albedo = vec3(1.0F, 1.0F, 1.0F);
        mesh.faces[i].v[1].albedo = vec3(1.0F, 1.0F, 1.0F);
        mesh.faces[i].v[2].albedo = vec3(1.0F, 1.0F, 1.0F);
    }

    /* resolve texture coords */
    for (i = 0; i < mesh.face_count; i++) {
        mesh.faces[i].v[0].texture = vec3(0.0F, 1.0F, 1337.42F);
        mesh.faces[i].v[1].texture = vec3(1.0F, 0.0F, 1337.42F);
        mesh.faces[i].v[2].texture = vec3(0.0F, 0.0F, 1337.42F);
    }

    return mesh;
}

static Vec3* generateBarycenters(Mesh mesh, Vec3 initial_pos) {
    /* each triangular face has its own barycenter */
    Vec3* points = malloc(mesh.face_count * sizeof(Vec3));

    int i;
    for (i = 0; i < mesh.face_count; i++) {
        const Vec3 v1 = mesh.faces[i].v[0].position;
        const Vec3 v2 = mesh.faces[i].v[1].position;
        const Vec3 v3 = mesh.faces[i].v[2].position;
        const Vec3 model_coord = vec3BarycenterTri(v1, v2, v3);
        const Vec3 world_coord = vec3AddVector(model_coord, initial_pos);
        points[i] = world_coord;
    }

    return points;
}

static int* generateBvhIndices(int count) {
    int* result = malloc(count * sizeof(int));
    int i;
    for (i = 0; i < count; i++) {
        result[i] = i;
    }
    return result;
}

static Asteroid initAsteroid(void) {
    Asteroid result;
    SphereMesh sphere;

    result.mass = 1.0F;
    result.position = vec3MulScalar(vec3Random(), randNormalizedFloat() * INTERACTION_BOUNDS_RADIUS);
    result.velocity = vec3MulScalar(vec3Random(), MAX_ASTEROID_VELOCITY);

    sphere = generateSphere(ASTEROID_BASE_RADIUS, 20, 20);
    sphere = distortSphereMesh(sphere);
    result.mesh = resolveMiscMeshData(parseSphereMesh(sphere));
    freeSphere(sphere);

    result.texture_id = asteroid_texture_id;

    result.barycenter_array = generateBarycenters(result.mesh, result.position);
    result.barycenter_count = result.mesh.face_count;

    result.bvh_index_count = result.barycenter_count;
    result.bvh_index_array = generateBvhIndices(result.bvh_index_count);
    result.bvh = bvhBuild(result.barycenter_array, result.bvh_index_array, result.bvh_index_count, MAX_POINTS_PER_LEAF);

    return result;
}

/* frees individual asteroid allocated data */
static void deinitAsteroid(Asteroid asteroid) {
    if (asteroid.mesh.faces != NULL) {
        free(asteroid.mesh.faces);
    }
    if (asteroid.barycenter_array != NULL) {
        free(asteroid.barycenter_array);
    }
    if (asteroid.bvh_index_array != NULL) {
        free(asteroid.bvh_index_array);
    }
    if (asteroid.bvh != NULL) {
        bvhFree(asteroid.bvh);
    }
}

static void updateBVHpos(BVHNode* node, Vec3 vel) {
    node->aabb.min = vec3AddVector(node->aabb.min, vel);
    node->aabb.max = vec3AddVector(node->aabb.max, vel);
    if (node->left != NULL) updateBVHpos(node->left, vel);
    if (node->right != NULL) updateBVHpos(node->right, vel);
}

static void updateAsteroid(Asteroid* asteroid, float dt) {
    int i;
    const Vec3 vel = vec3MulScalar(asteroid->velocity, dt);

    asteroid->position = vec3AddVector(asteroid->position, vel);
    for (i = 0; i < asteroid->barycenter_count; i++) {
        asteroid->barycenter_array[i] = vec3AddVector(asteroid->barycenter_array[i], vel);
    }
    updateBVHpos(asteroid->bvh, vel);
}

extern void updateAsteroids(float dt) {
    int i;
    for (i = 0; i < asteroid_count; i++) {
        updateAsteroid(&asteroid_array[i], dt);
    }
}

extern void initAsteroids(int max_asteroid_count) {
    int i;
    asteroid_texture_id = loadTexture("./data/asteroid-texture.jpg");
    asteroid_count = max_asteroid_count;
    asteroid_array = (Asteroid*)malloc(asteroid_count * sizeof(Asteroid));
    for (i = 0; i < asteroid_count; i++) {
        asteroid_array[i] = initAsteroid();
    }
}

extern void deinitAsteroids(void) {
    if (asteroid_array != NULL) {
        int i;
        for (i = 0; i < asteroid_count; i++) {
            deinitAsteroid(asteroid_array[i]);
        }
        free(asteroid_array);
    }
    asteroid_count = 0;
}

extern const Asteroid* getAsteroids(void) {
    return asteroid_array;
}

extern int getAsteroidCount(void) {
    return asteroid_count;
}
