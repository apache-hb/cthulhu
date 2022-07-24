#pragma once

#include "scan/node.h"

#include "digit.h"
#include "attribs.h"
#include "ops.h"

#include <gmp.h>

typedef struct Hlir Hlir;
typedef struct vector_t vector_t;

typedef enum HlirKind {
#define HLIR_KIND(ID, _) ID,
#include "hlir-def.inc"

    eHlirTotal
} HlirKind;

typedef struct Hlir {
    HlirKind kind; ///< enum tag
    node_t node; ///< where this was created

    Hlir *type; ///< the type of this node

    union {
        /* all expressions */

        /* eHlirDigitLiteral */
        struct HlirDigitLiteral {
            mpz_t value;
        } digitLiteral;

        /* eHlirBoolLiteral */
        struct HlirBoolLiteral {
            bool value;
        } boolLiteral;

        /* eHlirStringLiteral */ 
        struct HlirStringLiteral {
            char *value;
            size_t length;
        } stringLiteral;

        /* eHlirLoad */
        struct HlirName {
            Hlir *address;
        } loadExpr;

        /* eHlirUnary */
        struct HlirUnary {
            unary_t op;
            Hlir *operand;
        } unaryExpr;

        /* eHlirBinary */
        struct HlirBinary {
            binary_t op;
            Hlir *lhs;
            Hlir *rhs;
        } binaryExpr;

        /* eHlirCompare */
        struct HlirCompare {
            compare_t op;
            Hlir *lhs;
            Hlir *rhs;
        } compareExpr;

        /* eHlirCall */
        struct HlirCall {
            Hlir *function;
            vector_t *args;
        } callExpr;

        /* eHlirAccess */
        struct HlirAccess {
            Hlir *body;
            const char *field;
            bool indirect;
        } accessExpr;

        /* eHlirIndex */
        struct HlirIndex {
            Hlir *array;
            Hlir *offset;
        } indexExpr;

        /* eHlirBuiltin */
        struct HlirBuiltin {
            builtin_t op;
            Hlir *type;
        } builtinExpr;

        /* all statements */

        /* eHlirStmts */
        struct HlirStmts {
            vector_t *stmts;
        } stmts;

        /* eHlirBranch */
        struct HlirBranch {
            Hlir *condition;
            Hlir *then;
            Hlir *other;
        } branchStmt;

        /* eHlirLoop */
        struct HlirLoop {
            Hlir *condition;
            Hlir *then;
            Hlir *other;
        } loopStmt;

        /* eHlirBreak */
        struct HlirBreak {
            Hlir *loop;
        } breakStmt;

        /* eHlirContinue */
        struct HlirContinue {
            Hlir *loop;
        } continueStmt;

        /* eHlirAssign */
        struct HlirAssign {
            Hlir *dst;
            Hlir *src;
        } assignStmt;

        /* eHlirReturn */
        struct HlirReturn {
            Hlir *result;
        } returnStmt;

        /* all nodes that can be named */
        struct {
            const char *name;
            HlirAttribs attribs;

            union {
                /* all declarations */

                /* eHlirGlobal */
                struct HlirGlobal {
                    Hlir *value;
                } globalDecl;

                /* eHlirFunction */
                struct HlirFunction {
                    vector_t *locals;
                    Hlir *body;
                } functionDecl;

                /* eHlirForward */
                struct HlirForward {
                    HlirKind expectedKind;
                } forwardDecl;

                /* eHlirParam */
                
                /* eHlirLocal */
                
                /* eHlirError */
                struct HlirError {
                    const char *message;
                } error;

                /* all types */

                /* eHlirDigit */
                struct HlirDigit {
                    digit_t digit;
                    sign_t sign;
                } digitType;

                /* eHlirString */

                /* eHlirBool */

                /* eHlirVoid */

                /* eHlirArray */
                struct HlirArray {
                    Hlir *element;
                    Hlir *length;
                } arrayType;

                /* eHlirPointer */
                struct HlirPointer {
                    Hlir *pointer;
                    bool indexable;
                } pointerType;

                /* eHlirClosure */
                struct HlirClosure {
                    Hlir *returnType;
                    vector_t *parameters;
                    bool variadic;
                } closureType;

                /* eHlirStruct */
                struct HlirStruct {
                    vector_t *fields;
                } structType;

                /* eHlirUnion */
                struct HlirUnion {
                    vector_t *fields;
                } unionType;

                /* eHlirEnum */ 
                struct HlirEnum {
                    Hlir *underlyingType;
                    vector_t *cases;
                } enumType;

                /* eHlirAlias */
                struct HlirAlias {
                    Hlir *underlying;
                    bool newType;
                } aliasType;

                /* eHlirRecordField */
                
                struct HlirEnumField {
                    Hlir *init;
                } enumField;
            };
        };
    };
} Hlir;
