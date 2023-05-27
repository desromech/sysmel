#include "pal-common.c"

#if defined(_WIN32)
#include "pal-windows.c"
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include "pal-unix.c"
#else
#include "pal-stdlib.c"
#endif
