#include "stars.h"
#include "utils.h"

#define NUM_STARS 16000

static Star stars[NUM_STARS];

extern void initStars(void) {
    int i;
    for (i = 0; i < NUM_STARS; i++) {
        const Vec3 direction = vec3Random();
        const float magnitude = INTERACTION_BOUNDS_RADIUS + randNormalizedFloat() * INTERACTION_BOUNDS_RADIUS;
        stars[i].position = vec3MulScalar(direction, magnitude);
        stars[i].brightness = randRangedFloat(0.1F, 1.0F);
    }
}

extern const Star* getStars(void) {
    return stars;
}

extern int getStarCount(void) {
    return NUM_STARS;
}
