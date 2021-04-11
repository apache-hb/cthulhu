#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fmt/core.h>

using namespace cthulhu;

struct X86: Context {
    X86() : Context() {

        // these are all the builtin types
        builtins = {
            new ast::VoidType(),
            new ast::BoolType(),

            new ast::ScalarType("char", 1, true),
            new ast::ScalarType("short", 2, true),
            new ast::ScalarType("int", 4, true),
            new ast::ScalarType("long", 8, true),

            new ast::ScalarType("uchar", 1, false),
            new ast::ScalarType("ushort", 2, false),
            new ast::ScalarType("uint", 4, false),
            new ast::ScalarType("ulong", 8, false),
        };
    }
};

struct C: Visitor {
    void build(Context* it) {
        ctx = it;
        for (auto type : ctx->types) {
            type->visit(this);
        }
    }

    void print(std::string name) {
        auto guard = name + "_H";
        std::stringstream out;
        out << "#ifndef " << guard << std::endl;
        out << "#define " << guard << std::endl;
        out << defs.str() << std::endl;
        out << types.str() << std::endl;
        out << funcs.str() << std::endl;
        out << "#endif /* " << guard << " */" << std::endl;

        std::cout << out.str() << std::endl;
    }

    Context* ctx;
    std::stringstream defs;
    std::stringstream types;
    std::stringstream funcs;
    
    std::stringstream* current;

    // true when emiting a defition, false when emmiting a field
    bool emit_def = true;

    // 0 when emitting a toplevel field, >0 during nesting
    int depth = 0;

    std::string fname;

    virtual void visit(ast::RecordType* node) override {
        auto name = node->name + "_struct";

        if (emit_def) {
            emit_def = false;
            defs << "struct " << name << ";" << std::endl;

            types << "struct " << name << " {" << std::endl;
            
            for (auto [id, type] : node->fields) {
                fname = id;
                type->visit(this);
            }

            types << "};" << std::endl; 
            emit_def = true;
        } else {
            emit_field(types, "struct " + name);
        }
    }

    bool is_enum(ast::SumType* node) {
        for (auto item : node->cases) {
            if (item.fields.size() > 0) {
                return false;
            }
        }

        return true;
    }

    void emit_field(std::stringstream& ss, std::string name) {
        if (depth == 0) {
            ss << name << " " << fname << ";" << std::endl;
        } else {
            ss << name;
        }
    }

    virtual void visit(ast::SumType* node) override {
        if (emit_def) {
            emit_def = false;

            if (is_enum(node)) {
                auto name = node->name + "_enum";
                defs << "enum " << name << ";" << std::endl;
                types << "typedef enum {" << std::endl;
                for (auto item : node->cases) {
                    types << name << item.name << "," << std::endl;
                }
                types << "} " << name << ";" << std::endl;
            } else {
                auto tag = node->name + "_tag";
                auto data = node->name + "_data";
                // emit predefs
                defs << "enum " << tag << ";" << std::endl;
                defs << "union " << data << ";" << std::endl;
                defs << "struct " << node->name << "_variant" << ";" << std::endl;

                // emit tag
                types << "typedef enum {" << std::endl;

                for (auto item : node->cases) {
                    types << node->name << "_" << item.name << "_tag" << "," << std::endl;
                }

                types << "}" << tag << ";" << std::endl;

                // emit cases

                for (auto [id, fields] : node->cases) {
                    if (fields.size() > 0) {
                        defs << "struct " << node->name << "_" << id << ";" << std::endl;

                        types << "struct " << node->name << "_" << id << "{" << std::endl;

                        for (auto [name, type] : fields) {
                            fname = name;
                            type->visit(this);
                        }

                        types << "};" << std::endl;
                    }
                }

                // emit case container

                types << "typedef union {" << std::endl;

                for (auto [id, fields] : node->cases) {
                    if (fields.size() > 0) {
                        types << node->name << "_" << id << " " << id << ";" << std::endl;
                    }
                }

                types << "} " << data << ";" << std::endl;

                // emit tagged union

                types << "typedef struct {" << std::endl;

                types << tag << " tag;" << std::endl;
                types << data << " data;" << std::endl;

                types << "} " << node->name << ";" << std::endl;
            }

            emit_def = true;
        } else {
            if (is_enum(node)) {
                emit_field(types, "enum " + node->name + "_enum");
            } else {
                emit_field(types, "struct " + node->name + "_variant");
            }
        }
    }

