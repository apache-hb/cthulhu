#pragma once

#include <string>
#include <variant>
#include <memory>
#include <map>
#include <vector>
#include <istream>

namespace ctu
{
    enum class token_type
    {
        string,
        ident,
        integer,
        number,
        keyword,
        eof,
    };

    enum class keyword
    {
        // syntax

        kopenarr, // [
        kclosearr, // ]

        kopenarr2, // [[
        kclosearr2, // ]]

        kopenarg, // (
        kclosearg, // )

        kopenscope, // {
        kclosescope, // }

        karrow, // ->

        kassign, // :=

        kcolon, // :
        kcolon2, // ::

        kcomma, // ,
        kdot, // .

        kquestion, // ?

        // operators

        knot, // !

        kbitnot, // ~

        kbitor, // |
        kbitoreq, // |=

        kbitand, // &
        kbitandeq, // &=

        kbitxor, // ^
        kbitxoreq, // ^=

        kshl, // <<
        kshleq, // <<=

        kshr, // >>
        kshreq, // >>=

        keq, // ==
        kneq, // !=

        kgt, // >
        kgte, // >=

        klt, // <
        klte,  // <=

        kmul, // *
        kmuleq, // *=

        kdiv, // /
        kdiveq, // /=

        kmod, // %
        kmodeq, // %=

        kadd, // +
        kaddeq, // +=

        ksub, // -
        ksubeq, // -=

        kat, // @

        // type keywords

        kusing, // using
        kstruct, // struct
        kvariant, // variant
        kenum, // enum
        kunion, // union

        // structure keywords

        kimport, // import
        kmodule, // module

        // control keywords
        kif, // if
        kelse, // else
        kfor, // for
        kwhile, // while
        kswitch, // switch
        kcase, // case

        kcontinue, // continue
        kbreak, // break
        kreturn, // return

        kin, // in
        kas, // as
        kis, // is

        // decl keywords
        klet, // let
        kvar, // var
        kdef, // def

        // value keywords

        knull, // null
        ktrue, // true
        kfalse, // false

        // reserved keywords

        ksome, // some
        knone, // none
        kwhere, // where
        kwhen, // when
        kinline, // inline
        knew, // new
        kdelete, // delete
        kcast, // cast
        ktemplate, // template
        ktype, // type
        kself, // self
        kthis, // this
        ksuper, // super
        kclass, // class
        kconst, // const
        kfriend, // friend
        kexport, // export
        kextern, // extern
        kexplicit, // explicit
        katomic, // atomic
        ksynchronized, // synchronized
        ksync, // sync
        kmutable, // mutable
        koperator, // operator
        kinit, // init
        kdeinit, // deinit
        knamespace, // namespace
        kscope, // scope
        kconcept, // concept
        krequires, // requires
        kstatic, // static
        knot, // not
        kfinal, // final

        kasync, // async
        kawait, // await
        kyield, // yield

        kstub, // not a keyword
    };
    
    struct token
    {
        token_type type;
        std::variant<
            std::string,
            int_fast64_t,
            double,
            keyword
        > data;

        uint_fast64_t line;
        uint_fast64_t col;
        std::shared_ptr<std::string> file;
    };

    namespace ast
    {
        struct expr
        {

        };

        using dotted_name_decl = std::vector<std::string>;

        using module_decl = dotted_name_decl;
        using import_decl = dotted_name_decl;

        struct struct_decl;
        struct tuple_decl;
        struct variant_decl;
        struct enum_decl;
        struct alias_decl;
        struct array_decl;
        struct ptr_decl;
        struct ref_decl;

        using type_decl = std::variant<
            struct_decl, 
            tuple_decl, 
            variant_decl, 
            enum_decl, 
            alias_decl, 
            array_decl, 
            ptr_decl, 
            ref_decl
        >;

        struct struct_decl
        {
            std::map<std::string, type_decl> fields;
        };

        struct tuple_decl
        {
            std::vector<type_decl> fields;
        };

        struct variant_decl
        {
            std::map<std::string, type_decl> fields;
        };

        using plain_enum_decl = std::vector<std::string>;

        using valued_enum_decl = std::map<std::string, type_decl>;

        struct enum_decl
        {
            std::variant<plain_enum_decl, valued_enum_decl> body;
        };

        struct alias_decl
        {
            std::string name;
        };

        struct array_decl
        {
            type_decl type;
            expr size;
        };

        struct ptr_decl
        {
            type_decl type;
        };

        struct ref_decl
        {
            type_decl type;
        };

        struct using_decl
        {
            std::string name;

            type_decl type;
        };

        struct func_decl
        {
            std::string name;
        };

        using body_decl = std::variant<using_decl, func_decl>;

        struct ast
        {
            module_decl module_decl;
            std::vector<import_decl> import_decls;
            std::vector<body_decl> body_decls;
        };
    };

    std::unique_ptr<ast::ast> parse(std::istream& in);
}