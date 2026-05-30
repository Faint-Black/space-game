#include "hud.h"
#include <GL/gl.h>

/* ======================================================================== */
/*  HUD layout constants                                                     */
/* ======================================================================== */

#define HUD_MARGIN 30.0F
#define HUD_LINE_WIDTH 2.5F

#define DIGIT_WIDTH 16.0F
#define DIGIT_HEIGHT 24.0F
#define DIGIT_SPACING 6.0F
#define MAX_DIGITS 12

#define LIFE_ICON_RADIUS 10.0F
#define LIFE_ICON_GAP 28.0F
#define SCORE_ICON_RADIUS 12.0F
#define CROSSHAIR_RADIUS 10.0F
#define CROSSHAIR_GAP 3.0F

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

    if ((mask & 0x01) != 0) hudLine(x, y + h, x + w, y + h);
    if ((mask & 0x02) != 0) hudLine(x + w, y + mid, x + w, y + h);
    if ((mask & 0x04) != 0) hudLine(x + w, y, x + w, y + mid);
    if ((mask & 0x08) != 0) hudLine(x, y, x + w, y);
    if ((mask & 0x10) != 0) hudLine(x, y, x, y + mid);
    if ((mask & 0x20) != 0) hudLine(x, y + mid, x, y + h);
    if ((mask & 0x40) != 0) hudLine(x, y + mid, x + w, y + mid);
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
    glVertex2f(cx, cy + r);
    glVertex2f(cx - r * 0.7F, cy - r * 0.5F);
    glVertex2f(cx + r * 0.7F, cy - r * 0.5F);
    glEnd();
}

/* Diamond icon prefixing the score readout. */
static void drawScoreIcon(float cx, float cy) {
    float r = SCORE_ICON_RADIUS;
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx, cy + r);
    glVertex2f(cx + r, cy);
    glVertex2f(cx, cy - r);
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
    cull_was_enabled = glIsEnabled(GL_CULL_FACE);
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
    drawScoreIcon(HUD_MARGIN + SCORE_ICON_RADIUS, h - HUD_MARGIN - SCORE_ICON_RADIUS);
    drawNumber(score, HUD_MARGIN + (SCORE_ICON_RADIUS * 2.0F) + 16.0F, h - HUD_MARGIN - DIGIT_HEIGHT);

    /* Lives (top-right). */
    for (i = 0; i < lives; i++) {
        drawLifeIcon(w - HUD_MARGIN - LIFE_ICON_RADIUS - (float)i * LIFE_ICON_GAP, h - HUD_MARGIN - LIFE_ICON_RADIUS);
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

/* ======================================================================== */
/*  Pause overlay                                                            */
/* ======================================================================== */

#define PAUSE_BAR_WIDTH  20.0F
#define PAUSE_BAR_HEIGHT 60.0F
#define PAUSE_BAR_GAP    14.0F

#define PAUSE_COLOR_R 1.0F
#define PAUSE_COLOR_G 1.0F
#define PAUSE_COLOR_B 0.3F

extern void renderPauseOverlay(void) {
    GLint viewport[4];
    float w;
    float h;
    float cx;
    float cy;
    float bw;
    float bh_half;
    float gap;
    GLboolean depth_was_enabled;
    GLboolean light_was_enabled;
    GLboolean cull_was_enabled;
    GLfloat saved_color[4];

    glGetIntegerv(GL_VIEWPORT, viewport);
    w = (float)viewport[2];
    h = (float)viewport[3];

    cx = w * 0.5F;
    cy = h * 0.5F;
    bw = PAUSE_BAR_WIDTH;
    bh_half = PAUSE_BAR_HEIGHT * 0.5F;
    gap = PAUSE_BAR_GAP;

    depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    light_was_enabled = glIsEnabled(GL_LIGHTING);
    cull_was_enabled  = glIsEnabled(GL_CULL_FACE);
    glGetFloatv(GL_CURRENT_COLOR, saved_color);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)w, 0.0, (double)h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(PAUSE_COLOR_R, PAUSE_COLOR_G, PAUSE_COLOR_B);

    glBegin(GL_QUADS);
    /* Left bar */
    glVertex2f(cx - gap - bw, cy - bh_half);
    glVertex2f(cx - gap,      cy - bh_half);
    glVertex2f(cx - gap,      cy + bh_half);
    glVertex2f(cx - gap - bw, cy + bh_half);
    /* Right bar */
    glVertex2f(cx + gap,      cy - bh_half);
    glVertex2f(cx + gap + bw, cy - bh_half);
    glVertex2f(cx + gap + bw, cy + bh_half);
    glVertex2f(cx + gap,      cy + bh_half);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glColor4fv(saved_color);

    if (depth_was_enabled) glEnable(GL_DEPTH_TEST);
    if (light_was_enabled) glEnable(GL_LIGHTING);
    if (cull_was_enabled) glEnable(GL_CULL_FACE);
}

