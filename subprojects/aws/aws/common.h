#pragma once

#include <stddef.h>
#include <curl/curl.h>

typedef struct {
    const char *endpoint;
    CURL *curl;
} aws_runtime_t;

typedef enum {
    AWS_OK,
    AWS_CURL_FAILED
} aws_error_t;

aws_error_t new_aws_runtime(aws_runtime_t *runtime, const char *endpoint, const char *cert);
void delete_aws_runtime(aws_runtime_t *runtime);
