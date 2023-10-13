#ifndef SYSMEL_PAL_WINDOW_H
#define SYSMEL_PAL_WINDOW_H

#include "pal.h"

typedef struct sysmel_pal_window_s sysmel_pal_window_t;

typedef struct sysmel_pal_window_event_s
{

} sysmel_pal_window_event_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_initialize(void);
SYSMEL_PAL_EXTERN_C void sysmel_pal_windowSystem_finalize(void);

SYSMEL_PAL_EXTERN_C sysmel_pal_window_t *sysmel_pal_window_create(size_t titleSize, const char *title, int x, int y, int width, int height, uint32_t flags);
SYSMEL_PAL_EXTERN_C void sysmel_pal_window_destroy(sysmel_pal_window_t *window);

SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_pollEvent(sysmel_pal_window_event_t *event);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_window_waitEvent(sysmel_pal_window_event_t *event);

#endif //SYSMEL_PAL_WINDOW_H
