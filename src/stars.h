#ifndef STARS_H
#define STARS_H

#include "utils.h"

typedef struct Star {
    Vec3 position;
    float brightness;
} Star;

extern void initStars(void);

extern const Star* getStars(void);

extern int getStarCount(void);

#endif /* STARS_H */
