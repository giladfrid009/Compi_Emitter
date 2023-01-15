#include "statement_syntax.hpp"
#include "hw3_output.hpp"
#include "symbol_table.hpp"
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
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }

    add_child(condition);
    add_child(body);

    emit();
}

if_statement::if_statement(syntax_token* if_token, expression_syntax* condition, statement_syntax* body, syntax_token* else_token, statement_syntax* else_clause):
    if_token(if_token), condition(condition), body(body), else_token(else_token), else_clause(else_clause)
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }

    add_child(condition);
    add_child(body);
    add_child(else_clause);

    emit();
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

void if_statement::emit_node()
{
}

while_statement::while_statement(syntax_token* while_token, expression_syntax* condition, statement_syntax* body):
    while_token(while_token), condition(condition), body(body)
{
    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(while_token->position);
    }

    add_child(condition);
    add_child(body);

    emit();
}

while_statement::~while_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete while_token;
}

void while_statement::emit_node()
{
}

branch_statement::branch_statement(syntax_token* branch_token): branch_token(branch_token), kind(parse_kind(branch_token->text))
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

    emit();
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

void branch_statement::emit_node()
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
}

return_statement::return_statement(syntax_token* return_token): return_token(return_token), value(nullptr)
{
    auto& global_symbols = symtab.get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (func_sym->type != type_kind::Void)
    {
        output::error_mismatch(return_token->position);
    }

    emit();
}

return_statement::return_statement(syntax_token* return_token, expression_syntax* value): return_token(return_token), value(value)
{
    auto& global_symbols = symtab.get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (types::is_implictly_convertible(value->return_type, func_sym->type) == false)
    {
        output::error_mismatch(return_token->position);
    }

    add_child(value);

    emit();
}

return_statement::~return_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete return_token;
}

void return_statement::emit_node()
{
    if (value == nullptr)
    {
        codebuf.emit("ret void");
        return;
    }

    codebuf.backpatch(value->jump_list, value->jump_label);

    if (value->return_type != type_kind::Bool)
    {
        codebuf.emit("ret %s %s", ir_builder::get_type(value->return_type), value->place);
    }
    else
    {
        codebuf.emit("ret i32 %s", get_bool_reg(value));
    }
}

expression_statement::expression_statement(expression_syntax* expression): expression(expression)
{
    add_child(expression);

    emit();
}

expression_statement::~expression_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void expression_statement::emit_node()
{
    codebuf.backpatch(expression->jump_list, expression->jump_label);
}

assignment_statement::assignment_statement(syntax_token* identifier_token, syntax_token* assign_token, expression_syntax* value):
    identifier_token(identifier_token), identifier(identifier_token->text), assign_token(assign_token), value(value)
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

    add_child(value);

    emit();
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

void assignment_statement::emit_node()
{
    codebuf.backpatch(value->jump_list, value->jump_label);

    const symbol* symbol = symtab.get_symbol(identifier);

    string ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;

    if (value->return_type != type_kind::Bool)
    {
        string type = ir_builder::get_type(value->return_type);

        codebuf.emit("store %s %s , %s* %s", type, value->place, type, ptr_reg);

        return;
    }
    else
    {
        codebuf.emit("store i32 %s , i32* %s", get_bool_reg(value), ptr_reg);
    }
}

declaration_statement::declaration_statement(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text), assign_token(nullptr), value(nullptr)
{
    if (type->is_special())
    {
        output::error_mismatch(identifier_token->position);
    }

    if (symtab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    symtab.add_variable(identifier, type->kind);

    add_child(type);

    emit();
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

    if (symtab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    symtab.add_variable(identifier, type->kind);

    add_child(type);
    add_child(value);

    emit();
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

void declaration_statement::emit_node()
{
    string ptr_reg = static_cast<const variable_symbol*>(symtab.get_symbol(identifier, symbol_kind::Variable))->ptr_reg;

    codebuf.emit("%s = alloca i32", ptr_reg);

    if (value == nullptr)
    {
        codebuf.emit("store i32 0 , i32* %s", ptr_reg);
        return;
    }

    codebuf.backpatch(value->jump_list, value->jump_label);

    if (value->return_type != type_kind::Bool)
    {
        codebuf.emit("store i32 %s , i32* %s", value->place, ptr_reg);
    }
    else
    {
        codebuf.emit("store i32 %s , i32* %s", get_bool_reg(value), ptr_reg);
    }
}

block_statement::block_statement(list_syntax<statement_syntax>* statements): statements(statements)
{
    add_child(statements);

    emit();
}

block_statement::~block_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void block_statement::emit_node()
{
    statement_syntax* prev_statement = nullptr;

    for (auto statement : *statements)
    {
        break_list = codebuf.merge(break_list, statement->break_list);
        continue_list = codebuf.merge(continue_list, statement->continue_list);
    }
}
