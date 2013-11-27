
#ifndef _PACKAGE_H_
#define _PACKAGE_H_ 1

#include "parson.h"

typedef struct {
  char *name;
  char *version;
  char *repo;
  char *description;
  char *license;
  char *install;
  JSON_Object *dependencies;
  JSON_Array *src;
  JSON_Array *keywords;
  JSON_Object *json;
  char *json_string;
} package_t;

package_t *package_from_json(char *json);

package_t *package_from_repo(char *repo, char *version);

char *package_url(char *repo, char *version, char *file);

char *package_tarball(char *repo, char *vesrion);

int package_install(package_t *pkg, char *dir);

int package_install_binary(package_t *pkg);

void package_inspect(package_t *pkg);

#endif