/* ======================================================================== */
/*  Game-over screen                                                         */
/* ======================================================================== */

#define GAMEOVER_GLYPH_WIDTH   22.0F
#define GAMEOVER_GLYPH_HEIGHT  32.0F
#define GAMEOVER_GLYPH_SPACING 12.0F
#define GAMEOVER_HINT_SCALE    0.5F

#define GAMEOVER_COLOR_R 1.0F
#define GAMEOVER_COLOR_G 0.25F
#define GAMEOVER_COLOR_B 0.25F

#define GAMEOVER_HINT_R 0.0F
#define GAMEOVER_HINT_G 1.0F
#define GAMEOVER_HINT_B 0.4F

/* Letters are drawn as free line strokes inside a unit cell [0..1]x[0..1],
 * which (unlike the 7-segment cells used for digits) can render diagonals,
 * so M, V and R read correctly. Each glyphLine() takes a line within the cell
 * as (ax,ay)->(bx,by) with y growing upward and scales it into the cell. */
static void glyphLine(float x, float y, float gw, float gh,
                      float ax, float ay, float bx, float by) {
    hudLine(x + ax * gw, y + ay * gh, x + bx * gw, y + by * gh);
}

static void glyphLetter(char c, float x, float y, float gw, float gh) {
    switch (c) {
    case 'G':
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.0F, 0.0F);
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 1.0F, 0.0F, 1.0F, 0.45F);
        glyphLine(x, y, gw, gh, 1.0F, 0.45F, 0.5F, 0.45F);
        break;
    case 'A':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.5F, 1.0F);
        glyphLine(x, y, gw, gh, 0.5F, 1.0F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 0.25F, 0.5F, 0.75F, 0.5F);
        break;
    case 'M':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.5F, 0.45F);
        glyphLine(x, y, gw, gh, 0.5F, 0.45F, 1.0F, 1.0F);
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 1.0F, 0.0F);
        break;
    case 'E':
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.0F, 0.0F);
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 0.0F, 0.5F, 0.7F, 0.5F);
        break;
    case 'O':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 1.0F, 1.0F);
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 1.0F, 0.0F, 0.0F, 0.0F);
        break;
    case 'V':
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.5F, 0.0F);
        glyphLine(x, y, gw, gh, 0.5F, 0.0F, 1.0F, 1.0F);
        break;
    case 'R':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 1.0F, 0.78F);
        glyphLine(x, y, gw, gh, 1.0F, 0.78F, 0.0F, 0.55F);
        glyphLine(x, y, gw, gh, 0.4F, 0.55F, 1.0F, 0.0F);
        break;
    case 'P':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 1.0F, 0.75F);
        glyphLine(x, y, gw, gh, 1.0F, 0.75F, 0.0F, 0.5F);
        break;
    case 'S':
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.0F, 0.5F);
        glyphLine(x, y, gw, gh, 0.0F, 0.5F, 1.0F, 0.5F);
        glyphLine(x, y, gw, gh, 1.0F, 0.5F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 1.0F, 0.0F, 0.0F, 0.0F);
        break;
    case 'C':
        glyphLine(x, y, gw, gh, 1.0F, 1.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 0.0F, 0.0F);
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 1.0F, 0.0F);
        break;
    case 'N':
        glyphLine(x, y, gw, gh, 0.0F, 0.0F, 0.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 1.0F, 0.0F);
        glyphLine(x, y, gw, gh, 1.0F, 0.0F, 1.0F, 1.0F);
        break;
    case 'T':
        glyphLine(x, y, gw, gh, 0.0F, 1.0F, 1.0F, 1.0F);
        glyphLine(x, y, gw, gh, 0.5F, 1.0F, 0.5F, 0.0F);
        break;
    default:
        break;
    }
}

