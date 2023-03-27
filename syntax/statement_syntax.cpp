#include "statement_syntax.hpp"
#include "../output.hpp"
#include "../symbol/symbol_table.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <algorithm>
#include <stdexcept>

using std::string;
using std::list;

static symbol_table& symtab = symbol_table::instance();
static code_buffer& codebuf = code_buffer::instance();

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body):
    if_token(if_token), condition(condition), body(body), else_token(nullptr), else_clause(nullptr)
{
    semantic_analysis();

    add_child(condition);
    add_child(body);
}

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause):
    if_token(if_token), condition(condition), body(body), else_token(else_token), else_clause(else_clause)
{
    semantic_analysis();

    add_child(condition);
    add_child(body);
    add_child(else_clause);
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

void if_statement::semantic_analysis() const
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }
}

void if_statement::emit_code()
{
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    condition->emit_code();

    if (else_clause == nullptr)
    {
        codebuf.emit("br i1 %s , label %%%s, label %%%s", condition->reg, true_label, end_label);

        codebuf.increase_indent();
        codebuf.emit("%s:", true_label);
        body->emit_code();
        codebuf.emit("br label %%%s", end_label);
        codebuf.decrease_indent();

        codebuf.emit("%s:", end_label);

        break_list = body->break_list;
        continue_list = body->continue_list;
    }
    else
    {
        codebuf.emit("br i1 %s , label %%%s, label %%%s", condition->reg, true_label, false_label);
        
        codebuf.increase_indent();
        codebuf.emit("%s:", true_label);
        body->emit_code();
        codebuf.emit("br label %%%s", end_label);
        codebuf.decrease_indent();


        codebuf.increase_indent();
        codebuf.emit("%s:", false_label);
        else_clause->emit_code();
        codebuf.emit("br label %%%s", end_label);
        codebuf.decrease_indent();

        codebuf.emit("%s:", end_label);

        break_list =  codebuf.merge(body->break_list, else_clause->break_list);
        continue_list = codebuf.merge(body->continue_list, else_clause->continue_list);
    }

    codebuf.new_line();
}

while_statement::while_statement(syntax_token* while_token, expression_syntax* condition, statement_syntax* body):
    while_token(while_token), condition(condition), body(body)
{
    semantic_analysis();

    add_child(condition);
    add_child(body);
}

while_statement::~while_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete while_token;
}

void while_statement::semantic_analysis() const
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(while_token->position);
    }
}

void while_statement::emit_code()
{
    string cond_label = ir_builder::fresh_label();
    string body_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    codebuf.emit("br label %%%s", cond_label);
    codebuf.emit("%s:", cond_label);
    condition->emit_code();
    codebuf.emit("br i1 %s , label %%%s, label %%%s", condition->reg, body_label, end_label);

    codebuf.increase_indent();
    codebuf.emit("%s:", body_label);
    body->emit_code();
    codebuf.emit("br label %%%s", cond_label);
    codebuf.decrease_indent();

    codebuf.emit("%s:", end_label);
    codebuf.new_line();

    codebuf.backpatch(body->break_list, end_label);
    codebuf.backpatch(body->continue_list, cond_label);

    body->break_list.clear();
    body->continue_list.clear();
}

