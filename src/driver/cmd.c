#include "cmd.h"

static vector_t *sources = NULL;
static vector_t *objects = NULL;

void add_file(const char *file);
void init_cmd(void);
