
#ifndef CLIB_BINARY_H
#define CLIB_BINARY_H 1

#include "clib-package/clib-package.h"

char *
clib_package_tarball_url(clib_package_t *);

int
clib_package_install_binary(clib_package_t *, int);

#endif
