#include "cthulhu.h"

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

void TypeFields::add(const Field& field) {
    for (auto& elem : *this) {
        if (elem.name == field.name) {
            panic("field `{}` already defined", field.name);
        }
    }

    push_back(field);
}

void VariantCases::add(const VariantCase& field) {
    for (auto& elem : *this) {
        if (elem.name == field.name) {
            panic("variant case `{}` already defined", field.name);
        }
    }

    push_back(field);
}

void Context::parse() {
    for (auto node : tree->nodes) {
        switch (node->tag) {
        case "struct"_:
            add(record(node));
            break;
        case "alias"_:
            add(alias(node));
            break;
        case "union"_:
            add(union_(node));
            break;
        case "variant"_:
            add(variant(node));
            break;
        case "function"_:
            add(function(node));
            break;
        default:
            panic("unrecognized node `{}`", node->name);
        }
    }
}

FunctionType* Context::function(std::shared_ptr<peg::Ast> ast) {
    std::cout << ast_to_s(ast) << std::endl;

    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    Type* result = target::VOID;
    
    auto res = ast->nodes[ast->nodes.size() - 2];
    if (res->tag != "params"_ && ast->nodes.size() > 2) {
        result = type(res);
    }

    for (auto node : ast->nodes) {
        if (node->tag == "params"_) {
            for (auto each : ast->nodes[1]->nodes) {
                auto arg = each->nodes[0]->token_to_string();
                auto kind = type(each->nodes[1]);

                fields.add({ arg, kind });
            }
            break;
        }
    }

    auto stmt = body(ast->nodes.back());

    return new FunctionType(name, fields, result, stmt);
}

Stmt* Context::body(std::shared_ptr<peg::Ast> ast) {
    if (ast->tag == "compound"_) {
        return nullptr;
    }
    return nullptr;
}

RecordType* Context::record(std::shared_ptr<Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return new RecordType(name, fields);
}

UnionType* Context::union_(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return new UnionType(name, fields);
}

AliasType* Context::alias(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();
    auto other = type(ast->nodes[1]);

    return new AliasType(name, other);
}

VariantType* Context::variant(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    auto parent = ast->nodes.size() > 2
        ? type(ast->nodes[1])
        : target::VARIANT;
    VariantCases cases;

    auto nodes = ast->nodes.back()->nodes;
    for (auto node : nodes) {
        cases.add(vcase(node));
    }

    return new VariantType(name, parent, cases);
}

VariantCase Context::vcase(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();
    TypeFields fields;

    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return { name, fields };
}

Field Context::field(std::shared_ptr<peg::Ast> ast) {
    return { ast->nodes[0]->token_to_string(), type(ast->nodes[1]) };
}

Type* Context::type(std::shared_ptr<peg::Ast> ast) {
    auto collect = [&](std::shared_ptr<peg::Ast> node) {
        Types out;
        for (auto it : node->nodes) {
            out.push_back(type(it));
        }
        return out;
    };

    switch (ast->tag) {
    case "pointer"_:
        return new PointerType(type(ast->nodes[0]));
    case "closure"_:
        return new ClosureType(collect(ast->nodes[0]), type(ast->nodes[1]));
    case "ident"_:
        return get(ast->token_to_string());
    default:
        panic("unknown type `{}`", ast->name);
    }
}

}