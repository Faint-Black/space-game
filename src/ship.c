#include "ship.h"
#include "objloader.h"
#include "utils.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static Ship global_ship;

/* Key state arrays */
static int keys[512];

extern Vec3 getShipPosition() {
    return global_ship.position;
}

extern Vec3 getShipForward() {
    return global_ship.forward;
}

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
    global_ship.rotationSpeed = 2.0F;
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
}

extern void deinitShip() {
    if (global_ship.model != NULL) {
        freeOBJModel(global_ship.model);
        global_ship.model = NULL;
    }
}

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
            global_ship.projectiles[i].position = vec3AddVector(
                global_ship.position, vec3MulScalar(global_ship.forward, 3.0F));
            global_ship.projectiles[i].velocity = vec3AddVector(
                global_ship.velocity, vec3MulScalar(global_ship.forward, PROJECTILE_SPEED));
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
}

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
    if (keys[SDL_SCANCODE_Q]) vertInput   += 1.0F;
    if (keys[SDL_SCANCODE_E]) vertInput   -= 1.0F;
    if (keys[SDL_SCANCODE_J]) rollInput   += 1.0F;
    if (keys[SDL_SCANCODE_L]) rollInput   -= 1.0F;

    if (keys[SDL_SCANCODE_UP])    pitchInput += 1.0F;
    if (keys[SDL_SCANCODE_DOWN])  pitchInput -= 1.0F;
    if (keys[SDL_SCANCODE_LEFT])  yawInput   += 1.0F;
    if (keys[SDL_SCANCODE_RIGHT]) yawInput   -= 1.0F;

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
        thrust = vec3AddVector(thrust,
            vec3MulScalar(global_ship.forward, thrustInput * global_ship.thrustPower));
    }
    if (strafeInput != 0.0F) {
        thrust = vec3AddVector(thrust,
            vec3MulScalar(global_ship.right, strafeInput * global_ship.thrustPower));
    }
    if (vertInput != 0.0F) {
        thrust = vec3AddVector(thrust,
            vec3MulScalar(global_ship.up, vertInput * global_ship.thrustPower));
    }

    /* Apply acceleration (F=ma, m=1) */
    global_ship.velocity = vec3AddVector(global_ship.velocity, vec3MulScalar(thrust, dt));

    /* Linear damping */
    global_ship.velocity = vec3MulScalar(global_ship.velocity, 1.0F - global_ship.dampingLinear);

    /* Update position */
    global_ship.position = vec3AddVector(global_ship.position,
        vec3MulScalar(global_ship.velocity, dt));

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
            global_ship.projectiles[i].position = vec3AddVector(
                global_ship.projectiles[i].position,
                vec3MulScalar(global_ship.projectiles[i].velocity, dt));
            global_ship.projectiles[i].life -= dt;
            if (global_ship.projectiles[i].life <= 0.0F) {
                global_ship.projectiles[i].active = 0;
            }
        }
    }
}

extern void shipSetupCamera(int windowW, int windowH) {
    float aspect;
    Vec3 camPos, lookAt;

    if (windowH == 0) windowH = 1;
    aspect = (float)windowW / (float)windowH;

    /* Projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)aspect, 0.5, 600.0);

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
        camPos = vec3AddVector(global_ship.position,
            vec3AddVector(
                vec3MulScalar(global_ship.up, 0.8F),
                vec3MulScalar(global_ship.forward, 1.5F)));
        lookAt = vec3AddVector(camPos, vec3MulScalar(global_ship.forward, 10.0F));
    }

    gluLookAt(
        (double)camPos.x, (double)camPos.y, (double)camPos.z,
        (double)lookAt.x, (double)lookAt.y, (double)lookAt.z,
        0.0, 1.0, 0.0);
}

static void drawSimpleCube(float size) {
    float h = size * 0.5F;
    glBegin(GL_QUADS);
    /* Front */
    glNormal3f(0, 0, 1);
    glVertex3f(-h, -h, h); glVertex3f(h, -h, h);
    glVertex3f(h, h, h); glVertex3f(-h, h, h);
    /* Back */
    glNormal3f(0, 0, -1);
    glVertex3f(h, -h, -h); glVertex3f(-h, -h, -h);
    glVertex3f(-h, h, -h); glVertex3f(h, h, -h);
    /* Top */
    glNormal3f(0, 1, 0);
    glVertex3f(-h, h, h); glVertex3f(h, h, h);
    glVertex3f(h, h, -h); glVertex3f(-h, h, -h);
    /* Bottom */
    glNormal3f(0, -1, 0);
    glVertex3f(-h, -h, -h); glVertex3f(h, -h, -h);
    glVertex3f(h, -h, h); glVertex3f(-h, -h, h);
    /* Right */
    glNormal3f(1, 0, 0);
    glVertex3f(h, -h, h); glVertex3f(h, -h, -h);
    glVertex3f(h, h, -h); glVertex3f(h, h, h);
    /* Left */
    glNormal3f(-1, 0, 0);
    glVertex3f(-h, -h, -h); glVertex3f(-h, -h, h);
    glVertex3f(-h, h, h); glVertex3f(-h, h, -h);
    glEnd();
}

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

