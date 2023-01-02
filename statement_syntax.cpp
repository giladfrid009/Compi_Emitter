#include "statement_syntax.hpp"
#include "output.hpp"
#include "symbol_table.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <algorithm>
#include <stdexcept>

using std::string;
using std::vector;
using std::list;

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body):
    if_token(if_token), condition(condition), body(body), else_token(nullptr), else_clause(nullptr)
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }

    push_back_child(condition);
    push_back_child(body);
}

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause):
    if_token(if_token), condition(condition), body(body), else_token(else_token), else_clause(else_clause)
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }

    push_back_child(condition);
    push_back_child(body);
    push_back_child(else_clause);
}

if_statement::~if_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete if_token;
    delete else_token;
}

void if_statement::emit()
{
}

while_statement::while_statement(syntax_token* while_token, expression_syntax* condition, statement_syntax* body):
    while_token(while_token), condition(condition), body(body)
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(while_token->position);
    }

    push_back_child(condition);
    push_back_child(body);
}

while_statement::~while_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete while_token;
}

void while_statement::emit()
{
}

branch_statement::branch_statement(syntax_token* branch_token):
    branch_token(branch_token), kind(parse_kind(branch_token->text))
{
    const list<scope>& scopes = symbol_table::instance().get_scopes();

    if (std::all_of(scopes.rbegin(), scopes.rend(), [](const scope& sc) { return sc.loop_scope == false; }))
    {
        if (kind == branch_kind::Break)
        {
            output::error_unexpected_break(branch_token->position);
        }

        if (kind == branch_kind::Continue)
        {
            output::error_unexpected_continue(branch_token->position);
        }

        throw std::runtime_error("unknown branch_kind");
    }
}

branch_statement::~branch_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete branch_token;
}

branch_statement::branch_kind branch_statement::parse_kind(string str)
{
    if (str == "break") return branch_kind::Break;
    if (str == "continue") return branch_kind::Continue;

    throw std::invalid_argument("unknown type");
}

void branch_statement::emit()
{
}

return_statement::return_statement(syntax_token* return_token):
    return_token(return_token), value(nullptr)
{
    auto& global_symbols = symbol_table::instance().get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (func_sym->type != type_kind::Void)
    {
        output::error_mismatch(return_token->position);
    }
}

return_statement::return_statement(syntax_token* return_token, expression_syntax* value):
    return_token(return_token), value(value)
{
    auto& global_symbols = symbol_table::instance().get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (types::is_implictly_convertible(value->return_type, func_sym->type) == false)
    {
        output::error_mismatch(return_token->position);
    }

    push_back_child(value);
}

return_statement::~return_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete return_token;
}

void return_statement::emit()
{
}

expression_statement::expression_statement(expression_syntax* expression): expression(expression)
{
    push_back_child(expression);
}

expression_statement::~expression_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void expression_statement::emit()
{
}

assignment_statement::assignment_statement(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value)
{
    const symbol* identifier_symbol = symbol_table::instance().get_symbol(identifier);

    if (identifier_symbol == nullptr || identifier_symbol->kind != symbol_kind::Variable)
    {
        output::error_undef(identifier_token->position, identifier);
    }

    if (types::is_implictly_convertible(value->return_type, identifier_symbol->type) == false)
    {
        output::error_mismatch(assign_token->position);
    }

    push_back_child(value);
}

assignment_statement::~assignment_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
    delete assign_token;
}

void assignment_statement::emit()
{
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(nullptr), value(nullptr)
{
    if (type->is_special())
    {
        output::error_mismatch(identifier_token->position);
    }

    if (symbol_table::instance().contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    symbol_table::instance().add_variable(identifier, type->kind);

    push_back_child(type);
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value)
{
    if (type->is_special() || value->is_special())
    {
        output::error_mismatch(identifier_token->position);
    }

    if (types::is_implictly_convertible(value->return_type, type->kind) == false)
    {
        output::error_mismatch(identifier_token->position);
    }

    if (symbol_table::instance().contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    symbol_table::instance().add_variable(identifier, type->kind);

    push_back_child(type);
    push_back_child(value);
}

declaration_statement::~declaration_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
    delete assign_token;
}

void declaration_statement::emit()
{
    // todo: push variable to stack
    // generate random variable ptr name and save in symbol table? bye-bye decoupling.
}

block_statement::block_statement(list_syntax<statement_syntax>* statements): statements(statements)
{
    push_back_child(statements);
}

block_statement::~block_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void block_statement::emit()
{
}