/* Draws a string of stroke-letters horizontally centered on cx. */
static void drawLabel(const char* text, float cx, float y, float gw, float gh, float spacing) {
    int len = 0;
    int i;
    float step = gw + spacing;
    float total;
    float x;

    while (text[len] != '\0') len++;
    total = (float)len * step - spacing;
    x = cx - total * 0.5F;

    for (i = 0; i < len; i++) {
        if (text[i] != ' ') {
            glyphLetter(text[i], x, y, gw, gh);
        }
        x += step;
    }
}

extern void renderGameOverScreen(int final_score, int asteroids_destroyed) {
    GLint viewport[4];
    float w;
    float h;
    float cx;
    float cy;
    float hint_w;
    float hint_h;
    float hint_sp;
    GLboolean depth_was_enabled;
    GLboolean light_was_enabled;
    GLboolean cull_was_enabled;
    GLboolean blend_was_enabled;
    GLfloat saved_color[4];
    GLfloat saved_line_width;

    glGetIntegerv(GL_VIEWPORT, viewport);
    w = (float)viewport[2];
    h = (float)viewport[3];
    cx = w * 0.5F;
    cy = h * 0.5F;

    hint_w  = GAMEOVER_GLYPH_WIDTH  * GAMEOVER_HINT_SCALE;
    hint_h  = GAMEOVER_GLYPH_HEIGHT * GAMEOVER_HINT_SCALE;
    hint_sp = GAMEOVER_GLYPH_SPACING * GAMEOVER_HINT_SCALE;

    depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    light_was_enabled = glIsEnabled(GL_LIGHTING);
    cull_was_enabled  = glIsEnabled(GL_CULL_FACE);
    blend_was_enabled = glIsEnabled(GL_BLEND);
    glGetFloatv(GL_CURRENT_COLOR, saved_color);
    glGetFloatv(GL_LINE_WIDTH, &saved_line_width);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    /* Blended so the dimming panel darkens the frozen scene behind the text. */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)w, 0.0, (double)h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Dimming panel covering the whole screen. */
    glColor4f(0.0F, 0.0F, 0.0F, 0.65F);
    glBegin(GL_QUADS);
    glVertex2f(0.0F, 0.0F);
    glVertex2f(w,    0.0F);
    glVertex2f(w,    h);
    glVertex2f(0.0F, h);
    glEnd();

    /* Title. */
    glLineWidth(HUD_LINE_WIDTH);
    glColor3f(GAMEOVER_COLOR_R, GAMEOVER_COLOR_G, GAMEOVER_COLOR_B);
    drawLabel("GAME OVER", cx, cy + 60.0F,
              GAMEOVER_GLYPH_WIDTH, GAMEOVER_GLYPH_HEIGHT, GAMEOVER_GLYPH_SPACING);

    /* Final score and asteroid kill count, drawn with the score digits. */
    glColor3f(HUD_COLOR_R, HUD_COLOR_G, HUD_COLOR_B);
    drawScoreIcon(cx - 70.0F, cy + 12.0F);
    drawNumber(final_score, cx - 40.0F, cy);

    drawLifeIcon(cx - 65.0F, cy - 48.0F);
    drawNumber(asteroids_destroyed, cx - 40.0F, cy - 60.0F);

    /* Restart hint. */
    glLineWidth(HUD_LINE_WIDTH * GAMEOVER_HINT_SCALE + 0.5F);
    glColor3f(GAMEOVER_HINT_R, GAMEOVER_HINT_G, GAMEOVER_HINT_B);
    drawLabel("PRESS R", cx, cy - 120.0F, hint_w, hint_h, hint_sp);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glLineWidth(saved_line_width);
    glColor4fv(saved_color);

    if (!blend_was_enabled) glDisable(GL_BLEND);
    if (depth_was_enabled)  glEnable(GL_DEPTH_TEST);
    if (light_was_enabled)  glEnable(GL_LIGHTING);
    if (cull_was_enabled)   glEnable(GL_CULL_FACE);
}

