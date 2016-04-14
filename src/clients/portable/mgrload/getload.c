#ifdef linux
#include "linux.c"
#endif
#ifdef __FreeBSD__
#include "freebsd.c"
#endif
#ifdef sun
#include "sunos.c"
#endif
