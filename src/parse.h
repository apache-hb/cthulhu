typedef struct {
    Lexer* lex;
} Parser;

Parser NewParser(Lexer* lex)
{
    Parser out;
    out.lex = lex;
    return out;
}


typedef enum {
    NodeTypeStruct,
    NodeTypeUnion,
    NodeTypeEnum,
    NodeTypeAttribute,
    NodeTypeBuiltinType,
    NodeTypeBuiltinFunction
} NodeType;

typedef enum {
    BuiltinTypeU8,
    BuiltinTypeU16,
    BuiltinTypeU32,
    BuiltinTypeU64,
    BuiltinTypeI8,
    BuiltinTypeI16,
    BuiltinTypeI32,
    BuiltinTypeI64,
    BuiltinTypeStr,
    BuiltinTypeBool,
    BuiltinTypeC8
} BuiltinType;

typedef enum {
    BuiltinFunctionSizeof,
    BuiltinFunctionCast,
    BuiltinFunctionLocal,
    BuiltinFunctionAsm
} BuiltinFunction;

typedef enum {
    AttributeTypePacked,
    AttributeTypeAlign,
    AttributeTypeOrigin,
    AttributeTypeNoReturn,
    AttributeTypeTarget,
    AttributeTypeDeprecated,
    AttributeTypeInline
} AttributeType;

typedef enum {
    I8086,
    X86,
    X64
} TargetPlatform;

typedef enum {
    InlineAlways,
    InlineNever,
    InlineAuto
} InlineBehaviour;

typedef struct Node {
    NodeType type;

    union {
        struct {
            int fields;
            char* names;
            struct Node* types;
        } Struct;

        struct {
            int fields;
            char* names;
            struct Node* types;
        } Union;

        struct {
            int fields;
            struct Node* backing;
            char* names;
            struct Node* values;
        } Enum;

        struct {
            BuiltinType type;
            union {
                InlineBehaviour behaviour;
                TargetPlatform target;

                // deprecated message
                char* message;

                // align to 
                int alignment;

                // pack to
                int width;

                // origin
                int vaddr;
            };
            
            int decls;
            struct Node* body;
        } BuiltinT;

        struct {
            BuiltinFunction func;
        } BuiltinF;

        struct {
            AttributeType type;
        } Attribute;
    };
} Node;