#include "module.h"
#include "utils.h"
#include <math.h>

#define NUM_MODULES       20
#define MODULE_SPAWN_RADIUS 150.0F
#define MODULE_ROTATION_SPEED 60.0F  /* degrees per second */
#define MODULE_PULSE_SPEED    3.0F   /* radians per second */

static Module modules[NUM_MODULES];

extern void initModules(void) {
    int i;
    for (i = 0; i < NUM_MODULES; i++) {
        const float dist = randRangedFloat(40.0F, MODULE_SPAWN_RADIUS);
        modules[i].position      = vec3MulScalar(vec3Random(), dist);
        modules[i].rotationAngle = randRangedFloat(0.0F, 360.0F);
        modules[i].pulsePhase    = randRangedFloat(0.0F, 6.28318F);
        modules[i].active        = 1;
    }
}

extern void updateModules(float dt) {
    int i;
    for (i = 0; i < NUM_MODULES; i++) {
        if (!modules[i].active) {
            continue;
        }
        modules[i].rotationAngle += MODULE_ROTATION_SPEED * dt;
        if (modules[i].rotationAngle >= 360.0F) {
            modules[i].rotationAngle -= 360.0F;
        }
        modules[i].pulsePhase += MODULE_PULSE_SPEED * dt;
        if (modules[i].pulsePhase >= 6.28318F) {
            modules[i].pulsePhase -= 6.28318F;
        }
    }
}

extern const Module* getModules(void) {
    return modules;
}

extern int getModuleCount(void) {
    return NUM_MODULES;
}

extern void collectModule(int module_id) {
    if (module_id < 0 || module_id >= NUM_MODULES) {
        return;
    }
    modules[module_id].active = 0;
}