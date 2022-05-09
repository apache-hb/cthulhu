#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/map.h"
#include "cthulhu/util/report.h"

typedef struct ssa_step_t
{
    const node_t *node; ///< the node that created this step
    const hlir_t *type; ///< the resulting type of this step
} ssa_step_t;

typedef struct ssa_flow_t
{
    struct ssa_module_t *parent; ///< the module this flow is *implemented* in

    const node_t *node; ///< where this flow is implemented
    const hlir_t *type; ///< the type signature of this flow

    ssa_step_t *steps; ///< the steps in this flow
    size_t usedSteps;  ///< the number of used steps in this flow
    size_t totalSteps; ///< the number of total steps in this flow
} ssa_flow_t;

typedef struct ssa_module_t
{
    const char *moduleName; ///< name of this module
    const node_t *node;     ///< node that this module is associated with

    map_t *modules; ///< modules that this module depends on
                    ///  these can be recursive

    map_t *functions; ///< the functions in this module
    map_t *globals;   ///< the globals in this module
} ssa_module_t;

typedef struct
{
    bool globalsMustBeConst; ///< reduce all globals down to constants
                             ///  fail if globals cannot be made constant
} ssa_settings_t;

vector_t *ssa_compile_modules(reports_t *reports, vector_t *modules, const ssa_settings_t *settings);
