#include "ship.h"
#include "utils.h"

/* Global ship state */
static struct {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 forward;
} global_ship;

extern Vec3 getShipPosition(void) {
    return global_ship.position;
}

extern Vec3 getShipForward(void) {
    return global_ship.forward;
}

/* TODO */
extern void updateShip(float dt) {
    (void)dt;
}
