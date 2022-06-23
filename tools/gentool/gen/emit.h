#include "gen.h"

#include "platform/file.h"

typedef struct {
    reports_t *reports;
    const ast_t *root;

    const char *path;

    file_t header;
    file_t source;

    const char *id;
    const char *upperId;
} emit_t;

void emit(emit_t *config);
