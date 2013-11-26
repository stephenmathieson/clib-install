
#include <assert.h>
#include "http-get.h"

int main() {
  response_t *res = http_get("http://google.com");
  assert(res);
  assert(res->text);
  assert(res->ok);
  assert(200 == res->status);
}
