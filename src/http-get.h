
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_ 1

typedef struct {
  char *text;
  long status;
  int ok;
} response_t;

response_t *http_get(char *url);

int http_get_file(char *url, char *file);

#endif