branch_statement::branch_statement(syntax_token* branch_token): branch_token(branch_token), kind(parse_kind(branch_token->text))
{
    semantic_analysis();
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

void branch_statement::semantic_analysis() const
{
    const list<scope>& scopes = symtab.get_scopes();

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

void branch_statement::emit_code()
{
    size_t line = codebuf.emit("br label @");

    if (kind == branch_kind::Continue)
    {
        continue_list.push_back(patch_record(line, label_index::First));
    }
    else
    {
        break_list.push_back(patch_record(line, label_index::First));
    }

    codebuf.new_line();
}

return_statement::return_statement(syntax_token* return_token): return_token(return_token), value(nullptr)
{
    semantic_analysis();
}

return_statement::return_statement(syntax_token* return_token, expression_syntax* value): return_token(return_token), value(value)
{
    semantic_analysis();

    add_child(value);
}

return_statement::~return_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete return_token;
}

void return_statement::semantic_analysis() const
{
    auto& global_symbols = symtab.get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (value == nullptr)
    {
        if (func_sym->type != type_kind::Void)
        {
            output::error_mismatch(return_token->position);
        }
    }
    else
    {
        if (types::is_implictly_convertible(value->return_type, func_sym->type) == false)
        {
            output::error_mismatch(return_token->position);
        }
    }
}

void return_statement::emit_code()
{
    if (value == nullptr)
    {
        codebuf.emit("ret void");
    }
    else
    {
        value->emit_code();

        codebuf.emit("ret %s %s", ir_builder::get_type(value->return_type), value->reg);
    }

    codebuf.new_line();
}

expression_statement::expression_statement(expression_syntax* expression): expression(expression)
{
    add_child(expression);
}

expression_statement::~expression_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void expression_statement::semantic_analysis() const
{
}

void expression_statement::emit_code()
{
    expression->emit_code();

    codebuf.new_line();
}

assignment_statement::assignment_statement(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value), ptr_reg()
{
    semantic_analysis();

    const symbol* symbol = symtab.get_symbol(identifier);

    this->ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;

    add_child(value);
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

void assignment_statement::semantic_analysis() const
{
    const symbol* symbol = symtab.get_symbol(identifier);

    if (symbol == nullptr)
    {
        output::error_undef(identifier_token->position, identifier);
    }

    if (symbol->kind != symbol_kind::Variable && symbol->kind != symbol_kind::Parameter)
    {
        output::error_undef(identifier_token->position, identifier);
    }

    if (types::is_implictly_convertible(value->return_type, symbol->type) == false)
    {
        output::error_mismatch(assign_token->position);
    }
}

void assignment_statement::emit_code()
{
    string res_type = ir_builder::get_type(value->return_type);

    value->emit_code();

    codebuf.emit("store %s %s , %s* %s", res_type, value->reg, res_type, ptr_reg);

    codebuf.new_line();
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(nullptr), value(nullptr), ptr_reg()
{
    semantic_analysis();

    symtab.add_variable(identifier, type->kind);

    const symbol* sym = symtab.get_symbol(identifier, symbol_kind::Variable);

    this->ptr_reg = static_cast<const variable_symbol*>(sym)->ptr_reg;

    add_child(type);
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value), ptr_reg()
{
    semantic_analysis();

    symtab.add_variable(identifier, type->kind);

    const symbol* sym = symtab.get_symbol(identifier, symbol_kind::Variable);

    this->ptr_reg = static_cast<const variable_symbol*>(sym)->ptr_reg;

    add_child(type);
    add_child(value);
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

void declaration_statement::semantic_analysis() const
{
    if (type->is_special())
    {
        output::error_mismatch(identifier_token->position);
    }

    if (value != nullptr)
    {
        if (types::is_implictly_convertible(value->return_type, type->kind) == false)
        {
            output::error_mismatch(identifier_token->position);
        }
    }

    if (symtab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }
}

void declaration_statement::emit_code()
{
    string res_type = ir_builder::get_type(this->type->kind);

    if (value != nullptr)
    {
        value->emit_code();
    }

    codebuf.emit("%s = alloca %s", ptr_reg, res_type);

    if (value != nullptr)
    {
        codebuf.emit("store %s %s , %s* %s", res_type, value->reg, res_type, ptr_reg);
    }
    else
    {
        codebuf.emit("store %s 0 , %s* %s", res_type, res_type, ptr_reg);
    }

    codebuf.new_line();
}

block_statement::block_statement(list_syntax<statement_syntax>* statements): statements(statements)
{
    add_child(statements);
}

block_statement::~block_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void block_statement::semantic_analysis() const
{
}

void block_statement::emit_code()
{
    codebuf.increase_indent();
    statements->emit_code();
    codebuf.decrease_indent();

    for (auto statement : *statements)
    {
        break_list = codebuf.merge(break_list, statement->break_list);
        continue_list = codebuf.merge(continue_list, statement->continue_list);
    }
}
