#pragma once

#include "cthulhu/mediator/common.h"

typedef struct io_t io_t;
typedef struct vector_t vector_t;
typedef struct reports_t reports_t;

// mediator api

/**
 * @brief create the global mediator object, must be the first part of cthulhu called
 * 
 * @param id the name of the mediator
 * @param version the version of the mediator
 * @return mediator_t* the mediator object
 */
mediator_t *mediator_new(const char *id, version_info_t version);


// lifetime api

lifetime_t *lifetime_new(mediator_t *mediator);

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang);

const language_t *lifetime_get_language(lifetime_t *lifetime, const char *ext);

void lifetime_parse(lifetime_t *lifetime, const language_t *lang, io_t *io);

void lifetime_run_stage(lifetime_t *lifetime, compile_stage_t stage);

vector_t *lifetime_get_modules(lifetime_t *lifetime);
