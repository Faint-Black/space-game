#ifndef ASTEROID_H
#define ASTEROID_H

#include "aabb_bvh.h"
#include "render.h"
#include "utils.h"
#include <GL/gl.h>

typedef struct Asteroid {
    /* Graphical data */
    Mesh mesh;
    GLuint texture_id;

    /* Collision data */
    Vec3* barycenter_array;
    int barycenter_count;
    int* bvh_index_array;
    int bvh_index_count;
    BVHNode* bvh;

    /* Gameplay data */
    float mass;
    Vec3 position;
    Vec3 velocity;
    int hit_points;
} Asteroid;

typedef struct ExplosionParticle {
    Vec3  position;
    Vec3  velocity;
    Vec3  color;
    float lifetime;     
    float max_lifetime; 
} ExplosionParticle;

/**
 * @brief Initializes internal state of asteroids. Must be called once before game begins.
 * This also loads the asteroid texture into OpenGL.
 */
extern void initAsteroids(int max_asteroid_count);

/**
 * @brief Deinitializes internal state of asteroids. Must be called after game is finished.
 */
extern void deinitAsteroids(void);

/**
 * @brief Updates all the asteroids. Must be called before the frame is rendered.
 */
extern void updateAsteroids(float dt);

/**
 * @brief Fetches from global-state the constant reference to the asteroid entity array.
 *
 * @return Asteroid array.
 */
extern const Asteroid* getAsteroids(void);

/**
 * @brief Fetches from global-state the length of the asteroid entity array.
 *
 * @return Asteroid array length.
 */
extern int getAsteroidCount(void);

/**
 * @brief Destroys the asteroid at the given index and immediately respawns it
 *        at a new random position within the interaction bounds.
 *        Also emits an explosion particle burst at the asteroid's position.
 *
 * @param asteroid_id Index of the asteroid to destroy (0-based).
 */
extern void destroyAsteroid(int asteroid_id);

/**
 * @brief Applies one point of damage to the asteroid at the given index.
 *        When its hit-points reach zero, destroyAsteroid() is called
 *        automatically, respawning the asteroid at a new random location.
 *
 * @param asteroid_id Index of the asteroid to damage (0-based).
 */
extern void damageAsteroid(int asteroid_id);

/**
 * @brief Returns a read-only pointer to the explosion particle pool.
 *        Intended for use by render.c only.
 */
extern const ExplosionParticle* getExplosionParticles(void);

/**
 * @brief Returns the capacity of the explosion particle pool.
 *        Intended for use by render.c only.
 */
extern int getExplosionParticleCapacity(void);

#endif /* ASTEROID_H */
