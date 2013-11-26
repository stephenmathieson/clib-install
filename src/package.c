
#include <stdio.h>
#include <stdlib.h>
#include "http-get.h"
#include "fs.h"
#include "parson.h"
#include "path-join.h"
#include "package.h"


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

  char *package_json = path_join(dir, "package.json");
  rc = fs_write(package_json, pkg->json_string);
  free(package_json);
  if (-1 == rc) {
    return -1;
  }

  for (int i = 0; i < json_array_get_count(pkg->src); ++i) {
    char *file = (char *) json_array_get_string(pkg->src, i);
    char *path = path_join(dir, file);
    char *url = package_url(pkg->repo, pkg->version, file);

    response_t *res = http_get(url);
    if (!res->ok) return -1;
    if (-1 == fs_write(path, res->text)) return -1;
    free(file);
    free(path);
    free(url);
    free(res);
  }

  return 0;
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
}
