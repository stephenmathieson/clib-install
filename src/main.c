
//
// main.c
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//

#include <stdio.h>
#include <stdlib.h>
#include "clib-install.h"
#include "fs.h"
#include "commander.h"
#include "package.h"


// global opts
struct options {
  const char *dir;
};

static struct options opts;

/**
 * Option setters
 */

static void setopt_dir(command_t *self) {
  opts.dir = (char *) self->arg;
}


/**
 * Install dependencies of the package at ./
 */

static int install_local_pkg() {
  if (-1 == fs_exists("./package.json")) {
    fprintf(stderr, "Missing package.json\n");
    return 1;
  }

  char *json = fs_read("./package.json");
  if (NULL == json) return 1;

  package_t *pkg = package_from_json(json);
  if (!pkg) {
    fprintf(stderr, "Could not create package.  Perhaps package.json is malformed?\n");
    return 1;
  }

  int rc = package_install_dependencies(pkg, opts.dir);
  if (-1 == rc) fprintf(stderr, "Could not install local dependencies\n");

  free(pkg);
  return -1 == rc ? 1 : 0;
}

/**
 * Install `n` of the given `pkgs` (args)
 */

static int install_packages(int n, const char **pkgs) {
  for (int i = 1; i < n; i++) {
    package_t *pkg = package_from_repo(pkgs[i], "master");
    if (!pkg) return 1;
    int rc = package_install(pkg, opts.dir);
    free(pkg);
    if (-1 == rc) return 1;
  }

  return 0;
}

/**
 * Entry point
 */

int main(int argc, const char **argv) {
  command_t program;
  command_init(&program, "clib-install", CLIB_INSTALL_VERSION);

  program.usage = "[options] [name ...]";

  command_option(&program
    , "-o"
    , "--out <dir>"
    , "change the output directory [deps]"
    , setopt_dir);
  command_parse(&program, argc, argv);

  if (!opts.dir) {
    opts.dir = "./deps";
  }

  if (0 == program.argc) return install_local_pkg();

  return install_packages(program.argc, program.argv);
}
