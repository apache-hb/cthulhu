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
typedef struct map_t map_t;

CT_BEGIN_API

typedef struct ref_ast_t ref_ast_t;

typedef enum ref_kind_t
{
    eAstProgram,
    eAstImport,

    eAstClass,
    eAstVariant,
    eAstStruct,
    eAstAlias,
    eAstUnion,
    eAstUnionField,

    eAstPrivacy,
    eAstField,
    eAstMethod,
    eAstConstructor,
    eAstCase,
    eAstParam,

    eAstString,
    eAstInteger,
    eAstBool,

    eAstPointer,   // a pointer
    eAstOpaque,    // an opaque type, for stuff external to the reflection system
    eAstReference, // a reference, c++ semantics

    eAstSpan,
    eAstArray,
    eAstVector,
    eAstConst,

    eAstIdent,
    eAstBinary,
    eAstCompare,
    eAstUnary,

    eAstAttribConfig,     // load from textual config
    eAstAttribAssert,     // pre/post conditions on method
    eAstAttribDeprecated, // warn on use
    eAstAttribTypeId,     // type id for serialization
    eAstAttribAlign,      // alignment for serialization
    eAstAttribRemote,     // enable rpc
    eAstAttribDocs,       // documentation

    eAstAttribTag,    // a tag attribute, one of ref_attrib_tag_t
    eAstAttribString, // an attribute that has a single string argument

    eAstConfig, // field config

    eAstCount
} ref_kind_t;

typedef enum ref_attrib_tag_t
{
    eAttribTransient,  // this field should not be serialized
    eAttribBitflags,   // this enum is a bitflag type, define bitwise operators
    eAttribArithmatic, // this enum is an arithmatic type, define arithmatic operators
    eAttribIterator, // this enum is an iterator, define increment/decrement operators and generate
                     // begin/end functions
    eAttribOrdered,  // allow order comparison of enums
    eAttribInternal, // dont generate reflection data for this type

    // this is a synthetic type, it pretends a type exists when it does not
    // for wrapping things like WM_* or SW_* macros and the like
    // providing an opaque parent type allows wrapping externally defined
    // c style enums
    eAttribFacade,

    // this enum is a lookup key into a table
    // define min and max constants, and implicit (checked) conversion to the key type
    eAttribLookupKey,

    // this wraps an external type, which is defined, but not by us
    eAttribExternal,

    // match system abi
    eAttribLayoutSystem,

    // stable abi for persistent serialization
    // TODO: what should we match, protobuf, flatbuffers, capnproto, etc?
    eAttribLayoutStable,

    // optimize for d3d cbuffer transfer
    // TODO: need math types for this
    eAttribLayoutCBuffer,

    // pack fields
    eAttribLayoutPacked,

    // d3d12 input elements
    eAttribLayoutInput,

    // pick the best internal layout
    eAttribLayoutAny,

    ///
    /// these have a string arg
    ///

    eAttribCxxName, // c++ name for implementation
    eAttribFormat,  // string name
} ref_attrib_tag_t;

typedef enum ref_config_tag_t
{
    eRefConfigSerialize, // type to use for serialization
    eRefConfigApi,       // declspec dllimport/dllexport stuff
    eRefConfigArray,
    eRefConfigVector,
    eRefConfigSpan,

    eRefConfigCount
} ref_config_tag_t;

typedef enum ref_privacy_t
{
    ePrivacyDefault,
    ePrivacyPublic,
    ePrivacyPrivate,
    ePrivacyProtected,
    ePrivacyModule,

    ePrivacyCount
} ref_privacy_t;

typedef enum ref_flags_t
{
    eDeclNone = 0,
    eDeclSealed = 1 << 1,
    eDeclVirtual = 1 << 2,
    eDeclConst = 1 << 3,
    eDeclPrivate = 1 << 4,
    eDeclProtected = 1 << 5,
    eDeclPublic = 1 << 6,
} ref_flags_t;

typedef enum ref_param_t
{
    eParamIn,    // passed by value in whatever way is most efficient
    eParamOut,   // passed by reference
    eParamInOut, // passed by reference, but may be modified

    eParamCount
} ref_param_t;

