#include "ast.hpp"

#include <type_traits>

namespace {
    using namespace cthulhu;

    template<typename T>
    bool vequals(const vec<ptr<T>>& lhs, const vec<ptr<T>>& rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (size_t i = 0; i < lhs.size(); i++) {
            if (!lhs[i]->equals(rhs[i])) {
                return false;
            }
        }

        return true;
    }

    template<typename T>
    bool pequals(ptr<T> lhs, ptr<T> rhs) {
        if (lhs != nullptr && rhs != nullptr) {
            return lhs->equals(rhs);
        } else {
            return !!lhs == !!rhs;
        }
    }
}

namespace cthulhu::ast {
    Ident::Ident(Token ident)
        : token(ident)
    { }

    bool Ident::equals(const ptr<Node> other) const {
        if (auto o = SELF<Ident>(other); o) {
            return token.equals(o->token);
        }

        return false;
    }

    PointerType::PointerType(ptr<Type> type) 
        : type(type)
    { }

    bool PointerType::equals(const ptr<Node> other) const {
        if (auto o = SELF<PointerType>(other); o) {
            return type->equals(o->type);
        }

        return false;
    }

    MutableType::MutableType(ptr<Type> type) 
        : type(type)
    { }

    bool MutableType::equals(const ptr<Node> other) const {
        if (auto o = SELF<MutableType>(other); o) {
            return type->equals(o->type);
        }

        return false;
    }

    ArrayType::ArrayType(ptr<Type> type, ptr<Expr> size) 
        : type(type)
        , size(size)
    { }

    bool ArrayType::equals(const ptr<Node> other) const {
        if (auto o = SELF<ArrayType>(other); o) {
            return type->equals(o->type) && pequals(size, o->size);
        }

        return false;
    }

    NameType::NameType(ptr<Ident> name) 
        : name(name)
    { }

    bool NameType::equals(const ptr<Node> other) const {
        if (auto o = SELF<NameType>(other); o) {
            return name->equals(o->name);
        }

        return false;
    }

    ClosureType::ClosureType(vec<ptr<Type>> args, ptr<Type> result)
        : args(args)
        , result(result)
    { } 

    bool ClosureType::equals(const ptr<Node> other) const {
        if (auto o = SELF<ClosureType>(other); o) {
            return result->equals(o->result) && vequals(args, o->args);
        }

        return false;
    }

    QualifiedType::QualifiedType(vec<ptr<NameType>> names)
        : names(names)
    { }

    bool QualifiedType::equals(const ptr<Node> other) const {
        if (auto o = SELF<QualifiedType>(other); o) {
            return vequals(names, o->names);
        }

        return false;
    }

    UnaryExpr::UnaryExpr(UnaryOp op, ptr<Expr> expr) 
        : op(op)
        , expr(expr)
    { }

    bool UnaryExpr::equals(const ptr<Node> other) const {
        if (auto o = SELF<UnaryExpr>(other); o) {
            return op == o->op && expr->equals(o->expr);
        }

        return false;
    }

    BinaryExpr::BinaryExpr(BinaryOp op, ptr<Expr> lhs, ptr<Expr> rhs) 
        : op(op)
        , lhs(lhs)
        , rhs(rhs)
    { }

    bool BinaryExpr::equals(const ptr<Node> other) const {
        if (auto o = SELF<BinaryExpr>(other); o) {
            return op == o->op && lhs->equals(o->lhs) && rhs->equals(o->rhs);
        }

        return false;
    }

    TernaryExpr::TernaryExpr(ptr<Expr> cond, ptr<Expr> yes, ptr<Expr> no) 
        : cond(cond)
        , yes(yes)
        , no(no)
    { }

    bool TernaryExpr::equals(const ptr<Node> other) const {
        (void)other;
        return false;
    }

    StringExpr::StringExpr(const utf8::string* string) 
        : string(string)
    { }
    
    bool StringExpr::equals(const ptr<Node> other) const {
        if (auto o = SELF<StringExpr>(other); o) {
            return string == o->string;
        }
        return false;
    }

    IntExpr::IntExpr(const Number& number) 
        : number(number)
    { }
    
    bool IntExpr::equals(const ptr<Node> other) const {
        if (auto o = SELF<IntExpr>(other); o) {
            return number.number == o->number.number && number.suffix == o->number.suffix;
        }

        return false;
    }

    BoolExpr::BoolExpr(bool val) 
        : val(val)
    { }
    
    bool BoolExpr::equals(const ptr<Node> other) const {
        if (auto o = SELF<BoolExpr>(other); o) {
            return val == o->val;
        }
        
        return false;
    }

    CharExpr::CharExpr(c32 letter) 
        : letter(letter)
    { }
    
    bool CharExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    NameExpr::NameExpr(ptr<QualifiedType> name) 
        : name(name)
    { }
    
    bool NameExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    CoerceExpr::CoerceExpr(ptr<Type> type, ptr<Expr> expr) 
        : type(type)
        , expr(expr)
    { }
    
    bool CoerceExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    SubscriptExpr::SubscriptExpr(ptr<Expr> expr, ptr<Expr> index) 
        : expr(expr)
        , index(index)
    { }
    
    bool SubscriptExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    AccessExpr::AccessExpr(ptr<Expr> body, ptr<Ident> field, bool indirect) 
        : body(body)
        , field(field)
        , indirect(indirect)
    { }
    
    bool AccessExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    CallArg::CallArg(ptr<Ident> name, ptr<Expr> expr) 
        : name(name)
        , expr(expr)
    { }
    
    bool CallArg::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    CallExpr::CallExpr(ptr<Expr> func, vec<ptr<CallArg>> args) 
        : func(func)
        , args(args)
    { }

    bool CallExpr::equals(const ptr<Node> other) const {
        (void)other;
        throw std::runtime_error("unimplemented");
    }

    Attribute::Attribute(vec<ptr<Ident>> path, vec<ptr<CallArg>> args)
        : path(path)
        , args(args)
    { }

    Attributes::Attributes(vec<ptr<Attribute>> attributes, ptr<Decl> decl)
        : attributes(attributes)
        , decl(decl)
    { }

    Alias::Alias(ptr<Ident> name, ptr<Type> type)
        : name(name)
        , type(type)
    { }

    Import::Import(vec<ptr<Ident>> path, bool wildcard, vec<ptr<Ident>> items)
        : path(path)
        , items(items)
        , wildcard(wildcard)
    { }

    Unit::Unit(vec<ptr<Import>> imports, vec<ptr<Decl>> decls)
        : imports(imports)
        , decls(decls)
    { }
}
