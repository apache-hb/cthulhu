#pragma once

#include "aws/common.h"

typedef struct {
    const char *request;
    size_t deadline;
    const char *arn;
    const char *trace;
    const char *context;
    const char *identity;
} aws_event_t;

///
/// functions to interact with the runtime api
/// https://docs.aws.amazon.com/lambda/latest/dg/runtimes-api.html
///

/**
 * @brief /runtime/invocation/next
 * 
 * @param runtime the runtime to use
 * @param event the event to receive
 * @return an error code
 */
aws_error_t aws_next_event(aws_runtime_t *runtime, aws_event_t *event);

/**
 * @brief /runtime/invocation/AwsRequestId/response
 * 
 * @param runtime the runtime to use
 * @param event the event to respond to
 * @param response the response to send
 * @return an error code 
 */
aws_error_t aws_respond(aws_runtime_t *runtime, aws_event_t *event, const char *response);

/**
 * @brief /runtime/init/error
 * 
 * @param runtime the runtime to use
 * @param error the error message
 * @param type the type of the error
 * @param stack a stack trace
 * @param elements the number of elements in the stack trace
 * @return an error code 
 */
aws_error_t aws_init_error(aws_runtime_t *runtime, const char *error, const char *type, const char **stack, size_t elements);

/**
 * @brief /runtime/invocation/AwsRequestId/error
 * 
 * @param runtime the runtime to use
 * @param error the error message
 * @param type the type of the error
 * @param stack a stack trace
 * @param elements the number of elements in the stack trace
 * @return an error code
 */
aws_error_t aws_invoke_error(aws_runtime_t *runtime, const char *error, const char *type, const char **stack, size_t elements);
