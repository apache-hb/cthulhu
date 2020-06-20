#include <memory>
#include <vector>
#include <map>
#include <string>

#include "ast/ast.h"

namespace ct {
    struct Gen {
        virtual ~Gen() {}
        void emit(const std::string& str) {
            printf("%s\n", str.c_str());
        }

        virtual void gen(std::shared_ptr<ast::Unit> unit) = 0;
    };

    struct X86 : Gen {
        virtual ~X86() override {}

        void ret() {
            
        }

        void gen_stmt(std::unique_ptr<ast::Stmt> stmt) {
            if (auto* ret = std::dynamic_pointer_cast<ast::Return>(stmt); ret) {
                
            }
        }

        void gen_func(std::shared_ptr<ast::Function> func) {
            emit("push ebp");

            

            emit("pop ebp");
            emit("ret");
        }

        virtual void gen(std::shared_ptr<ast::Unit> unit) override {
            for (auto& [name, func] : unit->funcs) {
                emit(name);
                gen_func(func);
            }
        }
    };
}