/* ======================================================================== */
/*  Title screen                                                             */
/* ======================================================================== */

#define TITLE_COLOR_R 0.0F
#define TITLE_COLOR_G 1.0F
#define TITLE_COLOR_B 0.6F

extern void renderTitleScreen(void) {
    GLint viewport[4];
    float w;
    float h;
    float cx;
    float cy;
    float hint_w;
    float hint_h;
    float hint_sp;
    GLboolean depth_was_enabled;
    GLboolean light_was_enabled;
    GLboolean cull_was_enabled;
    GLboolean blend_was_enabled;
    GLfloat saved_color[4];
    GLfloat saved_line_width;

    glGetIntegerv(GL_VIEWPORT, viewport);
    w = (float)viewport[2];
    h = (float)viewport[3];
    cx = w * 0.5F;
    cy = h * 0.5F;

    hint_w  = GAMEOVER_GLYPH_WIDTH  * GAMEOVER_HINT_SCALE;
    hint_h  = GAMEOVER_GLYPH_HEIGHT * GAMEOVER_HINT_SCALE;
    hint_sp = GAMEOVER_GLYPH_SPACING * GAMEOVER_HINT_SCALE;

    depth_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    light_was_enabled = glIsEnabled(GL_LIGHTING);
    cull_was_enabled  = glIsEnabled(GL_CULL_FACE);
    blend_was_enabled = glIsEnabled(GL_BLEND);
    glGetFloatv(GL_CURRENT_COLOR, saved_color);
    glGetFloatv(GL_LINE_WIDTH, &saved_line_width);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)w, 0.0, (double)h, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Dimming panel covering the whole screen. */
    glColor4f(0.0F, 0.0F, 0.0F, 0.65F);
    glBegin(GL_QUADS);
    glVertex2f(0.0F, 0.0F);
    glVertex2f(w,    0.0F);
    glVertex2f(w,    h);
    glVertex2f(0.0F, h);
    glEnd();

    /* Title. */
    glLineWidth(HUD_LINE_WIDTH);
    glColor3f(TITLE_COLOR_R, TITLE_COLOR_G, TITLE_COLOR_B);
    drawLabel("SPACE GAME", cx, cy + 30.0F,
              GAMEOVER_GLYPH_WIDTH, GAMEOVER_GLYPH_HEIGHT, GAMEOVER_GLYPH_SPACING);

    /* Start hint. */
    glLineWidth(HUD_LINE_WIDTH * GAMEOVER_HINT_SCALE + 0.5F);
    glColor3f(HUD_COLOR_R, HUD_COLOR_G, HUD_COLOR_B);
    drawLabel("PRESS ENTER", cx, cy - 60.0F, hint_w, hint_h, hint_sp);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glLineWidth(saved_line_width);
    glColor4fv(saved_color);

    if (!blend_was_enabled) glDisable(GL_BLEND);
    if (depth_was_enabled)  glEnable(GL_DEPTH_TEST);
    if (light_was_enabled)  glEnable(GL_LIGHTING);
    if (cull_was_enabled)   glEnable(GL_CULL_FACE);
}
