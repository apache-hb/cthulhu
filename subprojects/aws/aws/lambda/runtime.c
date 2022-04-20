#include "runtime.h"

#include "cJSON.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#define VERSION "2018-06-01"

typedef struct {
    char *data;
    size_t size;
} chunk_t;

static chunk_t new_chunk(size_t size) {
    chunk_t chunk;
    chunk.data = malloc(size);
    chunk.size = size;
    return chunk;
}

static void chunk_write(void *data, size_t size, size_t scale, void *user) {
    size_t total = size * scale;
    chunk_t *chunk = user;

    char *ptr = realloc(chunk->data, chunk->size + total + 1);
    memcpy(ptr + chunk->size, data, total);

    chunk->data = ptr;
    chunk->size += total;
    chunk->data[chunk->size] = '\0';
}

static char *formatv(const char *fmt, va_list args) {
    /* make a copy of the args for the second format */
    va_list again;
    va_copy(again, args);

    /* get the number of bytes needed to format */
    int len = vsnprintf(NULL, 0, fmt, args) + 1;

    char *out = malloc(len);

    vsnprintf(out, len, fmt, again);

    return out;
}

static char *format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *str = formatv(fmt, args);
    va_end(args);

    return str;
}

aws_error_t new_aws_runtime(aws_runtime_t *runtime, const char *endpoint, const char *cert) {
    CURL *curl = curl_easy_init();
    if (curl == NULL) { return AWS_CURL_FAILED; }

    if (cert != NULL) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, cert);
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
    
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    runtime->endpoint = endpoint;
    runtime->curl = curl;

    return AWS_OK;
}

void delete_aws_runtime(aws_runtime_t *runtime) {
    curl_easy_cleanup(runtime->curl);
}

aws_error_t aws_next_event(aws_runtime_t *runtime, aws_event_t *event) {
    char *url = format("http://%s/" VERSION "/runtime/invocation/next", runtime->endpoint);
    CURLcode res;
    chunk_t chunk = new_chunk(0x1000);

    curl_easy_setopt(runtime->curl, CURLOPT_URL, url);
    curl_easy_setopt(runtime->curl, CURLOPT_WRITEFUNCTION, chunk_write);
    curl_easy_setopt(runtime->curl, CURLOPT_WRITEDATA, &chunk);

    res = curl_easy_perform(runtime->curl);
    if (res != CURLE_OK) { return AWS_CURL_FAILED; }

    cJSON *json = cJSON_Parse(chunk.data);

    return AWS_OK;
}

aws_error_t aws_respond(aws_runtime_t *runtime, aws_event_t *event, const char *response) {
    char *url = format("http://%s/" VERSION "/runtime/invocation/%s/response", runtime->endpoint, event->request);
    CURLcode res;

    curl_easy_setopt(runtime->curl, CURLOPT_URL, url);
    curl_easy_setopt(runtime->curl, CURLOPT_POST, 1);

    curl_easy_setopt(runtime->curl, CURLOPT_POSTFIELDSIZE, strlen(response));
    curl_easy_setopt(runtime->curl, CURLOPT_POSTFIELDS, response);

    res = curl_easy_perform(runtime->curl);
    if (res != CURLE_OK) { return AWS_CURL_FAILED; }

    return AWS_OK;
}

aws_error_t aws_init_error(aws_runtime_t *runtime, const char *error, const char *type, const char **stack, size_t elements) {
    return AWS_OK;
}

aws_error_t aws_invoke_error(aws_runtime_t *runtime, const char *error, const char *type, const char **stack, size_t elements) {
    return AWS_OK;
}
