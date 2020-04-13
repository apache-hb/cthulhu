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
    NodeTypeArray,
    NodeTypeVariant,
    NodeTypeName,
    NodeTypeAttribute,
    NodeTypeBuiltinType,
    NodeTypeBuiltinFunction,
    NodeTypeFunction,
    NodeTypeFunctionArgs
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
            char* name;
        } Name;

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

void Expect(Parser* parser, Keyword key)
{
    Token tok = LexerNext(parser->lex);
    if(tok.type == TokenTypeKeyword && tok.data.keyword == key)
    {
        TokenFree(tok);
    }
    else
    {
        printf("invalid keyword\n");
        exit(500);
    }
}

Node* TryParseAttribs(Parser* parser, Token* tok)
{
    // TODO: implement
    if(tok->type == TokenTypeKeyword && tok->data.keyword == KeywordBuiltin)
    {
        return NULL;
    }
    else
    {
        return NULL;
    }
}

Node* ParseType(Parser* parser)
{
    Token tok = LexerNext(parser->lex);

    if(tok.type == TokenTypeIdent)
    {
        Node* type = NewNode(NodeTypeName);
        type->Name.name = tok.data.ident;
        return type;
    }
    else if(tok.type == TokenTypeKeyword)
    {
        Node* attribs = TryParseAttribs(parser, &tok);
        if(tok.data.keyword == KeywordLSquare)
        {
            return ParseArray(parser);
        }
        else if(tok.data.keyword == KeywordLBrace)
        {
            return ParseStruct(parser);
        }
        else if(tok.data.keyword == KeywordEnum)
        {
            return ParseEnum(parser);
        }
        else if(tok.data.keyword == KeywordUnion)
        {
            return ParseEnum(parser);
        }
        else if(tok.data.keyword == KeywordVariant)
        {
            return ParseVariant(parser);
        }
        else
        {
            // oh no
        }
    }
    else
    {
        // error
    }
}

Node* ParseFuncArgs(Parser* parser, Token* tok)
{
    *tok = LexerNext(parser->lex);

    if(tok->type == TokenTypeKeyword && tok->data.keyword == KeywordRParen)
    {
        // empty function args
        return NULL;
    }

    Node* args = NewNode(NodeTypeFunctionArgs);

    char* names[64];
    Node* types[64];
    int i = 0;

    while(tok->type == TokenTypeIdent)
    {
        names[i] = tok->data.ident;
        Expect(parser, KeywordColon);
        types[i] = ParseType(parser);
    }
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

    if(tok.data.keyword == KeywordLParen)
    {
        func->Function.args = ParseFuncArgs(parser, &tok);
    }
    else
    {
        func->Function.args = NULL;
    }

    if(tok.data.keyword == KeywordArrow)
    {

    }
    else
    {
        func->Function.ret = NULL;
    }

    if(tok.data.keyword == KeywordLBrace)
    {

    }

    if(tok.data.keyword == KeywordAssign)
    {

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