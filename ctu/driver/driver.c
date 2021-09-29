#include "driver.h"

#include "ctu/util/str.h"

#include <string.h>

#include "ctu/frontend/pl0/driver.h"
#include "ctu/frontend/ctu/driver.h"
#include "ctu/frontend/c11/driver.h"

#include "ctu/backend/c99/c99.h"
#include "ctu/backend/gcc/gcc.h"
#include "ctu/backend/llvm/llvm.h"
#include "ctu/backend/null/null.h"

const frontend_t FRONTEND_PL0 = {
    .version = "0.0.1",
    .name = "PL/0",
    .parse = (parse_t)pl0_parse,
    .analyze = (analyze_t)pl0_analyze
};

const frontend_t FRONTEND_CTU = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .parse = (parse_t)ctu_parse,
    .analyze = (analyze_t)ctu_analyze
};

const frontend_t FRONTEND_C11 = {
    .version = "0.0.1",
    .name = "C",
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};

const backend_t BACKEND_LLVM = {
    .version = "0.0.1",
    .name = "LLVM",
    .compile = NULL //(compile_t)llvm_build
};

const backend_t BACKEND_C99 = {
    .version = "0.0.1",
    .name = "C99",
    .compile = (compile_t)c99_build
};

const backend_t BACKEND_GCCJIT = {
    .version = "0.0.1",
    .name = "GCCJIT",
    .compile = NULL //(compile_t)gccjit_build
};

const backend_t BACKEND_NULL = {
    .version = "1.0.0",
    .name = "NULL",
    .compile = (compile_t)null_build
};

const frontend_t *select_frontend(reports_t *reports, const char *name) {
    if (name == NULL) {
        report(reports, ERROR, NULL, "no frontend specified");
        return NULL;
    }

    if (streq(name, "pl0")) {
        return &FRONTEND_PL0;
    } else if (streq(name, "ctu")) {
        return &FRONTEND_CTU;
    } else if (streq(name, "c11")) {
        return &FRONTEND_C11;
    } else {
        report(reports, ERROR, NULL, "unknown frontend: %s", name);
        return NULL;
    }
}

const frontend_t *select_frontend_by_extension(reports_t *reports, const frontend_t *frontend, const char *path) {
    if (frontend != NULL) {
        return frontend;
    }

    if (endswith(path, ".pl0")) {
        return &FRONTEND_PL0;
    } else if (endswith(path, ".ct")) {
        return &FRONTEND_CTU;
    } else if (endswith(path, ".c")) {
        return &FRONTEND_C11;
    } else {
        report(reports, ERROR, NULL, "unknown extension on input: %s", path);
        return NULL;
    }
}

const backend_t *select_backend(reports_t *reports, const char *name) {
    if (name == NULL) {
        report(reports, ERROR, NULL, "no backend specified");
    }

    if (streq(name, "c99")) {
        return &BACKEND_C99;
    } else if (streq(name, "gccjit")) {
        return &BACKEND_GCCJIT;
    } else if (streq(name, "llvm")) {
        return &BACKEND_LLVM;
    } else if (streq(name, "null")) {
        return &BACKEND_NULL;
    } else {
        report(reports, ERROR, NULL, "unknown backend: %s", name);
        return NULL;
    }
}
