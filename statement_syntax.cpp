#include "statement_syntax.hpp"
#include "hw3_output.hpp"
#include "symbol_table.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <algorithm>
#include <stdexcept>

using std::string;
using std::vector;
using std::list;

if_statement_syntax::if_statement_syntax(syntax_token* if_token, expression_syntax* condition, statement_syntax* body):
    if_token(if_token), condition(condition), body(body), else_token(nullptr), else_clause(nullptr)
{
    if (condition->return_type != fundamental_type::Bool)
    {
        output::errorMismatch(if_token->definition_line);
    }

    condition->set_parent(this);
    body->set_parent(this);
}

if_statement_syntax::if_statement_syntax(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause):
    if_token(if_token), condition(condition), body(body), else_token(else_token), else_clause(else_clause)
{
    if (condition->return_type != fundamental_type::Bool)
    {
        output::errorMismatch(if_token->definition_line);
    }

    condition->set_parent(this);
    body->set_parent(this);
    else_clause->set_parent(this);
}

vector<syntax_base*> if_statement_syntax::get_children() const
{
    return vector<syntax_base*>{condition, body, else_clause};
}

vector<syntax_token*> if_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{if_token, else_token};
}

void if_statement_syntax::emit()
{
    
}

if_statement_syntax::~if_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

while_statement_syntax::while_statement_syntax(syntax_token* while_token, expression_syntax* condition, statement_syntax* body):
    while_token(while_token), condition(condition), body(body)
{
    if (condition->return_type != fundamental_type::Bool)
    {
        output::errorMismatch(while_token->definition_line);
    }

    condition->set_parent(this);
    body->set_parent(this);
}

vector<syntax_base*> while_statement_syntax::get_children() const
{
    return vector<syntax_base*>{condition, body};
}

vector<syntax_token*> while_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{while_token};
}

void while_statement_syntax::emit()
{
    
}

while_statement_syntax::~while_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

branch_statement_syntax::branch_statement_syntax(syntax_token* branch_token):
    branch_token(branch_token), type(parse_type(branch_token->text))
{
    const list<scope>& scopes = symbol_table::instance().get_scopes();

    if (std::all_of(scopes.rbegin(), scopes.rend(), [](const scope& sc) { return sc.is_loop_scope == false; }))
    {
        if (type == branch_type::Break)
        {
            output::errorUnexpectedBreak(branch_token->definition_line);
        }

        if (type == branch_type::Continue)
        {
            output::errorUnexpectedContinue(branch_token->definition_line);
        }

        throw std::runtime_error("unknown branch_type");
    }
}

vector<syntax_base*> branch_statement_syntax::get_children() const
{
    return vector<syntax_base*>();
}

vector<syntax_token*> branch_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{branch_token};
}

void branch_statement_syntax::emit()
{
    
}

branch_statement_syntax::~branch_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

branch_statement_syntax::branch_type branch_statement_syntax::parse_type(string str)
{
    if (str == "break") return branch_type::Break;
    if (str == "continue") return branch_type::Continue;

    throw std::invalid_argument("unknown type");
}

return_statement_syntax::return_statement_syntax(syntax_token* return_token):
    return_token(return_token), expression(nullptr)
{
    const list<symbol*>& global_symbols = symbol_table::instance().get_scopes().front().get_symbols();

    symbol* func_sym = global_symbols.back();

    if (func_sym->type != fundamental_type::Void)
    {
        output::errorMismatch(return_token->definition_line);
    }
}

return_statement_syntax::return_statement_syntax(syntax_token* return_token, expression_syntax* expression):
    return_token(return_token), expression(expression)
{
    const list<symbol*>& global_symbols = symbol_table::instance().get_scopes().front().get_symbols();

    symbol* func_sym = global_symbols.back();

    if (types::is_convertible(expression->return_type, func_sym->type) == false)
    {
        output::errorMismatch(return_token->definition_line);
    }

    expression->set_parent(this);
}

vector<syntax_base*> return_statement_syntax::get_children() const
{
    return vector<syntax_base*>{expression};
}

vector<syntax_token*> return_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{return_token};
}

void return_statement_syntax::emit()
{
    
}

return_statement_syntax::~return_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

expression_statement_syntax::expression_statement_syntax(expression_syntax* expression): expression(expression)
{
    expression->set_parent(this);
}

vector<syntax_base*> expression_statement_syntax::get_children() const
{
    return vector<syntax_base*>{expression};
}

vector<syntax_token*> expression_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>();
}

void expression_statement_syntax::emit()
{
    
}

expression_statement_syntax::~expression_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

assignment_statement_syntax::assignment_statement_syntax(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value)
{
    symbol* identifier_symbol = symbol_table::instance().get_symbol(identifier);

    if (identifier_symbol == nullptr || identifier_symbol->sym_type != symbol_type::Var)
    {
        output::errorUndef(identifier_token->definition_line, identifier);
    }

    if (types::is_convertible(value->return_type, identifier_symbol->type) == false)
    {
        output::errorMismatch(assign_token->definition_line);
    }

    value->set_parent(this);
}

vector<syntax_base*> assignment_statement_syntax::get_children() const
{
    return vector<syntax_base*>{value};
}

vector<syntax_token*> assignment_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{identifier_token, assign_token};
}

void assignment_statement_syntax::emit()
{
    
}

assignment_statement_syntax::~assignment_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

declaration_statement_syntax::declaration_statement_syntax(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(nullptr), value(nullptr)
{
    if (type->is_special())
    {
        output::errorMismatch(identifier_token->definition_line);
    }

    if (symbol_table::instance().contains_symbol(identifier))
    {
        output::errorDef(identifier_token->definition_line, identifier);
    }

    symbol_table::instance().add_variable(identifier, type->type);

    type->set_parent(this);
}

declaration_statement_syntax::declaration_statement_syntax(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value)
{
    if (type->is_special() || value->is_special())
    {
        output::errorMismatch(identifier_token->definition_line);
    }

    if (types::is_convertible(value->return_type, type->type) == false)
    {
        output::errorMismatch(identifier_token->definition_line);
    }

    if (symbol_table::instance().contains_symbol(identifier))
    {
        output::errorDef(identifier_token->definition_line, identifier);
    }

    symbol_table::instance().add_variable(identifier, type->type);

    type->set_parent(this);
    value->set_parent(this);
}

vector<syntax_base*> declaration_statement_syntax::get_children() const
{
    return vector<syntax_base*>{type, value};
}

vector<syntax_token*> declaration_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>{identifier_token, assign_token};
}

void declaration_statement_syntax::emit()
{
    
}

declaration_statement_syntax::~declaration_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}

block_statement_syntax::block_statement_syntax(list_syntax<statement_syntax>* statement_list): statement_list(statement_list)
{
    statement_list->set_parent(this);
}

vector<syntax_base*> block_statement_syntax::get_children() const
{
    return vector<syntax_base*>{statement_list};
}

vector<syntax_token*> block_statement_syntax::get_tokens() const
{
    return vector<syntax_token*>();
}

void block_statement_syntax::emit()
{
    
}


block_statement_syntax::~block_statement_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    for (syntax_token* token : get_tokens())
    {
        delete token;
    }
}
