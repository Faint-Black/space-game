#include "ship.h"
#include "objloader.h"
#include "utils.h"
#include "aabb_bvh.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SHIP_MAX_POINTS_PER_LEAF 4

/* ======================================================================== */
/*  Global ship instance                                                     */
/* ======================================================================== */

static Ship global_ship;

/* Key state arrays */
static int keys[512];

static Vec3 shipModelToWorld(Vec3 local) {
    Vec3 result;
    result.x = global_ship.position.x - local.x * global_ship.right.x + local.y * global_ship.up.x + local.z * global_ship.forward.x;
    result.y = global_ship.position.y - local.x * global_ship.right.y + local.y * global_ship.up.y + local.z * global_ship.forward.y;
    result.z = global_ship.position.z - local.x * global_ship.right.z + local.y * global_ship.up.z + local.z * global_ship.forward.z;
    return result;
}

static int countShipFaces(const OBJModel* model) {
    int i;
    int total = 0;
    if (model == NULL) {
        return 0;
    }
    for (i = 0; i < model->materialCount; i++) {
        total += model->materials[i].faceCount;
    }
    return total;
}

static Vec3* generateShipBarycenters(const OBJModel* model, int count) {
    Vec3* points = malloc(count * sizeof(Vec3));
    int m;
    int f;
    int idx = 0;
    for (m = 0; m < model->materialCount; m++) {
        const MaterialGroup* group = &model->materials[m];
        for (f = 0; f < group->faceCount; f++) {
            const Vec3 v1 = group->faces[f].v[0].position;
            const Vec3 v2 = group->faces[f].v[1].position;
            const Vec3 v3 = group->faces[f].v[2].position;
            points[idx] = vec3BarycenterTri(v1, v2, v3);
            idx++;
        }
    }
    return points;
}

static int* generateShipBvhIndices(int count) {
    int* result = malloc(count * sizeof(int));
    int i;
    for (i = 0; i < count; i++) {
        result[i] = i;
    }
    return result;
}

static void buildShipBVH(void) {
    int i;

    global_ship.bvh = NULL;
    global_ship.barycenter_local = NULL;
    global_ship.barycenter_world = NULL;
    global_ship.barycenter_count = 0;
    global_ship.bvh_index_array = NULL;
    global_ship.bvh_index_count = 0;

    if (global_ship.model == NULL) {
        return;
    }

    global_ship.barycenter_count = countShipFaces(global_ship.model);
    if (global_ship.barycenter_count <= 0) {
        global_ship.barycenter_count = 0;
        return;
    }

    global_ship.barycenter_local = generateShipBarycenters(global_ship.model, global_ship.barycenter_count);
    global_ship.barycenter_world = malloc(global_ship.barycenter_count * sizeof(Vec3));
    for (i = 0; i < global_ship.barycenter_count; i++) {
        global_ship.barycenter_world[i] = shipModelToWorld(global_ship.barycenter_local[i]);
    }

    global_ship.bvh_index_count = global_ship.barycenter_count;
    global_ship.bvh_index_array = generateShipBvhIndices(global_ship.bvh_index_count);
    global_ship.bvh = bvhBuild(global_ship.barycenter_world, global_ship.bvh_index_array, global_ship.bvh_index_count, SHIP_MAX_POINTS_PER_LEAF);
}

static void refitShipBVH(void) {
    int i;
    if (global_ship.bvh == NULL || global_ship.barycenter_world == NULL) {
        return;
    }
    for (i = 0; i < global_ship.barycenter_count; i++) {
        global_ship.barycenter_world[i] = shipModelToWorld(global_ship.barycenter_local[i]);
    }
    bvhRefit(global_ship.bvh, global_ship.barycenter_world);
}

/* ======================================================================== */
/*  Initialization / Deinitialization                                         */
/* ======================================================================== */

