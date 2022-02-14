#pragma once

#include <stddef.h>
#include <curl/curl.h>

typedef struct {
    const char *endpoint;
    CURL *curl;
} aws_runtime_t;

typedef struct {
    const char *request;
    size_t deadline;
    const char *arn;
    const char *trace;
    const char *context;
    const char *identity;
} aws_event_t;

typedef enum {
    AWS_OK,
    AWS_CURL_FAILED
} aws_error_t;

aws_error_t new_aws_runtime(aws_runtime_t *runtime, const char *endpoint);
void delete_aws_runtime(aws_runtime_t *runtime);

aws_error_t aws_next_event(aws_runtime_t *runtime, aws_event_t *event);
aws_error_t aws_respond(aws_runtime_t *runtime, aws_event_t *event, const char *response);
