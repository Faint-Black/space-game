#include "asteroid.h"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/* global variables */
static SDL_Window* global_sdl_window;
static SDL_GLContext global_gl_context;
static bool global_app_should_run = true;
static long global_tick_counter = 0;

/* returns 0 on success, -1 on failure */
static int initializeSDLandOpenGL(void) {
    const int window_flags = (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_Init' with error '%s'", SDL_GetError());
        return -1;
    }

    /* OpenGL 1.1 (fixed function pipeline only) */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    global_sdl_window = SDL_CreateWindow("Space Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (global_sdl_window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_CreateWindow' with error '%s'", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    global_gl_context = SDL_GL_CreateContext(global_sdl_window);

    return 0;
}

static void deinitializeSDLandOpenGL(void) {
    SDL_GL_DeleteContext(global_gl_context);
    SDL_DestroyWindow(global_sdl_window);
    SDL_Quit();
}

static void gameInit(void) {
    if (initializeSDLandOpenGL() != 0) exit(EXIT_FAILURE);

    initAsteroids(50);
}

static void gameDeinit(void) {
    deinitAsteroids();

    deinitializeSDLandOpenGL();
}

static void gamePollEvents(void) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) global_app_should_run = false;
    }
}

static void gameUpdate(void) {
    global_tick_counter += 1;
}

static void gameRenderFrame(void) {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    /* hello world triangle */
    glBegin(GL_TRIANGLES);
    {
        glColor3f(1.0F, 0.0F, 0.0F);
        glVertex2f(-0.5F, -0.5F);

        glColor3f(0.0F, 1.0F, 0.0F);
        glVertex2f(0.5F, -0.5F);

        glColor3f(0.0F, 0.0F, 1.0F);
        glVertex2f(0.0F, 0.5F);
    }
    glEnd();

    /* swap front and back buffers */
    SDL_GL_SwapWindow(global_sdl_window);
}

int main(void) {
    gameInit();

    while (global_app_should_run) {
        gamePollEvents();
        gameUpdate();
        gameRenderFrame();
    }

    gameDeinit();
    return 0;
}
