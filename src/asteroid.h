#ifndef ASTEROID_H
#define ASTEROID_H

#include "utils.h"

typedef struct Asteroid {
    Vec3* points;
    int point_count;
} Asteroid;

/**
 * @brief Fetches from global-state the reference to the asteroid entity array.
 *
 * @return Asteroid array.
 */
extern Asteroid* getAsteroids(void);

/**
 * @brief Fetches from global-state the length of the asteroid entity array.
 *
 * @return Asteroid array length.
 */
extern int getAsteroidCount(void);

#endif /* ASTEROID_H */
