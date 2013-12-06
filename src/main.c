
#include "fs.h"
#include "package.h"

int main(int argc, char const *argv[]) {

  if (1 == argc) {
    if (-1 == fs_exists("./package.json")) {
      fprintf(stderr, "Missing package.json\n");
      return 1;
    }
    printf("install local deps\n");
  } else {
    for (int i = 1; i < argc; i++) {
      printf("install %s\n", argv[i]);
    }
  }

  return 0;
}
