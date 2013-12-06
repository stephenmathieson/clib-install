
#include "fs.h"
#include "package.h"


// global opts
struct options {
  const char *dir;
  int verbose;
};

static struct options opts;

/**
 * Install dependencies of the package at ./
 */

static int install_local_pkg() {
  if (-1 == fs_exists("./package.json")) {
    fprintf(stderr, "Missing package.json\n");
    return -1;
  }

  char *json = fs_read("./package.json");
  if (NULL == json) return -1;

  package_t *pkg = package_from_json(json);
  if (!pkg) {
    fprintf(stderr, "Could not create package.  Perhaps package.json is malformed?\n");
    return -1;
  }

  if (-1 == package_install_dependencies(pkg, opts.dir)) {
    fprintf(stderr, "Could not install local dependencies\n");
    return -1;
  }

  return 0;
}

/**
 * Install `n - 1` of the given `pkgs` (args)
 */

static int install_packages(int n, char const *pkgs[]) {
  for (int i = 1; i < n; i++) {
    package_t *pkg = package_from_repo(pkgs[i], "master");
    if (!pkg) return 1;
    if (-1 == package_install(pkg, opts.dir)) return 1;
  }

  return 0;
}

/**
 * Entry point
 */

int main(int argc, char const *argv[]) {

  // TODO parse argv for options (commander?)

  opts.dir = "./deps";

  if (1 == argc) {
    return -1 == install_local_pkg() ? 1 : 0;
  }

  return install_packages(argc, argv);
}
