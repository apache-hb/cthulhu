#pragma once

#include <gmp.h>

#include "core/compiler.h"

#include "core/text.h"
#include "core/where.h"
#include "cthulhu/tree/ops.h"

#include <stdbool.h>

typedef struct vector_t vector_t;

typedef struct scan_t scan_t;
typedef struct node_t node_t;
typedef struct typevec_t typevec_t;

BEGIN_API

typedef struct ref_ast_t ref_ast_t;

typedef enum ref_kind_t {
    eAstProgram,
    eAstImport,

    eAstClass,
    eAstVariant,
    eAstStruct,
    eAstAlias,

    eAstConst,

    eAstArray,
    eAstVector,

    eAstPrivacy,
    eAstField,
    eAstMethod,
    eAstCase,
    eAstParam,

    eAstString,
    eAstInteger,
    eAstBool,

    eAstInstance,
    eAstName,
    eAstPointer,
    eAstOpaque,

    eAstIdent,
    eAstDigit,
    eAstBinary,
    eAstCompare,
    eAstUnary,

    eAstAttribTransient, // dont serialize field
    eAstAttribConfig, // load from textual config
    eAstAttribAssert, // pre/post conditions on method
    eAstAttribDeprecated, // warn on use
    eAstAttribTypeId, // type id for serialization
    eAstAttribLayout, // layout for serialization
    eAstAttribAlign, // alignment for serialization
    eAstAttribBitflags, // bitflags enum
    eAstAttribArithmatic, // arithmatic enum
    eAstAttribIterator, // iterator enum
    eAstAttribCxxName, // c++ name for implementation
    eAstAttribRemote, // enable rpc
    eAstAttribNoReflect, // dont reflect

    eAstCount
} ref_kind_t;

typedef enum ref_privacy_t {
    ePrivacyDefault,
    ePrivacyPublic,
    ePrivacyPrivate,
    ePrivacyProtected,
    ePrivacyModule,

    ePrivacyCount
} ref_privacy_t;

typedef enum ref_layout_t {
    eLayoutOptimal, // optimize for speed
    eLayoutPacked, // optimize for size
    eLayoutSystem, // use system default

    eLayoutCount
} ref_layout_t;

typedef enum ref_flags_t {
    eDeclNone = 0,
    eDeclExported = 1 << 0,
    eDeclTransient = 1 << 1,
    eDeclVirtual = 1 << 2,
    eDeclConst = 1 << 3,
    eDeclBitFlags = 1 << 4,
    eDeclDefault = 1 << 5,
} ref_flags_t;

typedef enum ref_param_t {
    eParamIn, // passed by value in whatever way is most efficient
    eParamOut, // passed by reference
    eParamInOut, // passed by reference, but may be modified

    eParamCount
} ref_param_t;

typedef struct ref_ast_t {
    ref_kind_t kind;
    const node_t *node;

    union {
        /* eAstDigit */
        mpz_t digit;

        /* eAstIdent, eAstName */
        char *ident;

        /* eAstBool */
        bool boolean;

        /* eAstString */
        text_t text;

        /* eAstInteger */
        mpz_t integer;

        mpz_t align;

        ref_layout_t layout;

        /* eAstProgram */
        struct {
            vector_t *mod;
            vector_t *imports;
            vector_t *decls;
        };

        /* eAstUnary */
        struct {
            unary_t unary;
            struct ref_ast_t *expr;
        };

        struct {
            union {
                binary_t binary;
                compare_t compare;
            };
            struct ref_ast_t *lhs;
            struct ref_ast_t *rhs;
        };

        /* eAstModule, eAstImport, eAstAttribConfig */
        vector_t *path;

        /* eAstInstance */
        struct {
            ref_ast_t *generic;
            vector_t *params;
        };

        struct ref_ast_t *ptr;

        struct {
            /* eAstPrivacy */
            ref_privacy_t privacy;

            /* eAstClass, eAstStruct, eAstVariant */
            ref_flags_t flags;
            vector_t *attributes;
            vector_t *tparams;
            char *name;

            union {
                /* eAstClass, eAstStruct */
                struct {
                    ref_ast_t *parent;
                    vector_t *fields;
                    vector_t *methods;
                };

                /* eAstVariant */
                struct {
                    ref_ast_t *underlying;
                    ref_ast_t *default_case;
                    vector_t *cases;
                };

                /* eAstField, eAstConst, eAstParam */
                struct {
                    ref_param_t param;
                    ref_ast_t *type;
                    ref_ast_t *initial;
                };

                /* eAstMethod */
                struct {
                    ref_ast_t *return_type;
                    vector_t *method_params;
                    ref_ast_t *body;
                    bool const_method;
                };

                /* eAstCase */
                struct {
                    bool is_default;
                    mpz_t value;
                };
            };
        };

        /* eAstAttribTransient */
        /* empty */

        /* eAstAttribTypeId */
        mpz_t id;

        /* eAstAttribDeprecated */
        char *message;

        /* eAstAttribAssert */
        struct {
            vector_t *before;
            vector_t *after;
            vector_t *always;
        };
    };
} ref_ast_t;