extern void initShip() {
    int i;

    memset(keys, 0, sizeof(keys));

    global_ship.position = vec3(0.0F, 0.0F, 0.0F);
    global_ship.velocity = vec3Null();
    global_ship.angularVelocity = vec3Null();

    global_ship.forward = vec3(0.0F, 0.0F, -1.0F);
    global_ship.up = vec3(0.0F, 1.0F, 0.0F);
    global_ship.right = vec3(1.0F, 0.0F, 0.0F);

    global_ship.pitch = 0.0F;
    global_ship.yaw = 0.0F;
    global_ship.roll = 0.0F;

    global_ship.thrustPower = 25.0F;
    global_ship.rotationSpeed = 1.0F;
    global_ship.dampingLinear = 0.004F;
    global_ship.dampingAngular = 0.008F;

    global_ship.armAngle = 0.0F;
    global_ship.clawAngle = 30.0F;
    global_ship.armYaw = 0.0F;
    global_ship.armPitch = 0.0F;
    global_ship.armExtended = 0;

    global_ship.thrusterAngle[0] = 0.0F;
    global_ship.thrusterAngle[1] = 0.0F;
    global_ship.thrusterGlow = 0.0F;

    global_ship.scannerAngle = 25.0F * (float)M_PI / 180.0F;
    global_ship.scannerRange = 50.0F;
    global_ship.scannerVisible = 1;

    global_ship.cameraLocked = 0;
    global_ship.cameraMode = 0;
    global_ship.cameraDistance = 20.0F;
    global_ship.cameraHeight = 7.0F;
    global_ship.cameraYaw = 0.0F;
    global_ship.cameraPitch = 0.15F;

    global_ship.thrusting = 0;

    for (i = 0; i < MAX_PROJECTILES; i++) {
        global_ship.projectiles[i].active = 0;
        global_ship.projectiles[i].life = 0.0F;
    }

    global_ship.model = loadOBJModel("data/ship/space_ship.obj");
    if (global_ship.model == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load ship model!");
    }

    global_ship.armModel = loadOBJModel("data/ship/mech_arm.obj");
    if (global_ship.armModel == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load arm model!");
    }

    buildShipBVH();
}

extern void deinitShip() {
    if (global_ship.model != NULL) {
        freeOBJModel(global_ship.model);
        global_ship.model = NULL;
    }
    if (global_ship.armModel != NULL) {
        freeOBJModel(global_ship.armModel);
        global_ship.armModel = NULL;
    }
    if (global_ship.bvh != NULL) {
        bvhFree(global_ship.bvh);
        global_ship.bvh = NULL;
    }
    if (global_ship.barycenter_local != NULL) {
        free(global_ship.barycenter_local);
        global_ship.barycenter_local = NULL;
    }
    if (global_ship.barycenter_world != NULL) {
        free(global_ship.barycenter_world);
        global_ship.barycenter_world = NULL;
    }
    if (global_ship.bvh_index_array != NULL) {
        free(global_ship.bvh_index_array);
        global_ship.bvh_index_array = NULL;
    }
    global_ship.barycenter_count = 0;
    global_ship.bvh_index_count = 0;
}

/* ======================================================================== */
/*  Input Handling                                                            */
/* ======================================================================== */

extern void shipKeyDown(int key) {
    if (key >= 0 && key < 512) {
        keys[key] = 1;
    }
}

extern void shipKeyUp(int key) {
    if (key >= 0 && key < 512) {
        keys[key] = 0;
    }
}

extern void shipFireProjectile() {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!global_ship.projectiles[i].active) {
            global_ship.projectiles[i].position = vec3AddVector(global_ship.position, vec3MulScalar(global_ship.forward, 3.0F));
            global_ship.projectiles[i].velocity = vec3AddVector(global_ship.velocity, vec3MulScalar(global_ship.forward, PROJECTILE_SPEED));
            global_ship.projectiles[i].life = PROJECTILE_LIFETIME;
            global_ship.projectiles[i].active = 1;
            break;
        }
    }
}

extern void shipToggleArm() {
    global_ship.armExtended = !global_ship.armExtended;
}

extern void shipToggleCamera() {
    global_ship.cameraMode = !global_ship.cameraMode;
}

