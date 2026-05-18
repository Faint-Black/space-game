#include "stars.h"
#include "utils.h"

#define NUM_STARS 8000

static Star stars[NUM_STARS];

extern void initStars(void) {
    int i;
    for (i = 0; i < NUM_STARS; i++) {
        Vec3 dir = vec3Random();
        float mag = vec3Magnitude(dir);
        if (mag < 0.001F) {
            dir = vec3(0.0F, 1.0F, 0.0F);
            mag = 1.0F;
        }
        stars[i].position = vec3MulScalar(vec3DivScalar(dir, mag), 500.0F);
        stars[i].brightness = randRangedFloat(0.1F, 1.0F);
    }
}

extern const Star* getStars(void) {
    return stars;
}

extern int getStarCount(void) {
    return NUM_STARS;
}
