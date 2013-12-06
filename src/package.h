
#ifndef _PACKAGE_H_
#define _PACKAGE_H_ 1

#include "parson.h"

typedef struct {
  const char *name;
  const char *version;
  const char *repo;
  const char *description;
  const char *license;
  const char *install;
  JSON_Object *dependencies;
  JSON_Array *src;
  JSON_Array *keywords;
  JSON_Object *json;
  const char *json_string;
} package_t;

package_t *package_from_json(const char *json);

package_t *package_from_repo(const char *repo, const char *version);

char *package_url(const char *repo, const char *version, const char *file);

char *package_tarball(const char *repo, const char *vesrion);

int package_install(package_t *pkg, const char *dir);

int package_install_dependencies(package_t *pkg, const char *dir);

int package_install_binary(package_t *pkg);

void package_inspect(package_t *pkg);

#endif
