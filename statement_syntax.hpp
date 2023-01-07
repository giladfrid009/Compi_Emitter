#ifndef _STATEMENT_SYNTAX_HPP_
#define _STATEMENT_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include "expression_syntax.hpp"
#include "generic_syntax.hpp"
#include <vector>
#include <string>

class if_statement_syntax final: public statement_syntax
{
    public:

    syntax_token* const if_token;
    expression_syntax* const condition;
    statement_syntax* const body;
    syntax_token* const else_token;
    statement_syntax* const else_clause;

    if_statement_syntax(syntax_token* if_token, expression_syntax* condition, statement_syntax* body);

    if_statement_syntax(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause);

    if_statement_syntax(const if_statement_syntax& other) = delete;

    if_statement_syntax& operator=(const if_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~if_statement_syntax();
};

class while_statement_syntax final: public statement_syntax
{
    public:

    syntax_token* const while_token;
    expression_syntax* const condition;
    statement_syntax* const body;

    while_statement_syntax(syntax_token* while_token, expression_syntax* condition, statement_syntax* body);

    while_statement_syntax(const while_statement_syntax& other) = delete;

    while_statement_syntax& operator=(const while_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~while_statement_syntax();
};

class branch_statement_syntax final: public statement_syntax
{
    public:

    enum class branch_type { Break, Continue };

    syntax_token* const branch_token;
    const branch_type type;

    branch_statement_syntax(syntax_token* branch_token);

    branch_statement_syntax(const branch_statement_syntax& other) = delete;

    branch_statement_syntax& operator=(const branch_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~branch_statement_syntax();

    static branch_type parse_type(std::string str);
};

class return_statement_syntax final: public statement_syntax
{
    public:

    syntax_token* const return_token;
    expression_syntax* const expression;

    return_statement_syntax(syntax_token* return_token);

    return_statement_syntax(syntax_token* return_token, expression_syntax* expression);

    return_statement_syntax(const return_statement_syntax& other) = delete;

    return_statement_syntax& operator=(const return_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~return_statement_syntax();
};

class expression_statement_syntax final: public statement_syntax
{
    public:

    expression_syntax* const expression;

    expression_statement_syntax(expression_syntax* expression);

    expression_statement_syntax(const expression_statement_syntax& other) = delete;

    expression_statement_syntax& operator=(const expression_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~expression_statement_syntax();
};

class assignment_statement_syntax final: public statement_syntax
{
    public:

    syntax_token* const identifier_token;
    const std::string identifier;
    syntax_token* const assign_token;
    expression_syntax* const value;

    assignment_statement_syntax(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value);

    assignment_statement_syntax(const assignment_statement_syntax& other) = delete;

    assignment_statement_syntax& operator=(const assignment_statement_syntax& other) = delete;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~assignment_statement_syntax();
};

class declaration_statement_syntax final: public statement_syntax
{
    public:

    type_syntax* const type;
    syntax_token* const identifier_token;
    const std::string identifier;
    syntax_token* const assign_token;
    expression_syntax* const value;

    declaration_statement_syntax(type_syntax* type, syntax_token* identifier_token);

    declaration_statement_syntax(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value);

    declaration_statement_syntax(const declaration_statement_syntax& other) = delete;

    declaration_statement_syntax& operator=(const declaration_statement_syntax& other) = delete;

    void emit() override;
    
    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~declaration_statement_syntax();
};

class block_statement_syntax final: public statement_syntax
{
    public:

    list_syntax<statement_syntax>* const statement_list;

    block_statement_syntax(list_syntax<statement_syntax>* statement_list);

    block_statement_syntax(const block_statement_syntax& other) = delete;

    block_statement_syntax& operator=(const block_statement_syntax& other) = delete;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~block_statement_syntax();
};

#endif