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
    memset(palEvent, 0, sizeof(*palEvent));

    switch(sdlEvent->type)
    {
    case SDL_QUIT:
        palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_QUIT;
        break;
    case SDL_WINDOWEVENT:
        palEvent->window = (sysmel_pal_window_t*)SDL_GetWindowFromID(sdlEvent->window.windowID);

        switch (sdlEvent->window.event)
        {
        case SDL_WINDOWEVENT_SHOWN:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_SHOWN;
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_HIDDEN;
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_EXPOSED;
            break;
        case SDL_WINDOWEVENT_MOVED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MOVED;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_RESIZED;
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_SIZE_CHANGED;
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MINIMIZED;
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_MAXIMIZED;
            break;
        case SDL_WINDOWEVENT_RESTORED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_RESTORED;
            break;
        case SDL_WINDOWEVENT_ENTER:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_ENTER;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_LEAVE;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_GAINED;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_FOCUS_LOST;
            break;
        case SDL_WINDOWEVENT_CLOSE:
            palEvent->type = SYSMEL_PAL_WINDOW_EVENT_TYPE_CLOSE;
            break;
        default:
            // By default ignored.
            break;
        }
        break;
    default:
        break;
    }
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

SYSMEL_PAL_EXTERN_C sysmel_pal_windowRenderer_t *sysmel_pal_windowRenderer_create(sysmel_pal_window_t *window, uint32_t flags)
{
    (void)flags;
    return (sysmel_pal_windowRenderer_t*)SDL_CreateRenderer((SDL_Window*)window, -1, 0);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_destroy(sysmel_pal_windowRenderer_t *renderer)
{
    SDL_DestroyRenderer((SDL_Renderer*)renderer);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_beginFrame(sysmel_pal_windowRenderer_t *renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if(!renderer) return;
    SDL_SetRenderDrawColor((SDL_Renderer*)renderer, r, g, b, a);
    SDL_RenderClear((SDL_Renderer*)renderer);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowRenderer_endFrame(sysmel_pal_windowRenderer_t *renderer)
{
    if(!renderer) return;
    SDL_RenderPresent((SDL_Renderer*)renderer);
}