    virtual void visit(ast::AliasType* node) override {
        if (emit_def)
            return;
        node->type->visit(this);
    }

    virtual void visit(ast::SentinelType* node) override {
        if (emit_def)
            return;
        ctx->get(node->name)->visit(this);
    }

    virtual void visit(ast::PointerType* node) override {
        depth++;
        node->type->visit(this);
        depth--;
        emit_field(types, "*");
    }

    virtual void visit(ast::ClosureType* node) override {
        depth++;
        node->result->visit(this);
        depth--;
        if (depth == 0) {
            types << "(*" << fname << ")";
        } else {
            types << "(*)";
        }

        types << "(";

        for (size_t i = 0; i < node->args.size(); i++) {
            if (i)
                types << ", ";
            depth++;
            node->args[i]->visit(this);
            depth--;
        }

        types << ")";

        if (depth == 0) {
            types << ";" << std::endl;
        }
    }

    virtual void visit(ast::ArrayType* node) override {
        depth++;
        node->type->visit(this);
        depth--;
        if (node->size) {
            types << "[";
            depth++;
            node->size->visit(this);
            depth--;
            types << "]";
        } else {
            types << "[]";
        }
    }

    virtual void visit(ast::ScalarType* node) override {
        emit_field(types, node->name);
    }

    virtual void visit(ast::BoolType*) override {
        emit_field(types, "int");
    }

    virtual void visit(ast::VoidType*) override {
        emit_field(types, "void");
    }

    virtual void visit(ast::IntLiteral* node) override {
        types << node->value;
    }

    virtual void visit(ast::Binary* node) override {
        node->lhs->visit(this);
        switch (node->op) {
        case ast::BinaryOp::ADD: types << "+"; break;
        case ast::BinaryOp::SUB: types << "-"; break;
        case ast::BinaryOp::DIV: types << "/"; break;
        case ast::BinaryOp::MOD: types << "%"; break;
        case ast::BinaryOp::MUL: types << "*"; break;
        case ast::BinaryOp::AND: types << "&&"; break;
        case ast::BinaryOp::OR: types << "||"; break;
        case ast::BinaryOp::XOR: types << "^"; break;
        case ast::BinaryOp::BITAND: types << "&"; break;
        case ast::BinaryOp::BITOR: types << "|"; break;
        case ast::BinaryOp::SHL: types << "<<"; break;
        case ast::BinaryOp::SHR: types << ">>"; break;
        case ast::BinaryOp::GT: types << ">"; break;
        case ast::BinaryOp::GTE: types << ">="; break;
        case ast::BinaryOp::LT: types << "<"; break;
        case ast::BinaryOp::LTE: types << "<="; break;
        default: panic("invalid binary op");
        }
        node->rhs->visit(this);
    }
};

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (std::ifstream in(path); !in.fail()) {
        auto text = std::string(std::istreambuf_iterator<char>{in}, {});
        
        try {
            init();

            X86 ctx;

            auto source = Builder(text);

            source.build(&ctx);

            C c;

            c.build(&ctx);

            std::replace(path.begin(), path.end(), '/', '_');
            std::replace(path.begin(), path.end(), '\\', '_');
            std::replace(path.begin(), path.end(), '.', '_');
            std::replace(path.begin(), path.end(), ':', '_');
            c.print(path);
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "failed to open: " << path << std::endl;
        return 1;
    }
}
