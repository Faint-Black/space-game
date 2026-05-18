#include "hud.h"
#include <GL/gl.h>

/* ======================================================================== */
/*  HUD layout constants                                                     */
/* ======================================================================== */

#define HUD_MARGIN         30.0F
#define HUD_LINE_WIDTH     2.5F

#define DIGIT_WIDTH        16.0F
#define DIGIT_HEIGHT       24.0F
#define DIGIT_SPACING      6.0F
#define MAX_DIGITS         12

#define LIFE_ICON_RADIUS   10.0F
#define LIFE_ICON_GAP      28.0F
#define SCORE_ICON_RADIUS  12.0F
#define CROSSHAIR_RADIUS   10.0F
#define CROSSHAIR_GAP      3.0F

/* HUD tint: green-cyan CRT readout. */
#define HUD_COLOR_R 0.0F
#define HUD_COLOR_G 1.0F
#define HUD_COLOR_B 0.4F

/* ======================================================================== */
/*  7-segment digit table                                                    */
/* ======================================================================== */

/* Bit i of the byte tells whether segment i is lit:
 *     0
 *   5   1
 *     6
 *   4   2
 *     3
 */
static const unsigned char digit_segments[10] = {
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F  /* 9 */
};

static void hudLine(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

static void drawDigit(int d, float x, float y) {
    unsigned char mask;
    float w;
    float h;
    float mid;

    if (d < 0 || d > 9) return;

    mask = digit_segments[d];
    w = DIGIT_WIDTH;
    h = DIGIT_HEIGHT;
    mid = h * 0.5F;

    if ((mask & 0x01) != 0) hudLine(x,     y + h,   x + w, y + h);
    if ((mask & 0x02) != 0) hudLine(x + w, y + mid, x + w, y + h);
    if ((mask & 0x04) != 0) hudLine(x + w, y,       x + w, y + mid);
    if ((mask & 0x08) != 0) hudLine(x,     y,       x + w, y);
    if ((mask & 0x10) != 0) hudLine(x,     y,       x,     y + mid);
    if ((mask & 0x20) != 0) hudLine(x,     y + mid, x,     y + h);
    if ((mask & 0x40) != 0) hudLine(x,     y + mid, x + w, y + mid);
}

/**
 * @brief Draws a non-negative integer growing to the right starting at x_left.
 */
static void drawNumber(int value, float x_left, float y) {
    int digits[MAX_DIGITS];
    int count;
    int i;
    float step;

    if (value < 0) value = 0;

    if (value == 0) {
        digits[0] = 0;
        count = 1;
    } else {
        count = 0;
        while (value > 0 && count < MAX_DIGITS) {
            digits[count++] = value % 10;
            value /= 10;
        }
    }

    step = DIGIT_WIDTH + DIGIT_SPACING;
    for (i = 0; i < count; i++) {
        drawDigit(digits[count - 1 - i], x_left + (float)i * step, y);
    }
}

/* ======================================================================== */
/*  Icons                                                                    */
/* ======================================================================== */

/* Small ship-like triangle representing a remaining life. */
static void drawLifeIcon(float cx, float cy) {
    float r = LIFE_ICON_RADIUS;
    glBegin(GL_TRIANGLES);
    glVertex2f(cx,            cy + r);
    glVertex2f(cx - r * 0.7F, cy - r * 0.5F);
    glVertex2f(cx + r * 0.7F, cy - r * 0.5F);
    glEnd();
}

/* Diamond icon prefixing the score readout. */
static void drawScoreIcon(float cx, float cy) {
    float r = SCORE_ICON_RADIUS;
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx,     cy + r);
    glVertex2f(cx + r, cy);
    glVertex2f(cx,     cy - r);
    glVertex2f(cx - r, cy);
    glEnd();
}

/* Center crosshair for aiming projectiles. */
static void drawCrosshair(float cx, float cy) {
    float r = CROSSHAIR_RADIUS;
    float g = CROSSHAIR_GAP;
    glBegin(GL_LINES);
    glVertex2f(cx - r, cy);
    glVertex2f(cx - g, cy);
    glVertex2f(cx + g, cy);
    glVertex2f(cx + r, cy);
    glVertex2f(cx, cy - r);
    glVertex2f(cx, cy - g);
    glVertex2f(cx, cy + g);
    glVertex2f(cx, cy + r);
    glEnd();
}

/* ======================================================================== */
/*  Public entry point                                                       */
/* ======================================================================== */

extern void renderHUD(int score, int lives) {
    GLint viewport[4];
    float w;
    float h;
    GLboolean depth_was_enabled;
    GLboolean light_was_enabled;
    GLboolean cull_was_enabled;
    GLfloat saved_color[4];
    GLfloat saved_line_width;
    int i;

    glGetIntegerv(GL_VIEWPORT, viewport);
    w = (float)viewport[2];
    h = (float)viewport[3];

    /* Cache state we are about to mutate so the 3D scene rendering is undisturbed. */
    depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    light_was_enabled = glIsEnabled(GL_LIGHTING);
    cull_was_enabled  = glIsEnabled(GL_CULL_FACE);
    glGetFloatv(GL_CURRENT_COLOR, saved_color);
    glGetFloatv(GL_LINE_WIDTH, &saved_line_width);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    /* 2D orthographic projection, origin at bottom-left, units = pixels. */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)w, 0.0, (double)h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glLineWidth(HUD_LINE_WIDTH);
    glColor3f(HUD_COLOR_R, HUD_COLOR_G, HUD_COLOR_B);

    /* Score (top-left). */
    drawScoreIcon(HUD_MARGIN + SCORE_ICON_RADIUS,
                  h - HUD_MARGIN - SCORE_ICON_RADIUS);
    drawNumber(score,
               HUD_MARGIN + (SCORE_ICON_RADIUS * 2.0F) + 16.0F,
               h - HUD_MARGIN - DIGIT_HEIGHT);

    /* Lives (top-right). */
    for (i = 0; i < lives; i++) {
        drawLifeIcon(w - HUD_MARGIN - LIFE_ICON_RADIUS - (float)i * LIFE_ICON_GAP,
                     h - HUD_MARGIN - LIFE_ICON_RADIUS);
    }

    /* Crosshair (center). */
    drawCrosshair(w * 0.5F, h * 0.5F);

    /* Restore matrices. */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    /* Restore mutable state. */
    glLineWidth(saved_line_width);
    glColor4fv(saved_color);

    if (depth_was_enabled) glEnable(GL_DEPTH_TEST);
    if (light_was_enabled) glEnable(GL_LIGHTING);
    if (cull_was_enabled) glEnable(GL_CULL_FACE);
}