typedef struct ref_ast_t
{
    ref_kind_t kind;
    const node_t *node;

    union {
        /* eAstInteger */
        mpz_t integer;

        /* eAstAttribString */
        struct
        {
            /* eAstIdent */
            const char *ident;

            /* eAttribCxxName, eAttribFormat */
            ref_attrib_tag_t attrib;
        };

        /* eAstBool */
        bool boolean;

        /* eAstString */
        text_t text;

        /* eAstProgram */
        struct
        {
            vector_t *mod;
            ref_ast_t *config[eRefConfigCount];
            vector_t *imports;
            vector_t *decls;
        };

        struct
        {
            union {
                /* eAstConfig */
                ref_config_tag_t cfg;

                /* eAstUnary */
                unary_t unary;
            };

            /* eAstAttribTypeId, eAstAttribAlignas */
            struct ref_ast_t *expr;
        };

        struct
        {
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
        struct
        {
            ref_ast_t *generic;
            vector_t *params;
        };

        /* eAstSpan, eAstVector, eAstArray */
        struct
        {
            struct ref_ast_t *ptr;

            /* only used in eAstArray for now */
            /* maybe in the future eAstSpan will also use this */
            struct ref_ast_t *size;
        };

        struct
        {
            /* eAstPrivacy, eDeclField, eDeclMethod */
            ref_privacy_t privacy;

            /* eAstClass, eAstStruct, eAstVariant */
            ref_flags_t flags;
            vector_t *attributes;
            vector_t *tparams;
            char *name;

            union {
                /* eAstClass, eAstStruct. eAstUnion, eAstUnionField */
                struct
                {
                    ref_ast_t *parent;
                    vector_t *methods;

                    /* eAstVariant */
                    struct
                    {
                        ref_ast_t *default_case;
                        vector_t *cases;
                        vector_t *fields;
                    };
                };

                /* eAstField, eAstConst, eAstParam */
                struct
                {
                    ref_param_t param;

                    /* eAstTypeAlias */
                    ref_ast_t *type;
                    ref_ast_t *initial;
                };

                /* eAstMethod */
                struct
                {
                    ref_ast_t *return_type;
                    vector_t *method_params;
                    ref_ast_t *body;
                };

                /* eAstCase */
                struct
                {
                    bool is_default;
                    ref_ast_t *value;
                };
            };
        };

        /* eAstAttribDeprecated */
        char *message;

        /* eAstAttribDocs */
        map_t *docs;

        /* eAstAttribAssert */
        struct
        {
            vector_t *before;
            vector_t *after;
            vector_t *always;
        };
    };
} ref_ast_t;

typedef struct ref_pair_t
{
    char *ident;
    char *body;
} ref_pair_t;

ref_pair_t ref_pair(char *ident, typevec_t *body);
char *ref_make_string(typevec_t *text);

ref_ast_t *ref_unary(scan_t *scan, where_t where, unary_t op, ref_ast_t *expr);
ref_ast_t *ref_binary(scan_t *scan, where_t where, binary_t op, ref_ast_t *lhs, ref_ast_t *rhs);
ref_ast_t *ref_compare(scan_t *scan, where_t where, compare_t op, ref_ast_t *lhs, ref_ast_t *rhs);

ref_ast_t *ref_program(scan_t *scan, where_t where, vector_t *mod, vector_t *leading,
                       vector_t *decls);

ref_ast_t *ref_import(scan_t *scan, where_t where, text_t text);

ref_ast_t *ref_class(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent,
                     vector_t *body);
ref_ast_t *ref_union(scan_t *scan, where_t where, char *name, ref_ast_t *key, vector_t *body);
ref_ast_t *ref_union_field(scan_t *scan, where_t where, vector_t *cases, vector_t *fields);

ref_ast_t *ref_struct(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent,
                      vector_t *body);

ref_ast_t *ref_privacy(scan_t *scan, where_t where, ref_privacy_t privacy);
ref_ast_t *ref_using(scan_t *scan, where_t where, char *name, ref_ast_t *type);

ref_ast_t *ref_field(scan_t *scan, where_t where, char *name, ref_ast_t *type, ref_ast_t *value);
ref_ast_t *ref_method(scan_t *scan, where_t where, ref_flags_t flags, char *name, vector_t *params,
                      ref_ast_t *type, ref_ast_t *body);
ref_ast_t *ref_param(scan_t *scan, where_t where, char *name, ref_param_t param, ref_ast_t *type);
ref_ast_t *ref_ctor(scan_t *scan, where_t where, vector_t *params, ref_ast_t *body);

ref_ast_t *ref_pointer(scan_t *scan, where_t where, ref_ast_t *type);
ref_ast_t *ref_reference(scan_t *scan, where_t where, ref_ast_t *type);

// non-owning view over a range of values
ref_ast_t *ref_span(scan_t *scan, where_t where, ref_ast_t *type);

// owning fixed size array of values
ref_ast_t *ref_array(scan_t *scan, where_t where, ref_ast_t *type, ref_ast_t *size);

// owning variable size array of values
ref_ast_t *ref_vector(scan_t *scan, where_t where, ref_ast_t *type);

ref_ast_t *ref_const(scan_t *scan, where_t where, ref_ast_t *type);

ref_ast_t *ref_variant(scan_t *scan, where_t where, char *name, ref_ast_t *underlying,
                       vector_t *cases);

ref_ast_t *ref_case(scan_t *scan, where_t where, char *name, ref_ast_t *value, bool is_default);

ref_ast_t *ref_opaque(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_opaque_text(scan_t *scan, where_t where, text_t text);

ref_ast_t *ref_ident(scan_t *scan, where_t where, char *ident);
ref_ast_t *ref_integer(scan_t *scan, where_t where, mpz_t integer);
ref_ast_t *ref_string(scan_t *scan, where_t where, typevec_t *text);
ref_ast_t *ref_bool(scan_t *scan, where_t where, bool value);

ref_ast_t *ref_attrib_transient(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_deprecated(scan_t *scan, where_t where, char *message);
ref_ast_t *ref_attrib_typeid(scan_t *scan, where_t where, ref_ast_t *expr);
ref_ast_t *ref_attrib_alignas(scan_t *scan, where_t where, ref_ast_t *expr);
ref_ast_t *ref_attrib_remote(scan_t *scan, where_t where);
ref_ast_t *ref_attrib_docs(scan_t *scan, where_t where, map_t *docs);

ref_ast_t *ref_attrib_tag(scan_t *scan, where_t where, ref_attrib_tag_t tag);
ref_ast_t *ref_attrib_string(scan_t *scan, where_t where, ref_attrib_tag_t tag, char *text);

ref_ast_t *ref_attrib_assert(scan_t *scan, where_t where, typevec_t *before, typevec_t *after,
                             typevec_t *always);

ref_ast_t *ref_config_tag(scan_t *scan, where_t where, ref_config_tag_t tag, ref_ast_t *expr);

void ref_set_attribs(ref_ast_t *ast, vector_t *attributes);
void ref_set_flags(ref_ast_t *ast, ref_flags_t flags);

CT_END_API
