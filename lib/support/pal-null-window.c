#include "sysmel/pal-window.h"

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void)
{
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void)
{
}

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags)
{
    (void)titleSize;
    (void)title;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)flags;
    return NULL;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window)
{
    (void)window;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event)
{
    (void)event;
    return false;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event)
{
    (void)event;
    return false;
}
