#include "ship.h"
#include "utils.h"

Ship globalShip;

extern Vec3 getShipPosition() {
    return globalShip.position;
}

extern Vec3 getShipForward() {
    return globalShip.forward;
}

/* TODO */
extern void updateShip(float dt) {
    // ship->;
}
