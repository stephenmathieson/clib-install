
//
// package.c
//
// Copyright (c) 2013 Stephen Mathieson
// MIT licensed
//

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdarg.h>
#include "http-get.h"
#include "fs.h"
#include "parson.h"
#include "path-join.h"
#include "package.h"
#include "str-replace.h"
#include "mkdirp.h"

/**
 * Debug/log functions
 */

static void package_debug(const char *type, const char *msg, int color) {
  printf("  \033[%dm%10s\033[0m : \033[90m%s\033[m\n", color, type, msg);
}

static void package_log(const char *type, const char *msg, ...) {
  char *buf = malloc(256);
  va_list args;
  va_start(args, msg);
  vsprintf(buf, msg, args);
  va_end(args);
  package_debug(type, msg, 36);
}

static void package_error(const char *type, const char *msg, ...) {
  char *buf = malloc(256);
  va_list args;
  va_start(args, msg);
  vsprintf(buf, msg, args);
  va_end(args);
  package_debug(type, buf, 31);
}


/**
 * Build a `package` from the given `json`
 */

package_t *package_from_json(const char *json) {
  package_t *pkg = malloc(sizeof(package_t));
  if (NULL == pkg) {
    package_error("error", "failed to allocate enough memory");
    return NULL;
  }

  JSON_Value *root = json_parse_string(json);
  pkg->json = json_value_get_object(root);
  pkg->json_string = json;
  pkg->name = json_object_dotget_string(pkg->json, "name");
  pkg->repo = json_object_dotget_string(pkg->json, "repo");
  pkg->version = json_object_dotget_string(pkg->json, "version");
  pkg->license = json_object_dotget_string(pkg->json, "license");
  pkg->description = json_object_dotget_string(pkg->json, "description");
  pkg->install = json_object_dotget_string(pkg->json, "install");
  pkg->src = json_object_dotget_array(pkg->json, "src");
  pkg->keywords = json_object_dotget_array(pkg->json, "keywords");
  pkg->dependencies = json_object_dotget_object(pkg->json, "dependencies");
  return pkg;
}

/**
 * Build a `package` from the given `repo` at tag `version`
 */

package_t *package_from_repo(const char *repo, const char *version) {
  char *url = package_url(repo, version, "package.json");

  package_log("fetch", url);

  response_t *res = http_get(url);
  if (!res->ok) {
    package_error("error", "failed to get %s", url);
  }
  if (!res->ok) return NULL;

  package_t *pkg = package_from_json(res->data);
  if (NULL == pkg) {
    package_error("error", "failed to create package from json");
    return NULL;
  }

  // support for missing repo in package.json
  if (!pkg->repo) pkg->repo = repo;

  // dep@master may have the version x.y.z, so we
  // must force the provided version
  pkg->version = version;

  free(url);

  return pkg;
}

/**
 * Get a URL for `file` from `repo` at `version`
 */

char *package_url(const char *repo, const char *version, const char *file) {
  char *buf = malloc(sizeof(char) * 256);
  if (NULL == buf) {
    package_error("error", "failed to allocate enough memory");
    return NULL;
  }

  // TODO if "*" == version, version = "master"

  sprintf(buf
    , "https://raw.github.com/%s/%s/%s"
    , repo
    , version
    , file);
  return buf;
}

/**
 * Install the given `pkg` into `dir`
 */

int package_install(package_t *pkg, const char *dir) {
  int rc;

  rc = mkdirp(dir, 0777);
  if (-1 == rc) {
    package_error("error", "failed to mkdir %s", dir);
    return -1;
  }

  for (int i = 0; i < json_array_get_count(pkg->src); ++i) {
    char *file = (char *) json_array_get_string(pkg->src, i);
    char *path = path_join(dir, basename(file));
    char *url = package_url(pkg->repo, pkg->version, file);

    package_log("fetch", url);
    rc = http_get_file(url, path);
    package_log("save", path);
    free(file);
    free(path);
    free(url);
    if (-1 == rc) {
      package_error("error", "failed to get file %s", path);
      return -1;
    }
  }

  return package_install_dependencies(pkg, dir);
}

/**
 * Install the given `pkg`'s dependencies in `dir`
 */

int package_install_dependencies(package_t *pkg, const char *dir) {
  if (-1 == mkdirp(dir, 0777)) {
    package_error("error", "failed to mkdir %s", dir);
    return -1;
  }

  if (pkg->dependencies) {
    for (int i = 0; i < json_object_get_count(pkg->dependencies); ++i) {
      // iterate and install each of them
      char *name = (char *) json_object_get_name(pkg->dependencies, i);
      char *version = (char *) json_object_get_string(pkg->dependencies, name);
      package_t *dep = package_from_repo(name, version);
      int rc = package_install(dep, dir);
      free(dep);
      free(name);
      free(version);
      if (-1 == rc) {
        package_error("error", "failed to get dependency %s", name);
        return -1;
      }
    }
  }

  return 0;
}

/**
 * Install the given binary `pkg`
 */

int package_install_binary(package_t *pkg) {
  char *url = package_tarball(pkg->repo, pkg->version);
  char *slug = str_replace(pkg->repo, "/", "-");
  if (NULL == url || NULL == slug) {
    package_error("error", "failed to allocate enough memory");
    return -1;
  }

  char *tarball = malloc(sizeof(char) * 256);
  if (NULL == tarball) {
    package_error("error", "failed to allocate enough memory");
    free(url);
    return -1;
  }

  sprintf(tarball
    , "/tmp/%s"
    , slug);

  // save archive to /tmp
  package_log("fetch", url);
  if (-1 == http_get_file(url, tarball)) {
    package_error("error", "failed to get file %s", tarball);
    return -1;
  }

  // TODO install pkg's deps first

  char *command = malloc(sizeof(char) * 256);
  int code;

  // cheap untar
  sprintf(command
    , "cd /tmp && tar -xf %s"
    , slug);

  code = system(command);
  if (0 != code) {
    package_error("error", "tar returned %d", code);
    free(command);
    return code;
  }

  // cheap install
  sprintf(command
    , "cd /tmp/%s-%s && %s"
    , pkg->name
    , pkg->version
    , pkg->install);

  code = system(command);
  free(command);
  return code;
}

/**
 * Get a URL for the `repo`'s tarball at `version`
 */

char *package_tarball(const char *repo, const char *version) {
  char *buf = malloc(sizeof(char) * 256);
  if (NULL == buf) {
    package_error("error", "failed to allocate enough memory");
    return NULL;
  }

  // TODO if "*" == version, version = "master"

  sprintf(buf
    , "https://github.com/%s/archive/%s.tar.gz"
    , repo
    , version);
  return buf;
}


/**
 * Inspect the given `pkg`
 */

void package_inspect(package_t *pkg) {
  printf("%s\n", pkg->json_string);
}
