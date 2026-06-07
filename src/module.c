#include "module.h"
#include "utils.h"

#define NUM_MODULES 20
#define MODULE_SPAWN_RADIUS 150.0F

static Module modules[NUM_MODULES];

extern void initModules(void) {
    int i;
    for (i = 0; i < NUM_MODULES; i++) {
        const float dist = randRangedFloat(40.0F, MODULE_SPAWN_RADIUS);
        modules[i].position = vec3MulScalar(vec3Random(), dist);
        modules[i].active   = 1;
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