static void drawMechanicalArm(float armAng, float clawAng) {
    GLfloat armColor[] = {0.5F, 0.5F, 0.55F, 1.0F};
    GLfloat clawColor[] = {0.7F, 0.3F, 0.1F, 1.0F};

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, armColor);

    /* Shoulder at front of ship (-Z is forward) */
    glTranslatef(0.0F, -0.5F, -3.0F);
    glRotatef(global_ship.armYaw, 0, 1, 0);
    glRotatef(global_ship.armPitch, 1, 0, 0);
    drawSimpleSphere(0.15F, 8, 6);

    /* Upper arm rotates at shoulder */
    glRotatef(armAng, 1, 0, 0);
    glTranslatef(0.0F, -0.5F, 0.0F);
    glPushMatrix();
    glScalef(0.1F, 0.5F, 0.1F);
    drawSimpleCube(1.0F);
    glPopMatrix();

    /* Elbow */
    glTranslatef(0.0F, -0.5F, 0.0F);
    drawSimpleSphere(0.12F, 8, 6);

    /* Forearm */
    glRotatef(armAng * 0.5F, 1, 0, 0);
    glTranslatef(0.0F, -0.4F, 0.0F);
    glPushMatrix();
    glScalef(0.08F, 0.4F, 0.08F);
    drawSimpleCube(1.0F);
    glPopMatrix();

    /* Claw base */
    glTranslatef(0.0F, -0.4F, 0.0F);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, clawColor);

    /* Left finger */
    glPushMatrix();
    glRotatef(-clawAng, 0, 0, 1);
    glTranslatef(-0.1F, -0.2F, 0.0F);
    glScalef(0.05F, 0.2F, 0.05F);
    drawSimpleCube(1.0F);
    glPopMatrix();

    /* Right finger */
    glPushMatrix();
    glRotatef(clawAng, 0, 0, 1);
    glTranslatef(0.1F, -0.2F, 0.0F);
    glScalef(0.05F, 0.2F, 0.05F);
    drawSimpleCube(1.0F);
    glPopMatrix();

    glPopMatrix();
}

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

    /* Mechanical arm (always visible, including 1st person) */
    glPushMatrix();
    glTranslatef(global_ship.position.x, global_ship.position.y, global_ship.position.z);
    glRotatef(global_ship.yaw * 180.0F / (float)M_PI, 0, 1, 0);
    glRotatef(global_ship.pitch * 180.0F / (float)M_PI, 1, 0, 0);
    glRotatef(global_ship.roll * 180.0F / (float)M_PI, 0, 0, 1);
    drawMechanicalArm(global_ship.armAngle, global_ship.clawAngle);
    glPopMatrix();

    /* Render projectiles (always visible) */
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, projColor);
    glMaterialfv(GL_FRONT, GL_EMISSION, projEmit);
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (global_ship.projectiles[i].active) {
            glPushMatrix();
            glTranslatef(
                global_ship.projectiles[i].position.x,
                global_ship.projectiles[i].position.y,
                global_ship.projectiles[i].position.z);
            drawSimpleSphere(0.15F, 6, 4);
            glPopMatrix();
        }
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
}

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
