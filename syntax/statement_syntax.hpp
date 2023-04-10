#ifndef _STATEMENT_SYNTAX_HPP_
#define _STATEMENT_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include "expression_syntax.hpp"
#include "generic_syntax.hpp"
#include <vector>
#include <string>

class if_statement final: public statement_syntax
{
    public:

    const syntax_token* const if_token;
    expression_syntax* const condition;
    statement_syntax* const body;
    const syntax_token* const else_token;
    statement_syntax* const else_clause;

    if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body);
    if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause);
    ~if_statement();

    if_statement(const if_statement& other) = delete;
    if_statement& operator=(const if_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class while_statement final: public statement_syntax
{
    public:

    const syntax_token* const while_token;
    expression_syntax* const condition;
    statement_syntax* const body;

    while_statement(syntax_token* while_token, expression_syntax* condition, statement_syntax* body);
    ~while_statement();

    while_statement(const while_statement& other) = delete;
    while_statement& operator=(const while_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class branch_statement final: public statement_syntax
{
    public:

    enum class branch_kind { Break, Continue };

    const syntax_token* const branch_token;
    const branch_kind kind;

    branch_statement(syntax_token* branch_token);
    ~branch_statement();

    branch_statement(const branch_statement& other) = delete;
    branch_statement& operator=(const branch_statement& other) = delete;

    void analyze() const override;
    void emit() override;

    private:

    static branch_kind parse_kind(std::string str);
};

class return_statement final: public statement_syntax
{
    public:

    const syntax_token* const return_token;
    expression_syntax* const value;

    return_statement(syntax_token* return_token);
    return_statement(syntax_token* return_token, expression_syntax* value);
    ~return_statement();

    return_statement(const return_statement& other) = delete;
    return_statement& operator=(const return_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class expression_statement final: public statement_syntax
{
    public:

    expression_syntax* const expression;

    expression_statement(expression_syntax* expression);
    ~expression_statement();

    expression_statement(const expression_statement& other) = delete;
    expression_statement& operator=(const expression_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class assignment_statement final: public statement_syntax
{
    public:

    const syntax_token* const identifier_token;
    const std::string identifier;
    const syntax_token* const assign_token;
    expression_syntax* const value;

    private:

    std::string _ptr_reg;

    public:

    assignment_statement(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value);
    ~assignment_statement();

    assignment_statement(const assignment_statement& other) = delete;
    assignment_statement& operator=(const assignment_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class declaration_statement final: public statement_syntax
{
    public:

    type_syntax* const type;
    const syntax_token* const identifier_token;
    const std::string identifier;
    const syntax_token* const assign_token;
    expression_syntax* const value;

    private:

    std::string _ptr_reg;

    public:

    declaration_statement(type_syntax* type, syntax_token* identifier_token);
    declaration_statement(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value);
    ~declaration_statement();

    declaration_statement(const declaration_statement& other) = delete;
    declaration_statement& operator=(const declaration_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

class block_statement final: public statement_syntax
{
    public:

    list_syntax<statement_syntax>* const statements;

    block_statement(list_syntax<statement_syntax>* statements);
    ~block_statement();

    block_statement(const block_statement& other) = delete;
    block_statement& operator=(const block_statement& other) = delete;

    void analyze() const override;
    void emit() override;
};

#endif