
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
                "}";
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

void test_package_install_dependencies() {
  package_t *pkg = package_from_repo("stephenmathieson/mkdirp.c", "master");
  assert(pkg);
  // ensure we've got the right pkg
  assert(0 == strcmp("src/mkdirp.c", json_array_get_string(pkg->src, 0)));
  assert(0 == strcmp("src/mkdirp.h", json_array_get_string(pkg->src, 1)));

  assert(0 == package_install_dependencies(pkg, "./test/fixtures/mkdirp-deps"));
  // sources should not exist
  assert(-1 == fs_exists("./test/fixtures/mkdirp-deps/mkdirp.c"));
  assert(-1 == fs_exists("./test/fixtures/mkdirp-deps/mkdirp.h"));
  // deps
  assert(0 == fs_exists("./test/fixtures/mkdirp-deps/path-normalize.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-deps/path-normalize.h"));
  free(pkg);
}


void test_package_install_neseted_dependencies() {
  package_t *pkg = package_from_repo("stephenmathieson/rimraf.c", "temp-deps");
  assert(pkg);
  // ensure we've got the right pkg
  assert(0 == strcmp("src/rimraf.c", json_array_get_string(pkg->src, 0)));
  assert(0 == strcmp("src/rimraf.h", json_array_get_string(pkg->src, 1)));

  assert(0 == package_install(pkg, "./test/fixtures"));
  // sources
  assert(0 == fs_exists("./test/fixtures/rimraf.c"));
  assert(0 == fs_exists("./test/fixtures/rimraf.h"));
  // deps
  assert(0 == fs_exists("./test/fixtures/path-join.h"));
  assert(0 == fs_exists("./test/fixtures/path-join.c"));
  // deps' deps
  assert(0 == fs_exists("./test/fixtures/str-ends-with.h"));
  assert(0 == fs_exists("./test/fixtures/str-ends-with.c"));
  assert(0 == fs_exists("./test/fixtures/str-starts-with.h"));
  assert(0 == fs_exists("./test/fixtures/str-starts-with.c"));
  free(pkg);
}

void test_package_install_specified_version() {
  package_t *pkg = package_from_repo("stephenmathieson/mkdirp.c", "temp-deps");
  assert(pkg);

  assert(0 == package_install(pkg, "./test/fixtures/mkdirp-temp-deps"));
  // sources
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/mkdirp.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/mkdirp.h"));
  // deps
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/str-copy.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/str-copy.h"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/path-normalize.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-temp-deps/path-normalize.h"));
  free(pkg);

}

void test_package_install_creates_dir() {
  package_t *pkg = package_from_repo("stephenmathieson/mkdirp.c", "master");
  assert(pkg);

  assert(0 == package_install(pkg, "./test/fixtures/mkdirp"));
  // sources
  assert(0 == fs_exists("./test/fixtures/mkdirp/mkdirp.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp/mkdirp.h"));
  // deps
  assert(0 == fs_exists("./test/fixtures/mkdirp/path-normalize.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp/path-normalize.h"));
  free(pkg);
}

void test_package_from_repo_404() {
  package_t *pkg = package_from_repo("stephenmathieson/notreal.c", "4.3.2");
  assert(NULL == pkg);
}

void test_package_from_json_broken_json() {
  char json[] = "{"
                "  ,"
                "}";
  package_t *pkg = package_from_json(json);
  assert(NULL == pkg);
}

void test_package_from_repo_version_star() {
  package_t *pkg = package_from_repo("stephenmathieson/mkdirp.c", "*");
  assert(pkg);
  assert(0 == strcmp("master", pkg->version));

  assert(0 == package_install(pkg, "./test/fixtures/mkdirp-star"));
  // sources
  assert(0 == fs_exists("./test/fixtures/mkdirp-star/mkdirp.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-star/mkdirp.h"));
  // deps
  assert(0 == fs_exists("./test/fixtures/mkdirp-star/path-normalize.c"));
  assert(0 == fs_exists("./test/fixtures/mkdirp-star/path-normalize.h"));
  free(pkg);

}


int main() {
  test_package_from_json();
  test_package_url();
  test_package_from_repo();
  test_package_from_repo_404();
  test_package_install();
  test_package_install_basename();
  test_package_install_binary();
  test_package_install_dependencies();
  test_package_install_neseted_dependencies();
  test_package_install_creates_dir();
  test_package_install_specified_version();
  test_package_from_json_broken_json();
  test_package_from_repo_version_star();
  return 0;
}
