#include "asteroid.h"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/* returns 0 on success, -1 on failure */
static int initializeSDLandOpenGL(SDL_Window** sdl_window, SDL_GLContext* gl_context) {
    const int window_flags = (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_Init' with error '%s'", SDL_GetError());
        return -1;
    }

    /* OpenGL 1.1 (fixed function pipeline only) */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    *sdl_window = SDL_CreateWindow("Space Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (sdl_window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_CreateWindow' with error '%s'", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    *gl_context = SDL_GL_CreateContext(*sdl_window);

    return 0;
}

static void deinitializeSDLandOpenGL(SDL_Window** sdl_window, SDL_GLContext* gl_context) {
    SDL_GL_DeleteContext(*gl_context);
    SDL_DestroyWindow(*sdl_window);
    SDL_Quit();
}

int main(void) {
    SDL_Window* sdl_window;
    SDL_Event sdl_event;
    SDL_GLContext gl_context;
    bool running = true;

    if (initializeSDLandOpenGL(&sdl_window, &gl_context) != 0) {
        return -1;
    }

    /* game init code here */
    initAsteroids(50);

    while (running) {
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT) running = false;
        }

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
        SDL_GL_SwapWindow(sdl_window);
    }

    /* game deinit code here */
    deinitAsteroids();

    deinitializeSDLandOpenGL(&sdl_window, &gl_context);
    return 0;
}
