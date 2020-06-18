#pragma once

#include <vector>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include <memory>

using str = std::string;

template<typename T>
using unique = std::unique_ptr<T>;

template<typename T>
using shared = std::shared_ptr<T>;

template<typename T>
using vec = std::vector<T>;

namespace ct::ast {
    using namespace std;

    struct Node {

    };

    enum class CallConv {
        // windows x64
        win64,

        // system v amd64
        sysv64,

        // cdecl
        externc,

        // let the codegen do whatever it thinks its fastest
        internal
    };

    struct Function : Node {
        CallConv conv;
    };

    struct Type : Node {

    };

    struct Stmt : Node {

    };

    struct StmtList : Stmt {
        vec<Stmt*> stmts;
    };

    struct Expr : Stmt {

    };
}