extern void shipLockCamera() {
    global_ship.cameraLocked = !global_ship.cameraLocked;
    global_ship.cameraMode = 0;
    global_ship.cameraYaw = 0.0F;
    global_ship.cameraPitch = 0.15F;
}

extern void shipToggleScanner() {
    global_ship.scannerVisible = !global_ship.scannerVisible;
}

extern void shipMouseMotion(int dx, int dy) {
    float camSens = 0.001F;

    /* Rotate camera orbit */
    global_ship.cameraYaw -= (float)dx * camSens;
    global_ship.cameraPitch += (float)dy * camSens;
    if (global_ship.cameraPitch < -1.2F) global_ship.cameraPitch = -1.2F;
    if (global_ship.cameraPitch > 1.2F) global_ship.cameraPitch = 1.2F;
}

/* ======================================================================== */
/*  Orientation update from Euler angles (YXZ order)                         */
/* ======================================================================== */

static void updateOrientation() {
    float cp = (float)cos((double)global_ship.pitch);
    float sp = (float)sin((double)global_ship.pitch);
    float cy = (float)cos((double)global_ship.yaw);
    float sy = (float)sin((double)global_ship.yaw);
    float cr = (float)cos((double)global_ship.roll);
    float sr = (float)sin((double)global_ship.roll);

    /* Forward direction (-Z local in world) */
    global_ship.forward.x = -sy * cp;
    global_ship.forward.y = sp;
    global_ship.forward.z = -cy * cp;
    global_ship.forward = vec3Normalize(global_ship.forward);

    /* Right axis */
    global_ship.right.x = cy * cr + sy * sp * sr;
    global_ship.right.y = cp * sr;
    global_ship.right.z = -sy * cr + cy * sp * sr;
    global_ship.right = vec3Normalize(global_ship.right);

    /* Up = cross(right, forward) */
    global_ship.up = vec3Normalize(vec3Cross(global_ship.right, global_ship.forward));

    if (global_ship.cameraLocked) {
        global_ship.cameraYaw = global_ship.yaw;
        global_ship.cameraPitch = -global_ship.pitch;
    }
}

/* ======================================================================== */
/*  Physics Update                                                            */
/* ======================================================================== */

