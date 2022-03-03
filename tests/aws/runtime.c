#include "aws/runtime.h"

#include <stdbool.h>
#include <assert.h>

// argv[0] = executable name
// argv[1] = $AWS_LAMBDA_RUNTIME_API
int main(int argc, const char **argv) {
    aws_runtime_t runtime;
    aws_event_t event;
    aws_error_t error;

    assert(argc > 1);

    error = new_aws_runtime(&runtime, argv[1], NULL);
    if (error != AWS_OK) {
        aws_init_error(&runtime, "new-aws-runtime", "runtime.init", NULL, 0);
        return 1;
    }

    while (true) {
        error = aws_next_event(&runtime, &event);
        if (error != AWS_OK) {
            aws_invoke_error(&runtime, "aws-next-event", "runtime.next", NULL, 0);
            return 1;
        }

        aws_respond(&runtime, &event, "{ \"status\": \"ok\" }");
    }

    return 0;
}
