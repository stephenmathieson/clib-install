
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "http-get.h"

/**
 * HTTP GET write callback
 */

static void http_get_cb(void *buf, size_t size, size_t n, void *ptr) {
  char **res_ptr = (char **) ptr;
  *res_ptr = strndup(buf, (size_t)(size * n));
}

/**
 * Perform an HTTP(S) GET on `url`
 */

response_t *http_get(char *url) {
  CURL *req = curl_easy_init();

  response_t *res = malloc(sizeof(response_t));
  if (NULL == res) return NULL;

  curl_easy_setopt(req, CURLOPT_URL, url);
  curl_easy_setopt(req, CURLOPT_HTTPGET, 1);

  curl_easy_setopt(req, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, http_get_cb);
  curl_easy_setopt(req, CURLOPT_WRITEDATA, &res->text);

  int c = curl_easy_perform(req);

  curl_easy_getinfo(req, CURLINFO_RESPONSE_CODE, &res->status);
  res->ok = (200 == res->status && CURLE_ABORTED_BY_CALLBACK != c) ? 1 : 0;
  curl_easy_cleanup(req);

  return res;
}


/**
 * HTTP GET file write callback
 */

static size_t http_get_file_cb(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite(ptr, size, nmemb, stream);
}

/**
 * Request `url` and save to `file`
 */

int http_get_file(char *url, char *file) {
  CURL *req = curl_easy_init();
  if (!req) return -1;

  FILE *fp = fopen(file, "wb");
  if (!fp) return -1;

  curl_easy_setopt(req, CURLOPT_URL, url);
  curl_easy_setopt(req, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, http_get_file_cb);
  curl_easy_setopt(req, CURLOPT_WRITEDATA, fp);
  int res = curl_easy_perform(req);

  long status;
  curl_easy_getinfo(req, CURLINFO_RESPONSE_CODE, &status);

  curl_easy_cleanup(req);
  fclose(fp);

  if (200 == status && CURLE_ABORTED_BY_CALLBACK != res) {
    return 0;
  }

  return -1;
}