extern void updateShip(float dt) {
    float thrustInput = 0.0F;
    float strafeInput = 0.0F;
    float vertInput = 0.0F;
    float pitchInput = 0.0F;
    float yawInput = 0.0F;
    float rollInput = 0.0F;
    float dampFactor;
    float targetAngle, targetGlow, armTarget, clawTarget;
    Vec3 thrust;
    int i;

    /* Read input from key arrays (SDL scancodes) */
    if (keys[SDL_SCANCODE_W]) thrustInput += 1.0F;
    if (keys[SDL_SCANCODE_S]) thrustInput -= 1.0F;
    if (keys[SDL_SCANCODE_A]) strafeInput -= 1.0F;
    if (keys[SDL_SCANCODE_D]) strafeInput += 1.0F;
    if (keys[SDL_SCANCODE_Q]) vertInput += 1.0F;
    if (keys[SDL_SCANCODE_E]) vertInput -= 1.0F;
    if (keys[SDL_SCANCODE_J]) rollInput += 1.0F;
    if (keys[SDL_SCANCODE_L]) rollInput -= 1.0F;

    if (keys[SDL_SCANCODE_UP]) pitchInput += 1.0F;
    if (keys[SDL_SCANCODE_DOWN]) pitchInput -= 1.0F;
    if (keys[SDL_SCANCODE_LEFT]) yawInput += 1.0F;
    if (keys[SDL_SCANCODE_RIGHT]) yawInput -= 1.0F;

    /* Angular velocity with inertia */
    global_ship.angularVelocity.x += pitchInput * global_ship.rotationSpeed * dt;
    global_ship.angularVelocity.y += yawInput * global_ship.rotationSpeed * dt;
    global_ship.angularVelocity.z += rollInput * global_ship.rotationSpeed * dt;

    /* Angular damping */
    dampFactor = 1.0F - global_ship.dampingAngular;
    global_ship.angularVelocity = vec3MulScalar(global_ship.angularVelocity, dampFactor);

    /* Apply rotation */
    global_ship.pitch += global_ship.angularVelocity.x * dt;
    global_ship.yaw += global_ship.angularVelocity.y * dt;
    global_ship.roll += global_ship.angularVelocity.z * dt;

    updateOrientation();

    /* Linear thrust with inertia */
    global_ship.thrusting = (thrustInput != 0.0F || strafeInput != 0.0F || vertInput != 0.0F);

    thrust = vec3Null();
    if (thrustInput != 0.0F) {
        thrust = vec3AddVector(thrust, vec3MulScalar(global_ship.forward, thrustInput * global_ship.thrustPower));
    }
    if (strafeInput != 0.0F) {
        thrust = vec3AddVector(thrust, vec3MulScalar(global_ship.right, strafeInput * global_ship.thrustPower));
    }
    if (vertInput != 0.0F) {
        thrust = vec3AddVector(thrust, vec3MulScalar(global_ship.up, vertInput * global_ship.thrustPower));
    }

    /* Apply acceleration (F=ma, m=1) */
    global_ship.velocity = vec3AddVector(global_ship.velocity, vec3MulScalar(thrust, dt));

    /* Linear damping */
    global_ship.velocity = vec3MulScalar(global_ship.velocity, 1.0F - global_ship.dampingLinear);

    /* Update position */
    global_ship.position = vec3AddVector(global_ship.position, vec3MulScalar(global_ship.velocity, dt));

    /* Thruster animation */
    targetAngle = thrustInput * 15.0F;
    global_ship.thrusterAngle[0] += (targetAngle - global_ship.thrusterAngle[0]) * 5.0F * dt;
    global_ship.thrusterAngle[1] += (targetAngle - global_ship.thrusterAngle[1]) * 5.0F * dt;

    targetGlow = global_ship.thrusting ? 1.0F : 0.0F;
    global_ship.thrusterGlow += (targetGlow - global_ship.thrusterGlow) * 4.0F * dt;

    /* Arm animation */
    armTarget = global_ship.armExtended ? 45.0F : 0.0F;
    global_ship.armAngle += (armTarget - global_ship.armAngle) * 3.0F * dt;

    clawTarget = global_ship.armExtended ? 45.0F : 15.0F;
    global_ship.clawAngle += (clawTarget - global_ship.clawAngle) * 3.0F * dt;

    /* Update projectiles */
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (global_ship.projectiles[i].active) {
            global_ship.projectiles[i].position =
                vec3AddVector(global_ship.projectiles[i].position, vec3MulScalar(global_ship.projectiles[i].velocity, dt));
            global_ship.projectiles[i].life -= dt;
            if (global_ship.projectiles[i].life <= 0.0F) {
                global_ship.projectiles[i].active = 0;
            }
        }
    }

    refitShipBVH();
}

/* ======================================================================== */
/*  Camera Setup                                                              */
/* ======================================================================== */

extern void shipSetupCamera(int windowW, int windowH) {
    float aspect;
    Vec3 camPos, lookAt;

    if (windowH == 0) windowH = 1;
    aspect = (float)windowW / (float)windowH;

    /* Projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)aspect, 0.5, INTERACTION_BOUNDS_RADIUS * 2.0F);

    /* Modelview / camera */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (global_ship.cameraMode == 0) {
        /* 3rd person: orbit camera using mouse angles */
        float cy = (float)cos((double)global_ship.cameraYaw);
        float sy = (float)sin((double)global_ship.cameraYaw);
        float cp = (float)cos((double)global_ship.cameraPitch);
        float sp = (float)sin((double)global_ship.cameraPitch);
        float dist = global_ship.cameraDistance;
        Vec3 offset;

        offset.x = dist * cp * sy;
        offset.y = dist * sp;
        offset.z = dist * cp * cy;

        camPos = vec3AddVector(global_ship.position, offset);
        lookAt = global_ship.position;
    } else {
        /* 1st person: cockpit */
        camPos = vec3AddVector(global_ship.position, vec3AddVector(vec3MulScalar(global_ship.up, 0.8F), vec3MulScalar(global_ship.forward, 1.5F)));
        lookAt = vec3AddVector(camPos, vec3MulScalar(global_ship.forward, 10.0F));
    }

    gluLookAt((double)camPos.x, (double)camPos.y, (double)camPos.z, (double)lookAt.x, (double)lookAt.y, (double)lookAt.z, 0.0, 1.0, 0.0);
}

