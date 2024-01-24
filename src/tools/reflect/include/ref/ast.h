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
    eAstConstructor,
    eAstCase,
    eAstParam,

    eAstString,
    eAstInteger,
    eAstBool,

    eAstInstance,
    eAstPointer,
    eAstOpaque,
    eAstReference,

    eAstIdent,
    eAstBinary,
    eAstCompare,
    eAstUnary,

    eAstAttribConfig, // load from textual config
    eAstAttribAssert, // pre/post conditions on method
    eAstAttribDeprecated, // warn on use
    eAstAttribTypeId, // type id for serialization
    eAstAttribAlign, // alignment for serialization
    eAstAttribCxxName, // c++ name for implementation
    eAstAttribRemote, // enable rpc

    eAstAttribTag, // a tag attribute, one of ref_attrib_tag_t

    eAstCount
} ref_kind_t;

typedef enum ref_attrib_tag_t {
    eAttribTransient, // this field should not be serialized
    eAttribBitflags, // this enum is a bitflag type, define bitwise operators
    eAttribArithmatic, // this enum is an arithmatic type, define arithmatic operators
    eAttribIterator, // this enum is an iterator, define increment/decrement operators and generate begin/end functions
    eAttribOrdered, // allow order comparison of enums
    eAttribInternal, // dont generate reflection data for this type
    eAttribFacade, // this
    eAttribFacadeUnscopedEnum, // facade over a c style unscoped enum

    eAttribLayoutSystem, // match system abi

    // stable abi for serialization
    // TODO: what should we match, protobuf, flatbuffers, capnproto, etc?
    eAttribLayoutStable,

    // optimize for d3d cbuffer transfer
    // TODO: need math types for this
    eAttribLayoutCBuffer,
    eAttribLayoutPacked, // pack fields
    eAttribLayoutAny, // pick the best internal layout
} ref_attrib_tag_t;

typedef enum ref_privacy_t {
    ePrivacyDefault,
    ePrivacyPublic,
    ePrivacyPrivate,
    ePrivacyProtected,
    ePrivacyModule,

    ePrivacyCount
} ref_privacy_t;

typedef enum ref_flags_t {
    eDeclNone = 0,
    eDeclSealed = 1 << 1,
    eDeclVirtual = 1 << 2,
    eDeclConst = 1 << 3,
    eDeclPrivate = 1 << 4,
    eDeclProtected = 1 << 5,
    eDeclPublic = 1 << 6,
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
        /* eAstInteger */
        mpz_t integer;

        /* eAstIdent, eAstAttribCxxName */
        const char *ident;

        ref_attrib_tag_t tag;

        /* eAstBool */
        bool boolean;

        /* eAstString */
        text_t text;

        /* eAstProgram */
        struct {
            char *api;
            vector_t *mod;
            vector_t *imports;
            vector_t *decls;
        };

        /* eAstUnary */
        struct {
            unary_t unary;

            /* eAstAttribTypeId, eAstAttribAlignas */
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
            /* eAstPrivacy, eDeclField, eDeclMethod */
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
                    vector_t *methods;

                    union {
                        vector_t *fields;

                        /* eAstVariant */
                        struct {
                            ref_ast_t *default_case;
                            vector_t *cases;
                        };
                    };
                };

                /* eAstField, eAstConst, eAstParam */
                struct {
                    ref_param_t param;

                    /* eAstTypeAlias */
                    ref_ast_t *type;
                    ref_ast_t *initial;
                };

                /* eAstMethod */
                struct {
                    ref_ast_t *return_type;
                    vector_t *method_params;
                    ref_ast_t *body;
                };

                /* eAstCase */
                struct {
                    bool is_default;
                    ref_ast_t *value;
                };
            };
        };

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

ref_ast_t *ref_program(scan_t *scan, where_t where, vector_t *mod, char *api, vector_t *imports, vector_t *decls);

ref_ast_t *ref_import(scan_t *scan, where_t where, text_t text);

ref_ast_t *ref_class(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body);

ref_ast_t *ref_struct(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body);

ref_ast_t *ref_privacy(scan_t *scan, where_t where, ref_privacy_t privacy);

ref_ast_t *ref_field(scan_t *scan, where_t where, char *name, ref_ast_t *type, ref_ast_t *value);
ref_ast_t *ref_method(scan_t *scan, where_t where, ref_flags_t flags, char *name, vector_t *params, ref_ast_t *type, ref_ast_t *body);
ref_ast_t *ref_param(scan_t *scan, where_t where, char *name, ref_param_t param, ref_ast_t *type);
ref_ast_t *ref_ctor(scan_t *scan, where_t where, vector_t *params, ref_ast_t *body);

ref_ast_t *ref_instance(scan_t *scan, where_t where, ref_ast_t *type, vector_t *params);
ref_ast_t *ref_pointer(scan_t *scan, where_t where, ref_ast_t *type);
ref_ast_t *ref_reference(scan_t *scan, where_t where, ref_ast_t *type);

ref_ast_t *ref_variant(scan_t *scan, where_t where, char *name, ref_ast_t *underlying, vector_t *cases);

ref_ast_t *ref_case(scan_t *scan, where_t where, char *name, ref_ast_t *value, bool is_default);

ref_ast_t *ref_opaque(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_opaque_text(scan_t *scan, where_t where, text_t text);

ref_ast_t *ref_ident(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_integer(scan_t *scan, where_t where, mpz_t integer);
ref_ast_t *ref_string(scan_t *scan, where_t where, typevec_t *text);
ref_ast_t *ref_bool(scan_t *scan, where_t where, bool value);

ref_ast_t *ref_attrib_transient(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_deprecated(scan_t *scan, where_t where, typevec_t *message);
ref_ast_t *ref_attrib_typeid(scan_t *scan, where_t where, ref_ast_t *expr);
ref_ast_t *ref_attrib_alignas(scan_t *scan, where_t where, ref_ast_t *expr);
ref_ast_t *ref_attrib_cxxname(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_attrib_remote(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_rename(scan_t *scan, where_t where, char *ident);

ref_ast_t *ref_attrib_tag(scan_t *scan, where_t where, ref_attrib_tag_t tag);

ref_ast_t *ref_attrib_assert(scan_t *scan, where_t where, typevec_t *before, typevec_t *after, typevec_t *always);

void ref_set_attribs(ref_ast_t *ast, vector_t *attributes);
void ref_set_flags(ref_ast_t *ast, ref_flags_t flags);

END_API
