
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include "http-get.h"
#include "fs.h"
#include "parson.h"
#include "path-join.h"
#include "package.h"
#include "str-replace.h"


static void package_log(char *type, char *msg) {
  int color = 36;
  printf("  \033[%dm%10s\033[0m : \033[90m%s\033[m\n", color, type, msg);
}


/**
 * Build a `package` from the given `json`
 */

package_t *package_from_json(char *json) {
  package_t *pkg = malloc(sizeof(package_t));
  if (NULL == pkg) return NULL;

  JSON_Value *root = json_parse_string(json);
  pkg->json = json_value_get_object(root);
  pkg->json_string = json;
  pkg->name = (char *) json_object_dotget_string(pkg->json, "name");
  pkg->repo = (char *) json_object_dotget_string(pkg->json, "repo");
  pkg->version = (char *) json_object_dotget_string(pkg->json, "version");
  pkg->license = (char *) json_object_dotget_string(pkg->json, "license");
  pkg->description = (char *) json_object_dotget_string(pkg->json, "description");
  pkg->install = (char *) json_object_dotget_string(pkg->json, "install");
  pkg->src = json_object_dotget_array(pkg->json, "src");
  pkg->keywords = json_object_dotget_array(pkg->json, "keywords");
  pkg->dependencies = json_object_dotget_object(pkg->json, "dependencies");
  return pkg;
}

/**
 * Build a `package` from the given `repo` at tag `version`
 */

package_t *package_from_repo(char *repo, char *version) {
  char *url = package_url(repo, version, "package.json");

  package_log("fetch", url);

  response_t *res = http_get(url);
  if (!res->ok) return NULL;

  package_t *pkg = package_from_json(res->text);

  free(url);
  free(res);

  return pkg;
}

/**
 * Get a URL for `file` from `repo` at `version`
 */

char *package_url(char *repo, char *version, char *file) {
  char *buf = malloc(sizeof(char) * 256);
  if (NULL == buf) return NULL;
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

int package_install(package_t *pkg, char *dir) {
  int rc;

  for (int i = 0; i < json_array_get_count(pkg->src); ++i) {
    char *file = (char *) json_array_get_string(pkg->src, i);
    char *path = path_join(dir, basename(file));
    char *url = package_url(pkg->repo, pkg->version, file);

    package_log("fetch", url);
    package_log("save", path);
    rc = http_get_file(url, path);
    free(file);
    free(path);
    free(url);
    if (-1 == rc) return -1;
  }

  // if deps are listed
  if (pkg->dependencies) {
    for (int i = 0; i < json_object_get_count(pkg->dependencies); ++i) {
      // iterate and install each of them
      char *name = (char *) json_object_get_name(pkg->dependencies, i);
      char *version = (char *) json_object_get_string(pkg->dependencies, name);
      package_t *dep = package_from_repo(name, version);
      rc = package_install(dep, dir);
      free(dep);
      free(name);
      free(version);
      if (-1 == rc) return -1;
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
  if (NULL == url || NULL == slug) return -1;

  char *tarball = malloc(sizeof(char) * 256);
  if (NULL == tarball) return -1;
  sprintf(tarball
    , "/tmp/%s"
    , slug);

  // save archive to /tmp
  if (-1 == http_get_file(url, tarball)) {
    return -1;
  }

  char *command = malloc(sizeof(char) * 256);
  int code;

  // cheap untar
  sprintf(command
    , "cd /tmp && tar -xf %s"
    , slug);

  code = system(command);
  if (0 != code) return code;

  // cheap install
  sprintf(command
    , "cd /tmp/%s-%s && %s"
    , pkg->name
    , pkg->version
    , pkg->install);

  return system(command);
}

/**
 * Get a URL for the `repo`'s tarball at `version`
 */

char *package_tarball(char *repo, char *version) {
  char *buf = malloc(sizeof(char) * 256);
  if (NULL == buf) return NULL;
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
  printf("%s: %s\n", "name", pkg->name);
  printf("%s: %s\n", "version", pkg->version);
  printf("%s: %s\n", "repo", pkg->repo);
  printf("%s: %s\n", "license", pkg->license);
  printf("%s: %s\n", "description", pkg->description);

  if (pkg->dependencies) {
    printf("%s:\n", "dependencies");
    for (int i = 0; i < json_object_get_count(pkg->dependencies); ++i) {
      char *name = (char *) json_object_get_name(pkg->dependencies, i);
      char *version = (char *) json_object_get_string(pkg->dependencies, name);
      printf("  %s: %s\n", name, version);
      free(name);
      free(version);
    }
  }

  printf("%s:\n", "src");
  for (int i = 0; i < json_array_get_count(pkg->src); ++i) {
    printf("  %s\n", json_array_get_string(pkg->src, i));
  }

  if (pkg->install) {
    printf("  %s: %s\n", "install", pkg->install);
  }
}
