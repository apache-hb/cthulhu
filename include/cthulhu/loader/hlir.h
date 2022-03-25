#include "loader.h"

typedef struct {
    bool embed_source;
} save_settings_t;

/**
 * @brief attempt to load a module from a given file
 * 
 * @param reports report sink
 * @param path the path to the file
 * @return hlir_t* the module if it was found
 */
hlir_t *load_module(reports_t *reports, const char *path);

/**
 * @brief save a module to a file
 * 
 * @param reports report sink
 * @param module the module to save
 * @param path where to save the module
 */
void save_module(reports_t *reports, save_settings_t *settings, hlir_t *module, const char *path);