/* ======================================================================== */
/*  Simple primitives (replacements for GLUT)                                */
/* ======================================================================== */

static void drawSimpleSphere(float radius, int slices, int stacks) {
    int i, j;
    for (i = 0; i < stacks; i++) {
        float lat0 = (float)M_PI * (-0.5F + (float)i / (float)stacks);
        float lat1 = (float)M_PI * (-0.5F + (float)(i + 1) / (float)stacks);
        float y0 = (float)sin((double)lat0);
        float yr0 = (float)cos((double)lat0);
        float y1 = (float)sin((double)lat1);
        float yr1 = (float)cos((double)lat1);

        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= slices; j++) {
            float lng = 2.0F * (float)M_PI * (float)j / (float)slices;
            float x = (float)cos((double)lng);
            float z = (float)sin((double)lng);

            glNormal3f(x * yr0, y0, z * yr0);
            glVertex3f(radius * x * yr0, radius * y0, radius * z * yr0);

            glNormal3f(x * yr1, y1, z * yr1);
            glVertex3f(radius * x * yr1, radius * y1, radius * z * yr1);
        }
        glEnd();
    }
}

static void drawSimpleCone(float radius, float height, int slices) {
    int i;
    float step = 2.0F * (float)M_PI / (float)slices;

    /* Side */
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (i = slices; i >= 0; i--) {
        float angle = (float)i * step;
        float x = (float)cos((double)angle);
        float z = (float)sin((double)angle);
        glNormal3f(x, 0.3F, z);
        glVertex3f(radius * x, 0.0F, radius * z);
    }
    glEnd();
}

/* ======================================================================== */
/*  Thruster flame effect                                                     */
/* ======================================================================== */

static void drawThrusterFlame(float glow) {
    GLfloat flameColor[4];
    if (glow < 0.01F) return;

    flameColor[0] = 1.0F;
    flameColor[1] = 0.5F * glow;
    flameColor[2] = 0.0F;
    flameColor[3] = glow;
    glMaterialfv(GL_FRONT, GL_EMISSION, flameColor);
    glColor4f(1.0F, 0.5F * glow, 0.0F, glow);
    drawSimpleCone(0.3F, 0.8F * glow, 8);
    flameColor[0] = 0.0F;
    flameColor[1] = 0.0F;
    flameColor[2] = 0.0F;
    flameColor[3] = 1.0F;
    glMaterialfv(GL_FRONT, GL_EMISSION, flameColor);
}

/* ======================================================================== */
/*  Ship Rendering                                                            */
/* ======================================================================== */

