#include "sysmel/pal-window.h"
#include "SDL.h"

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void)
{
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void)
{
    SDL_Quit();
}

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags)
{
    (void)flags;
    Uint32 sdlFlags = SDL_WINDOW_ALLOW_HIGHDPI;

    char *titleCString = malloc(titleSize + 1);
    memcpy(titleCString, title, titleSize);
    titleCString[titleSize] = 0;

    int windowX = x;
    if(windowX < 0)
        windowX = SDL_WINDOWPOS_CENTERED;

    int windowY = y;
    if(windowY < 0)
        windowY = SDL_WINDOWPOS_CENTERED;

    SDL_Window *window = SDL_CreateWindow(titleCString, windowX, windowY, width, height, sdlFlags);

    free(titleCString);
    return (sysmel_pal_window_t*)window;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window)
{
    SDL_DestroyWindow((SDL_Window*)window);
}

static void sysmel_pal_sdl2Window_convertEvent(SDL_Event *sdlEvent, sysmel_pal_window_event_t *palEvent)
{
    // TODO: Implement this conversion.
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event)
{
    SDL_Event sdlEvent;
    if(!SDL_PollEvent(&sdlEvent))
        return false;

    sysmel_pal_sdl2Window_convertEvent(&sdlEvent, event);
    return true;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event)
{
    SDL_Event sdlEvent;
    if(!SDL_WaitEvent(&sdlEvent))
        return false;

    sysmel_pal_sdl2Window_convertEvent(&sdlEvent, event);
    return true;
}
