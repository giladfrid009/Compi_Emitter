#ifndef _ABSTRACT_SYNTAX_HPP_
#define _ABSTRACT_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "types.hpp"
#include "code_buffer.hpp"
#include <vector>
#include <string>
#include <list>

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

    void emit();
    virtual void emit_init();
    virtual void emit_node() = 0;
    virtual void emit_clean();

    protected:

    void push_back_child(syntax_base* child);
    void push_front_child(syntax_base* child);
};

class expression_syntax: public syntax_base
{
    public:

    const type_kind return_type;

    const std::string place;
    const std::string label;
    std::list<patch_record> true_list;
    std::list<patch_record> false_list;
    std::list<patch_record> jump_list;

    expression_syntax(type_kind return_type);
    virtual ~expression_syntax() = default;

    expression_syntax(const expression_syntax& other) = delete;
    expression_syntax& operator=(const expression_syntax& other) = delete;

    bool is_numeric() const;
    bool is_special() const;

    protected:

    void emit_init() override;
    void emit_clean() override;
};

class statement_syntax: public syntax_base
{
    public:

    std::list<patch_record> next_list;
    std::list<patch_record> break_list;
    std::list<patch_record> continue_list;
    const std::string label;
    
    statement_syntax();
    virtual ~statement_syntax() = default;

    statement_syntax(const statement_syntax& other) = delete;
    statement_syntax& operator=(const statement_syntax& other) = delete;

    protected:

    void emit_init() override;
    void emit_clean() override;
};

#endif