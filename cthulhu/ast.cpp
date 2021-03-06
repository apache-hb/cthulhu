#include "ast.hpp"

#include <type_traits>

#define SELF std::dynamic_pointer_cast

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

    ast::UnaryExpr::UnaryOp unop(Token) {
        return {};
    }

    ast::BinaryExpr::BinaryOp binop(Token) {
        return {};
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
            if (!size) {
                if (o->size) {
                    return false;
                }
            }

            return type->equals(o->type) && size->equals(o->size);
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

    UnaryExpr::UnaryExpr(Token op, ptr<Expr> expr) 
        : op(unop(op))
        , expr(expr)
    { }

    bool UnaryExpr::equals(const ptr<Node> other) const {
        (void)other;
        return false;
    }

    BinaryExpr::BinaryExpr(Token op, ptr<Expr> lhs, ptr<Expr> rhs) 
        : op(binop(op))
        , lhs(lhs)
        , rhs(rhs)
    { }

    bool BinaryExpr::equals(const ptr<Node> other) const {
        (void)other;
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
}
