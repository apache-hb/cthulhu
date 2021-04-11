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
    bool def = true;

    virtual void visit(ast::RecordType* node) override {
        auto name = node->name + "_struct";

        if (def) {
            def = false;
            defs << "struct " << name << ";" << std::endl;

            types << "struct " << name << " {" << std::endl;
            
            for (auto [id, type] : node->fields) {
                type->visit(this);
                types << " " << id << ";" << std::endl;
            }

            types << "};" << std::endl; 
            def = true;
        } else {
            types << "struct " << name;
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

    virtual void visit(ast::SumType* node) override {
        if (def) {
            def = false;

            if (is_enum(node)) {
                defs << "enum " << node->name << "_enum" << ";" << std::endl;
                types << "typedef enum {" << std::endl;
                for (auto item : node->cases) {
                    types << node->name << item.name << "," << std::endl;
                }
                types << "} " << node->name << "_enum" << ";" << std::endl;
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
                            type->visit(this);
                            types << " " << name << ";" << std::endl;
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

            def = true;
        } else {
            if (is_enum(node)) {
                types << "enum " << node->name << "_enum";
            } else {
                types << "struct " << node->name << "_variant";
            }
        }
    }

    virtual void visit(ast::AliasType* node) override {
        if (def)
            return;
        node->type->visit(this);
    }

    virtual void visit(ast::SentinelType* node) override {
        if (def)
            return;
        ctx->get(node->name)->visit(this);
    }

    virtual void visit(ast::PointerType* node) override {
        node->type->visit(this);
        types << "*";
    }

    virtual void visit(ast::ClosureType*) override {

    }

    virtual void visit(ast::ArrayType*) override {

    }

    virtual void visit(ast::ScalarType* node) override {
        types << node->name;
    }

    virtual void visit(ast::BoolType*) override {
        types << "int";
    }

    virtual void visit(ast::VoidType*) override {
        types << "void";
    }

    virtual void visit(ast::IntLiteral*) override {

    }

    virtual void visit(ast::Binary*) override {

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
