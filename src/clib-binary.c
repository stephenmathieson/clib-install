
#include <stdlib.h>
#include <stdio.h>
#include "http-get/http-get.h"
#include "str-replace/str-replace.h"
#include "clib-binary.h"

char *
clib_package_tarball_url(clib_package_t *pkg) {
  char *buf = malloc(256);
  sprintf(buf
    , "https://github.com/%s/%s/archive/%s.tar.gz"
    , pkg->author
    , pkg->name
    , pkg->version);
  return buf;
}

int
clib_package_install_binary(clib_package_t *pkg, int verbose) {
  if (NULL == pkg) return -1;

  char *url = clib_package_tarball_url(pkg);
  if (NULL == url) return -1;

  char *tarball = malloc(256);
  if (NULL == tarball) {
    free(url);
    return -1;
  }

  char *file = malloc(256);
  if (NULL == file) {
    free(url);
    free(tarball);
    return -1;
  }

  sprintf(file, "%s-%s.tar.gz", pkg->name, pkg->version);
  sprintf(tarball, "/tmp/%s", file);

  int rc;

  // TODO log
  rc = http_get_file(url, tarball);

  if (-1 == rc) {
    free(url);
    free(file);
    free(tarball);
    return -1;
  }

  char *command = malloc(512);

  // cheap untar
  sprintf(command, "cd /tmp && tar -xf %s", file);

  rc = system(command);

  if (0 != rc) {
    free(url);
    free(file);
    free(tarball);
    free(command);
    return rc;
  }

  // cheap install
  sprintf(command, "cd /tmp/%s-%s && %s", pkg->name, pkg->version, pkg->install);

  rc = system(command);

  free(url);
  free(file);
  free(tarball);
  free(command);

  return rc;
}
