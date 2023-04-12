#include "statement_syntax.hpp"
#include "../output.hpp"
#include "../symbol/symbol_table.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <algorithm>
#include <stdexcept>

using std::string;
using std::list;

static symbol_table& sym_tab = symbol_table::instance();
static code_buffer& code_buf = code_buffer::instance();

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body):
    if_token(if_token), condition(condition), body(body), else_token(nullptr), else_clause(nullptr)
{
    analyze();
    add_children({ condition , body });
}

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause):
    if_token(if_token), condition(condition), body(body), else_token(else_token), else_clause(else_clause)
{
    analyze();
    add_children({ condition , body, else_clause });
}

if_statement::~if_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete if_token;
    delete else_token;
}

void if_statement::analyze() const
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }
}

void if_statement::emit()
{
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    condition->emit();

    if (else_clause == nullptr)
    {
        code_buf.emit("br i1 %s, label %%%s, label %%%s", condition->reg, true_label, end_label);

        code_buf.increase_indent();
        code_buf.emit("%s:", true_label);
        body->emit();
        code_buf.emit("br label %%%s", end_label);
        code_buf.decrease_indent();

        code_buf.emit("%s:", end_label);

        break_list = body->break_list;
        continue_list = body->continue_list;
    }
    else
    {
        code_buf.emit("br i1 %s, label %%%s, label %%%s", condition->reg, true_label, false_label);

        code_buf.increase_indent();
        code_buf.emit("%s:", true_label);
        body->emit();
        code_buf.emit("br label %%%s", end_label);
        code_buf.decrease_indent();

        code_buf.increase_indent();
        code_buf.emit("%s:", false_label);
        else_clause->emit();
        code_buf.emit("br label %%%s", end_label);
        code_buf.decrease_indent();

        code_buf.emit("%s:", end_label);

        break_list.merge(body->break_list);
        break_list.merge(else_clause->break_list);
        continue_list.merge(body->continue_list);
        continue_list.merge(else_clause->continue_list);
    }
}

while_statement::while_statement(syntax_token* while_token, expression_syntax* condition, statement_syntax* body):
    while_token(while_token), condition(condition), body(body)
{
    analyze();
    add_children({ condition, body });
}

while_statement::~while_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete while_token;
}

void while_statement::analyze() const
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(while_token->position);
    }
}

void while_statement::emit()
{
    string cond_label = ir_builder::fresh_label();
    string body_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    code_buf.emit("br label %%%s", cond_label);
    code_buf.emit("%s:", cond_label);
    condition->emit();
    code_buf.emit("br i1 %s, label %%%s, label %%%s", condition->reg, body_label, end_label);

    code_buf.increase_indent();
    code_buf.emit("%s:", body_label);
    body->emit();
    code_buf.emit("br label %%%s", cond_label);
    code_buf.decrease_indent();

    code_buf.emit("%s:", end_label);

    code_buf.backpatch(body->break_list, end_label);
    code_buf.backpatch(body->continue_list, cond_label);

    body->break_list.clear();
    body->continue_list.clear();
}

branch_statement::branch_statement(syntax_token* branch_token): branch_token(branch_token), kind(parse_kind(branch_token->text))
{
    analyze();
}

branch_statement::~branch_statement()
{
    for (syntax_base* child : children())
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

void branch_statement::analyze() const
{
    const list<scope>& scopes = sym_tab.scopes();

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

void branch_statement::emit()
{
    size_t line = code_buf.emit("br label @");

    if (kind == branch_kind::Continue)
    {
        continue_list.push_back(line);
    }
    else
    {
        break_list.push_back(line);
    }
}

return_statement::return_statement(syntax_token* return_token): return_token(return_token), value(nullptr)
{
    analyze();
}

return_statement::return_statement(syntax_token* return_token, expression_syntax* value): return_token(return_token), value(value)
{
    analyze();
    add_child(value);
}

return_statement::~return_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete return_token;
}

void return_statement::analyze() const
{
    auto& global_symbols = sym_tab.scopes().front().symbols();

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
        if (types::is_implicitly_convertible(value->return_type, func_sym->type) == false)
        {
            output::error_mismatch(return_token->position);
        }
    }
}

