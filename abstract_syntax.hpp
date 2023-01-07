#ifndef _ABSTRACT_SYNTAX_HPP_
#define _ABSTRACT_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "types.hpp"
#include <vector>
#include <string>

class syntax_base
{
    private:

    syntax_base* parent = nullptr;

    public:

    syntax_base();

    syntax_base(const syntax_base& other) = delete;

    syntax_base& operator=(const syntax_base& other) = delete;

    syntax_base* get_parent() const;

    void set_parent(syntax_base* new_parent);

    virtual ~syntax_base() = default;

    virtual std::vector<syntax_base*> get_children() const = 0;

    virtual std::vector<syntax_token*> get_tokens() const = 0;

    virtual void emit() = 0;
};

class expression_syntax: public syntax_base
{
    public:

    const std::string place;

    const fundamental_type return_type;

    expression_syntax(fundamental_type return_type);

    expression_syntax(const expression_syntax& other) = delete;

    expression_syntax& operator=(const expression_syntax& other) = delete;

    bool is_numeric() const;

    bool is_special() const;

    virtual ~expression_syntax() = default;

    virtual std::vector<syntax_base*> get_children() const = 0;

    virtual std::vector<syntax_token*> get_tokens() const = 0;

    virtual void emit() = 0;
};

class statement_syntax: public syntax_base
{
    public:

    statement_syntax();

    virtual ~statement_syntax() = default;

    statement_syntax(const statement_syntax& other) = delete;

    statement_syntax& operator=(const statement_syntax& other) = delete;

    virtual std::vector<syntax_base*> get_children() const = 0;

    virtual std::vector<syntax_token*> get_tokens() const = 0;

    virtual void emit() = 0;
};

#endif