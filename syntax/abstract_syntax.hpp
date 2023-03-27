#ifndef _ABSTRACT_SYNTAX_HPP_
#define _ABSTRACT_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "../types.hpp"
#include "../emit/code_buffer.hpp"
#include <vector>
#include <string>
#include <list>

class expression_syntax;

class syntax_base
{
    private:

    std::list<syntax_base*> children;
    syntax_base* parent = nullptr;

    public:

    syntax_base();
    virtual ~syntax_base() = default;

    syntax_base(const syntax_base& other) = delete;
    syntax_base& operator=(const syntax_base& other) = delete;

    const syntax_base* get_parent() const;
    const std::list<syntax_base*>& get_children() const;

    virtual void semantic_analysis() const = 0;
    virtual void emit_code() = 0;

    protected:

    void add_child(syntax_base* child);
    void add_child_front(syntax_base* child);
};

class expression_syntax: public syntax_base
{
    public:

    const type_kind return_type;
    const std::string reg;

    expression_syntax(type_kind return_type);
    virtual ~expression_syntax() = default;

    expression_syntax(const expression_syntax& other) = delete;
    expression_syntax& operator=(const expression_syntax& other) = delete;

    bool is_numeric() const;
    bool is_special() const;
};

class statement_syntax: public syntax_base
{
    public:

    std::list<patch_record> break_list;
    std::list<patch_record> continue_list;

    statement_syntax();
    virtual ~statement_syntax() = default;

    statement_syntax(const statement_syntax& other) = delete;
    statement_syntax& operator=(const statement_syntax& other) = delete;
};

#endif