void return_statement::emit()
{
    if (value == nullptr)
    {
        code_buf.emit("ret void");
    }
    else
    {
        value->emit();

        code_buf.emit("ret %s %s", ir_builder::get_ir_type(value->return_type), value->reg);
    }
}

expression_statement::expression_statement(expression_syntax* expression): expression(expression)
{
    add_child(expression);
}

expression_statement::~expression_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }
}

void expression_statement::analyze() const
{
}

void expression_statement::emit()
{
    expression->emit();
}

assignment_statement::assignment_statement(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value), _ptr_reg()
{
    analyze();

    const symbol* symbol = sym_tab.get_symbol(identifier);

    this->_ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;

    add_child(value);
}

assignment_statement::~assignment_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete identifier_token;
    delete assign_token;
}

void assignment_statement::analyze() const
{
    const symbol* symbol = sym_tab.get_symbol(identifier);

    if (symbol == nullptr)
    {
        output::error_undef(identifier_token->position, identifier);
    }

    if (symbol->kind != symbol_kind::Variable && symbol->kind != symbol_kind::Parameter)
    {
        output::error_undef(identifier_token->position, identifier);
    }

    if (types::is_implicitly_convertible(value->return_type, symbol->type) == false)
    {
        output::error_mismatch(assign_token->position);
    }
}

void assignment_statement::emit()
{
    string res_type = ir_builder::get_ir_type(value->return_type);

    value->emit();

    code_buf.emit("store %s %s, %s* %s", res_type, value->reg, res_type, _ptr_reg);
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(nullptr), value(nullptr), _ptr_reg()
{
    analyze();

    sym_tab.add_variable(identifier, type->kind);

    const symbol* sym = sym_tab.get_symbol(identifier, symbol_kind::Variable);

    this->_ptr_reg = static_cast<const variable_symbol*>(sym)->ptr_reg;

    add_child(type);
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value), _ptr_reg()
{
    analyze();

    sym_tab.add_variable(identifier, type->kind);

    const symbol* sym = sym_tab.get_symbol(identifier, symbol_kind::Variable);

    this->_ptr_reg = static_cast<const variable_symbol*>(sym)->ptr_reg;

    add_children({ type ,value });
}

declaration_statement::~declaration_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete identifier_token;
    delete assign_token;
}

void declaration_statement::analyze() const
{
    if (type->is_special())
    {
        output::error_mismatch(identifier_token->position);
    }

    if (value != nullptr)
    {
        if (types::is_implicitly_convertible(value->return_type, type->kind) == false)
        {
            output::error_mismatch(identifier_token->position);
        }
    }

    if (sym_tab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }
}

void declaration_statement::emit()
{
    string res_type = ir_builder::get_ir_type(this->type->kind);

    if (value != nullptr)
    {
        value->emit();
    }

    code_buf.emit("%s = alloca %s", _ptr_reg, res_type);

    if (value != nullptr)
    {
        code_buf.emit("store %s %s, %s* %s", res_type, value->reg, res_type, _ptr_reg);
    }
    else
    {
        code_buf.emit("store %s 0, %s* %s", res_type, res_type, _ptr_reg);
    }
}

block_statement::block_statement(list_syntax<statement_syntax>* statements): statements(statements)
{
    add_child(statements);
}

block_statement::~block_statement()
{
    for (syntax_base* child : children())
    {
        delete child;
    }
}

void block_statement::analyze() const
{
}

void block_statement::emit()
{
    statements->emit();

    for (auto statement : *statements)
    {
        break_list.merge(statement->break_list);
        continue_list.merge(statement->continue_list);
    }
}
