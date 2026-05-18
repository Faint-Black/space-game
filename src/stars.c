#include "stars.h"
#include "utils.h"

#define NUM_STARS 8000

static Star stars[NUM_STARS];

extern void initStars(void) {
    int i;
    for (i = 0; i < NUM_STARS; i++) {
        stars[i].position = vec3MulScalar(vec3Random(), 500.0F);
        stars[i].brightness = randRangedFloat(0.1F, 1.0F);
    }
}

extern const Star* getStars(void) {
    return stars;
}

extern int getStarCount(void) {
    return NUM_STARS;
}