extern void renderShip() {
    int i;
    GLfloat projColor[] = {1.0F, 0.9F, 0.2F, 1.0F};
    GLfloat projEmit[] = {0.8F, 0.7F, 0.0F, 1.0F};
    GLfloat noEmit[] = {0.0F, 0.0F, 0.0F, 1.0F};

    /* Only render ship model in 3rd person */
    if (global_ship.cameraMode == 0) {
        glPushMatrix();

        /* Transform to ship position and orientation */
        glTranslatef(global_ship.position.x, global_ship.position.y, global_ship.position.z);
        glRotatef(global_ship.yaw * 180.0F / (float)M_PI, 0, 1, 0);
        glRotatef(global_ship.pitch * 180.0F / (float)M_PI, 1, 0, 0);
        glRotatef(global_ship.roll * 180.0F / (float)M_PI, 0, 0, 1);

        /* Render OBJ model (rotate 180 Y so model front faces -Z) */
        if (global_ship.model != NULL) {
            glPushMatrix();
            glRotatef(180.0F, 0, 1, 0);
            glColor3f(1.0F, 1.0F, 1.0F);
            renderOBJModel(global_ship.model);
            glPopMatrix();
        }

        /* Render thruster flames */
        glPushMatrix();
        glTranslatef(-1.2F, 0.0F, 2.0F);
        glRotatef(180.0F, 1, 0, 0);
        drawThrusterFlame(global_ship.thrusterGlow);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(1.2F, 0.0F, 2.0F);
        glRotatef(180.0F, 1, 0, 0);
        drawThrusterFlame(global_ship.thrusterGlow);
        glPopMatrix();

        glPopMatrix();
    }

    /* Mechanical arm: only render when extended */
    if (global_ship.armExtended && global_ship.armModel != NULL) {
        glPushMatrix();
        glTranslatef(global_ship.position.x, global_ship.position.y, global_ship.position.z);
        glRotatef(global_ship.yaw * 180.0F / (float)M_PI, 0, 1, 0);
        glRotatef(global_ship.pitch * 180.0F / (float)M_PI, 1, 0, 0);
        glRotatef(global_ship.roll * 180.0F / (float)M_PI, 0, 0, 1);
        /* Shoulder offset: front-bottom of ship, matching old procedural arm */
        glTranslatef(0.0F, -1.8F, -3.0F);
        glScalef(0.75F, 0.75F, 0.75F);
		glRotatef(90.0F, 0, 1, 0);
		glRotatef(global_ship.armYaw, 0, 1, 0);
        glRotatef(global_ship.armPitch, 1, 0, 0);
        glColor3f(1.0F, 1.0F, 1.0F);
        renderOBJModel(global_ship.armModel);
        glPopMatrix();
    }

    /* Render projectiles (always visible) */
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, projColor);
    glMaterialfv(GL_FRONT, GL_EMISSION, projEmit);
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (global_ship.projectiles[i].active) {
            glPushMatrix();
            glTranslatef(global_ship.projectiles[i].position.x, global_ship.projectiles[i].position.y, global_ship.projectiles[i].position.z);
            drawSimpleSphere(0.15F, 6, 4);
            glPopMatrix();
        }
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
}

/* ======================================================================== */
/*  Scanner                                                                   */
/* ======================================================================== */

extern int shipPointInScanner(Vec3 point) {
    Vec3 toPoint = vec3SubVector(point, global_ship.position);
    float dist = vec3Magnitude(toPoint);
    float cosAngle;

    if (dist > global_ship.scannerRange || dist < 0.001F) {
        return 0;
    }

    cosAngle = vec3Dot(global_ship.forward, vec3Normalize(toPoint));
    return (cosAngle >= (float)cos((double)global_ship.scannerAngle));
}

extern void renderScanner() {
    int i;
    int segments = 16;
    float range = global_ship.scannerRange;
    float halfAngle = global_ship.scannerAngle;
    float radius;

    if (!global_ship.scannerVisible) return;

    radius = range * (float)tan((double)halfAngle);

    glPushMatrix();
    glTranslatef(global_ship.position.x, global_ship.position.y, global_ship.position.z);
    glRotatef(global_ship.yaw * 180.0F / (float)M_PI, 0, 1, 0);
    glRotatef(global_ship.pitch * 180.0F / (float)M_PI, 1, 0, 0);
    glRotatef(global_ship.roll * 180.0F / (float)M_PI, 0, 0, 1);

    glDisable(GL_LIGHTING);
    glColor4f(0.0F, 1.0F, 0.5F, 0.3F);

    /* Draw wireframe cone */
    glBegin(GL_LINES);
    for (i = 0; i < segments; i++) {
        float angle = 2.0F * (float)M_PI * (float)i / (float)segments;
        float x = radius * (float)cos((double)angle);
        float y = radius * (float)sin((double)angle);

        /* Line from origin to cone rim */
        glVertex3f(0.0F, 0.0F, 0.0F);
        glVertex3f(x, y, -range);
    }
    glEnd();

    /* Draw ring at cone base */
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < segments; i++) {
        float angle = 2.0F * (float)M_PI * (float)i / (float)segments;
        float x = radius * (float)cos((double)angle);
        float y = radius * (float)sin((double)angle);
        glVertex3f(x, y, -range);
    }
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void buildShipWorldTris(Vec3* out) {
    const OBJModel* model = global_ship.model;
    int m;
    int f;
    int idx = 0;

    if (model == NULL) {
        return;
    }
    for (m = 0; m < model->materialCount; m++) {
        const MaterialGroup* group = &model->materials[m];
        for (f = 0; f < group->faceCount; f++) {
            out[idx * 3 + 0] = shipModelToWorld(group->faces[f].v[0].position);
            out[idx * 3 + 1] = shipModelToWorld(group->faces[f].v[1].position);
            out[idx * 3 + 2] = shipModelToWorld(group->faces[f].v[2].position);
            idx++;
        }
    }
}

