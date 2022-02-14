#include "aws/runtime.h"

#include <stdlib.h>

int main(int argc, const char **argv) {
    aws_runtime_t runtime;
    aws_event_t event;
    aws_error_t error;
    const char *endpoint;

    endpoint = getenv("AWS_LAMBDA_RUNTIME_API");
    error = new_aws_runtime(&runtime, endpoint);
    if (error != AWS_OK) { return 1; }

    error = aws_next_event(&runtime, &event);
    if (error != AWS_OK) { return 1; }

    error = aws_respond(&runtime, "Hello, world!");
    if (error != AWS_OK) { return 1; }

    delete_aws_runtime(&runtime);
}