ref_ast_t *ref_unary(scan_t *scan, where_t where, unary_t op, ref_ast_t *expr);
ref_ast_t *ref_binary(scan_t *scan, where_t where, binary_t op, ref_ast_t *lhs, ref_ast_t *rhs);
ref_ast_t *ref_compare(scan_t *scan, where_t where, compare_t op, ref_ast_t *lhs, ref_ast_t *rhs);

ref_ast_t *ref_program(scan_t *scan, where_t where, vector_t *mod, vector_t *imports, vector_t *decls);

ref_ast_t *ref_import(scan_t *scan, where_t where, text_t text);

ref_ast_t *ref_class(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body);

ref_ast_t *ref_struct(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body);

ref_ast_t *ref_privacy(scan_t *scan, where_t where, ref_privacy_t privacy);

ref_ast_t *ref_name(scan_t *scan, where_t where, char *ident);

ref_ast_t *ref_field(scan_t *scan, where_t where, char *name, ref_ast_t *type, ref_ast_t *value);
ref_ast_t *ref_method(scan_t *scan, where_t where, bool const_method, char *name, vector_t *params, ref_ast_t *type, ref_ast_t *body);
ref_ast_t *ref_param(scan_t *scan, where_t where, char *name, ref_param_t param, ref_ast_t *type);

ref_ast_t *ref_instance(scan_t *scan, where_t where, ref_ast_t *type, vector_t *params);
ref_ast_t *ref_pointer(scan_t *scan, where_t where, ref_ast_t *type);

ref_ast_t *ref_variant(scan_t *scan, where_t where, char *name, ref_ast_t *underlying, vector_t *cases);

ref_ast_t *ref_case(scan_t *scan, where_t where, char *name, mpz_t value, bool is_default);

ref_ast_t *ref_opaque(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_opaque_text(scan_t *scan, where_t where, text_t text);


ref_ast_t *ref_integer(scan_t *scan, where_t where, mpz_t digit);
ref_ast_t *ref_string(scan_t *scan, where_t where, typevec_t *text);
ref_ast_t *ref_bool(scan_t *scan, where_t where, bool value);

ref_ast_t *ref_attrib_transient(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_deprecated(scan_t *scan, where_t where, typevec_t *message);
ref_ast_t *ref_attrib_typeid(scan_t *scan, where_t where, mpz_t id);
ref_ast_t *ref_attrib_layout(scan_t *scan, where_t where, ref_layout_t layout);
ref_ast_t *ref_attrib_alignas(scan_t *scan, where_t where, mpz_t align);
ref_ast_t *ref_attrib_bitflags(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_arithmatic(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_iterator(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_cxxname(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_attrib_remote(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_noreflect(scan_t *scan, where_t where);

ref_ast_t *ref_attrib_assert(scan_t *scan, where_t where, typevec_t *before, typevec_t *after, typevec_t *always);

void ref_set_attribs(ref_ast_t *ast, vector_t *attributes);
void ref_set_export(ref_ast_t *ast, bool exported);

END_API