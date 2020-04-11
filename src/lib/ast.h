#pragma once

#include "utils/stream.h"

#include <variant>
#include <vector>

namespace ctu::ast
{
    struct Node
    {

    };

    // [[packed]] type-decl
    // [[packed(num)]] type-decl
    struct Packed
    {
        uint32_t width;
    };

    // [[align(num)]] type-decl
    struct Align
    {
        uint32_t align;
    };

    // [[segment(name)]]
    // func-decl

    // [[segment(name)]]
    // var-decl

    // [[segment(name)]] {
    //     body-decls
    // }
    struct Segment
    {
        std::string name;
    };

    // [[origin(vaddr)]]
    // func-decl

    // [[origin(vaddr)]]
    // var-decl

    // [[origin(vaddr)]] {
    //     body-decls
    // }
    struct Origin
    {
        uint64_t origin;
    };

    // [[target(id)]]
    // func-decl

    // [[target(id)]] {
    //     body-decls
    // }
    enum class Target
    {
        I8086,
        X86,
        X64
    };

    // [[interrupt(target)]] 
    // func-decl
    struct Interrupt { Target target; };

    // [[syscall(target)]]
    // func-decl
    struct SysCall { Target target; };

    // [[noreturn]]
    // func-decl
    struct NoReturn { };

    // [[inline(never)]]
    // func-decl

    // [[inline(always)]]
    // func-decl

    // [[inline(default)]]
    // func-decl

    // [[inline]]
    // func-decl
    enum class Inline
    {
        // function is never inlined
        Never,
        // function is always inlined
        Always,
        // function is inlined when the compiler thinks it will help
        Needed,
    };

    using AttributeData = std::variant<
        Packed,
        Align,
        Segment,
        Origin,
        Target,
        Interrupt,
        SysCall,
        Inline,
        NoReturn
    >;

    struct Attribute : Node
    {
        AttributeData attrib;
    };

    
    struct Expr : Node
    {

    };


    struct Type : Node
    {
        std::vector<Attribute> attributes;
    };


    // @sizeof(type-decl)
    struct SizeOf
    {
        
    };

    // @asm { asm-decls }
    struct Asm
    {

    };

    // @cast<type-decl>(expr)
    struct Cast
    {

    };

    // @local var-decl
    struct Local
    {

    };



    struct Builtin : Expr
    {
        
    };
}