#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <algorithm>

namespace AST
{
    using namespace std;

    struct Type {
        virtual ~Type() {}
        virtual size_t size() const = 0;

        virtual string str() const = 0;
    };

    struct MutableType : Type {
        MutableType(Type* o)
            : of(o)
        { }

        virtual ~MutableType() override { }

        Type* of;

        virtual size_t size() const override { return of->size(); }

        virtual string str() const override { return "mut(" + of->str() + ")"; }
    };

    struct PointerType : Type {
        PointerType(Type* t)
            : to(t)
        { }

        virtual ~PointerType() override { }

        Type* to;

        // TODO: support pointers and segmentation
        virtual size_t size() const override { return 8; }

        virtual string str() const override { return to->str() + "*"; }
    };

    enum class Builtin {
        U8,
        U16,
        U32,
        U64,
        I8,
        I16,
        I32,
        I64,
        F32,
        F64,
        VOID
    };

    struct BuiltinType : Type {
        BuiltinType(Builtin b)
            : type(b)
        { }

        virtual ~BuiltinType() override { }

        Builtin type;

        virtual size_t size() const override {
            switch(type) {
            case Builtin::U8: return 1;
            case Builtin::U16: return 2;
            case Builtin::U32: return 4;
            case Builtin::U64: return 8;
            case Builtin::I8: return 1;
            case Builtin::I16: return 2;
            case Builtin::I32: return 4;
            case Builtin::I64: return 8;
            case Builtin::F32: return 4;
            case Builtin::F64: return 8;
            case Builtin::VOID: return 0;
            default: return -1;
            }
        }

        virtual string str() const override {
            switch(type) {
            case Builtin::U8: return "u8";
            case Builtin::U16: return "u16";
            case Builtin::U32: return "u32";
            case Builtin::U64: return "u64";
            case Builtin::I8: return "i8";
            case Builtin::I16: return "i16";
            case Builtin::I32: return "i32";
            case Builtin::I64: return "i64";
            case Builtin::F32: return "f32";
            case Builtin::F64: return "f64";
            case Builtin::VOID: return "void";
            default: return "err";
            }
        }
    };

    struct FunctionType : Type {
        FunctionType(vector<Type*> a, Type* r)
            : args(a)
            , ret(r)
        { }

        virtual ~FunctionType() override { }

        vector<Type*> args;
        Type* ret;

        // TODO: captures will change the size of the object
        virtual size_t size() const override { return 8; }

        virtual string str() const override {
            string out = ret->str() + "(";

            for(size_t i = 0; i < args.size(); i++) {
                if(i != 0)
                    out += ", ";
                out += args[i]->str();
            }
            out += ")";

            return out;
        }
    };

    struct UnionType : Type {
        UnionType(map<string, Type*> f)
            : fields(f)
        { }

        virtual ~UnionType() override { }

        map<string, Type*> fields;

        virtual size_t size() const override {
            size_t out = 0;
            for(const auto& pair : fields) {
                auto type = pair.second;
                if(type->size() > out)
                    out = type->size();
            }
            return out;
        }

        virtual string str() const override {
            string out = "union {\n";

            for(auto& [name, type] : fields) {
                // TODO: proper indents
                out += "    " + type->str() + " " + name + ";\n";
            }

            out += "}";

            return out;
        }
    };

    struct StructType : Type {
        StructType(vector<pair<string, Type*>> f)
            : fields(f)
        { }

        virtual ~StructType() override { }

        vector<pair<string, Type*>> fields;

        virtual size_t size() const override {
            size_t out = 0;
            for(auto& each : fields) {
                out += each.second->size();
            }
            return out;
        }

        virtual string str() const override {
            string out = "struct {\n";
            for(auto& [name, type] : fields) {
                out += "    " + type->str() + " " + name + ";\n";
            }
            out += "}\n";

            return out;
        }
    };

    struct VariantType : Type {
        VariantType(map<string, Type*> f)
            : fields(f)
        { }

        virtual ~VariantType() override { }

        map<string, Type*> fields;

        virtual size_t size() const override {
            size_t out = 0;
            for(auto& pair : fields) {
                if(pair.second->size() > out)
                    out = pair.second->size();
            }
            // TODO: configurable backing
            return out + 4;
        }

        virtual string str() const override {
            string out = "variant {\n";
            for(auto& [name, type] : fields) {
                out += "    " + type->str() + " " + name + ";\n";
            }
            out += "}\n";
            return out;
        }
    };

    struct TemplateType : Type {
        vector<pair<string, Type*>> params;
        Type* of;
    };
}