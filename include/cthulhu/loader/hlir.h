#include "loader.h"

typedef struct {
    bool embedSource;
} save_settings_t;

vector_t *load_modules(reports_t *reports, const char *path);
void save_modules(reports_t *reports, save_settings_t *settings, vector_t *modules, const char *path);

bool is_hlir_module(const char *path);
