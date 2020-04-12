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
    NodeTypeBuiltinFunction,
    NodeTypeFunction
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

        struct {
            int count;
            char* names;

            // type of argument
            struct Node* types;

            // default values
            struct Node* values;
        } FunctionArguments;

        struct {
            // name of function
            char* name;

            // function arguments
            struct Node* args;

            // function return type, NULL if deduced
            struct Node* ret;
        } Function;
    };
} Node;

Node* NewNode(NodeType type)
{
    Node* node = malloc(sizeof(Node));
    node->type = type;
    return node;
}

Node* ParseDef(Parser* parser)
{
    Token tok = LexerNext(parser->lex);
    if(tok.type != TokenTypeIdent)
    {
        // error
        return NULL;
    }

    Node* func = NewNode(NodeTypeFunction);
    func->Function.name = tok.data.ident;

    tok = LexerNext(parser->lex);
    if(tok.type != TokenTypeKeyword)
    {
        // error
        return NULL;
    }

    switch(tok.data.keyword)
    {
    case KeywordLParen:
    case KeywordAssign:
    case KeywordArrow:
    case KeywordLBrace:
    default:
        // error
        return NULL;
    }
}

Node* ParserNext(Parser* parser)
{
    Token tok = LexerNext(parser->lex);
    if(tok.type == TokenTypeKeyword)
    {
        switch(tok.data.keyword)
        {
        case KeywordLSquare2:
        case KeywordBuiltin:
        case KeywordImport:
        case KeywordDef:
            return ParseDef(parser);
        case KeywordType:
        default:
            return NULL;
            break;
        }
    }
    else
    {
        // error
        return NULL;
    }
}