static void debugDrawShipBVHLeafTris(const BVHNode* node, const Vec3* tris, int tri_count, int* leaf_index) {
    int i;

    if (node == NULL) {
        return;
    }

    if (bvhNodeIsLeaf(node)) {
        int c = (*leaf_index) % 6;
        float r = (c == 0 || c == 3 || c == 5) ? 1.0F : 0.25F;
        float g = (c == 1 || c == 3 || c == 4) ? 1.0F : 0.25F;
        float b = (c == 2 || c == 4 || c == 5) ? 1.0F : 0.25F;

        (*leaf_index)++;
        glColor3f(r, g, b);

        for (i = 0; i < node->num_points; i++) {
            int fi = node->point_indices[i];
            if (fi < 0 || fi >= tri_count) {
                continue;
            }
            {
                const Vec3 a  = tris[fi * 3 + 0];
                const Vec3 vb = tris[fi * 3 + 1];
                const Vec3 vc = tris[fi * 3 + 2];

                glVertex3f(a.x,  a.y,  a.z);   glVertex3f(vb.x, vb.y, vb.z);
                glVertex3f(vb.x, vb.y, vb.z);  glVertex3f(vc.x, vc.y, vc.z);
                glVertex3f(vc.x, vc.y, vc.z);  glVertex3f(a.x,  a.y,  a.z);
            }
        }
        return;
    }

    debugDrawShipBVHLeafTris(node->left,  tris, tri_count, leaf_index);
    debugDrawShipBVHLeafTris(node->right, tris, tri_count, leaf_index);
}

extern void debugRenderShipBVH(void) {
    Vec3* tris;
    int leaf_index = 0;

    if (global_ship.bvh == NULL || global_ship.model == NULL || global_ship.barycenter_count <= 0) {
        return;
    }

    tris = malloc((size_t)global_ship.barycenter_count * 3 * sizeof(Vec3));
    if (tris == NULL) {
        return;
    }
    buildShipWorldTris(tris);

    glDisable(GL_LIGHTING);
    glLineWidth(1.0F);

    glBegin(GL_LINES);
    debugDrawShipBVHLeafTris(global_ship.bvh, tris, global_ship.barycenter_count, &leaf_index);
    glEnd();

    glColor3f(1.0F, 1.0F, 1.0F);
    glEnable(GL_LIGHTING);

    free(tris);
}
/* ======================================================================== */
/*  accessor                                                     */
/* ======================================================================== */

extern const Projectile* getProjectiles(void) {
    return global_ship.projectiles;
}

extern Vec3 getShipPosition() {
    return global_ship.position;
}

extern Vec3 getShipForward() {
    return global_ship.forward;
}

extern Vec3 getShipUp() {
    return global_ship.up;
}

extern Vec3 getShipRight() {
    return global_ship.right;
}

extern int getShipArmExtended() {
    return global_ship.armExtended;
}

extern const BVHNode* getShipBVH(void) {
    return global_ship.bvh;
}