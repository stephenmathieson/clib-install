
//
// main.c
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fs/fs.h"
#include "commander/commander.h"
#include "clib-package/clib-package.h"
#include "clib-binary.h"
#include "clib-install.h"

// global opts
struct options {
  const char *dir;
  int verbose;
  int dev;
};

static struct options opts;

/**
 * Option setters
 */

static void
setopt_dir(command_t *self) {
  opts.dir = (char *) self->arg;
}

static void
setopt_quite(command_t *self) {
  opts.verbose = 0;
}


static void
setopt_dev(command_t *self) {
  opts.dev = 1;
}

/**
 * Install dependencies of the package at ./
 */

static int
install_local_pkg() {
  if (-1 == fs_exists("./package.json")) {
    fprintf(stderr, "Missing package.json\n");
    return 1;
  }

  char *json = fs_read("./package.json");
  if (NULL == json) return 1;

  clib_package_t *pkg = clib_package_new(json, opts.verbose);
  if (NULL == pkg) {
    free(json);
    return 1;
  }

  int rc = clib_package_install_dependencies(pkg, opts.dir, opts.verbose);

  free(json);

  if (-1 == rc) {
    clib_package_free(pkg);
    return 1;
  }

  if (opts.dev) {
    rc = clib_package_install_development(pkg, opts.dir, opts.verbose);
  }

  clib_package_free(pkg);
  return -1 == rc
    ? 1
    : 0;
}

/**
 * Install `n` of the given `pkgs` (args)
 */

static int
install_packages(int n, const char **pkgs) {
  for (int i = 0; i < n; i++) {
    clib_package_t *pkg = clib_package_new_from_slug(pkgs[i], opts.verbose);
    if (NULL == pkg) return 1;

    int rc;
    if (pkg->install) {
      rc = clib_package_install_binary(pkg, opts.verbose);
    } else {
      rc = clib_package_install(pkg, opts.dir, opts.verbose);

      if (-1 == rc) {
        clib_package_free(pkg);
        return 1;
      }

      if (opts.dev) {
        rc = clib_package_install_development(pkg, opts.dir, opts.verbose);
      }

      clib_package_free(pkg);
    }
    if (-1 == rc) return 1;
  }

  return 0;
}

/**
 * Entry point
 */

int
main(int argc, const char **argv) {
  opts.dir = "./deps";
  opts.verbose = 1;
  opts.dev = 0;

  command_t program;
  command_init(&program, "clib-install", CLIB_INSTALL_VERSION);

  program.usage = "[options] [name ...]";

  command_option(&program
    , "-o"
    , "--out <dir>"
    , "change the output directory [deps]"
    , setopt_dir);
  command_option(&program
    , "-q"
    , "--quite"
    , "disable verbose output"
    , setopt_quite);
  command_option(&program
    , "-d"
    , "--dev"
    , "install development dependencies"
    , setopt_dev);
  command_parse(&program, argc, argv);

  if (0 == program.argc) return install_local_pkg();

  return install_packages(program.argc, program.argv);
}
