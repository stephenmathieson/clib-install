
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "fs.h"
#include "path-join.h"
#include "package.h"


void test_package_from_json() {
  char json[] = "{"
                "  \"name\": \"foo\","
                "  \"version\": \"1.0.0\","
                "  \"repo\": \"foobar/foo\","
                "  \"license\": \"mit\","
                "  \"description\": \"lots of foo\","
                "  \"src\": ["
                "    \"foo.h\","
                "    \"foo.c\""
                "  ]"
                "}\0";
  package_t *pkg = package_from_json(json);
  assert(pkg);
  assert(0 == strcmp(json, pkg->json_string));
  assert(0 == strcmp("foo", pkg->name));
  assert(0 == strcmp("1.0.0", pkg->version));
  assert(0 == strcmp("foobar/foo", pkg->repo));
  assert(0 == strcmp("mit", pkg->license));
  assert(0 == strcmp("lots of foo", pkg->description));
  assert(0 == strcmp("foo.h", json_array_get_string(pkg->src, 0)));
  assert(0 == strcmp("foo.c", json_array_get_string(pkg->src, 1)));
  assert(NULL == pkg->dependencies);
  free(pkg);
}

void test_package_url() {
  char *url = package_url("foobar/foo", "1.0.0", "package.json");
  assert(0 == strcmp("https://raw.github.com/foobar/foo/1.0.0/package.json", url));
  free(url);
}

void test_package_from_repo() {
  package_t *pkg = package_from_repo("stephenmathieson/case.c", "0.1.0");
  assert(pkg);
  assert(0 == strcmp("case", pkg->name));
  assert(0 == strcmp("0.1.0", pkg->version));
  assert(0 == strcmp("stephenmathieson/case.c", pkg->repo));
  assert(0 == strcmp("MIT", pkg->license));
  assert(0 == strcmp("String case conversion utility", pkg->description));
  assert(0 == strcmp("src/case.c", json_array_get_string(pkg->src, 0)));
  assert(0 == strcmp("src/case.h", json_array_get_string(pkg->src, 1)));
  assert(2 == json_array_get_count(pkg->src));
  assert(0 == strcmp("string", json_array_get_string(pkg->keywords, 0)));
  assert(0 == strcmp("uppercase", json_array_get_string(pkg->keywords, 1)));
  assert(0 == strcmp("lowercase", json_array_get_string(pkg->keywords, 2)));
  assert(3 == json_array_get_count(pkg->keywords));
  free(pkg);
}

void test_package_install() {
  package_t *pkg = package_from_repo("clibs/buffer", "0.2.0");
  assert(pkg);
  assert(0 == package_install(pkg, "./test/fixtures"));
  assert(-1 == fs_exists("./test/fixtures/package.json"));
  assert(0 == fs_exists("./test/fixtures/buffer.c"));
  assert(0 == fs_exists("./test/fixtures/buffer.h"));
  free(pkg);
}

void test_package_install_basename() {
  package_t *pkg = package_from_repo("stephenmathieson/case.c", "0.1.0");
  assert(pkg);
  // ensure we've got the right pkg
  assert(0 == strcmp("src/case.c", json_array_get_string(pkg->src, 0)));
  assert(0 == strcmp("src/case.h", json_array_get_string(pkg->src, 1)));

  assert(0 == package_install(pkg, "./test/fixtures"));
  // verify we're `basename`-ing the source files
  assert(0 == fs_exists("./test/fixtures/case.c"));
  assert(0 == fs_exists("./test/fixtures/case.h"));
  free(pkg);
}

void test_package_install_binary() {
  package_t *pkg = package_from_repo("stephenmathieson/tabs-to-spaces", "master");
  assert(pkg);
  assert(0 == package_install_binary(pkg));
  free(pkg);
}

int main() {
  test_package_from_json();
  test_package_url();
  test_package_from_repo();
  test_package_install();
  test_package_install_basename();
  test_package_install_binary();
  return 0;
}
