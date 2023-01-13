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

    push_back_child(condition);
    push_back_child(body);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void if_statement::emit_code()
{
    codebuf.backpatch(condition->jump_list, condition->label);

    if (else_token == nullptr)
    {
        codebuf.backpatch(condition->true_list, body->label);

        next_list = codebuf.merge(condition->false_list, body->next_list);
        
        break_list = body->break_list;
        continue_list = body->continue_list;
    }
    else
    {
        codebuf.backpatch(condition->true_list, body->label);
        codebuf.backpatch(condition->false_list, else_clause->label);

        next_list = codebuf.merge(body->next_list, else_clause->next_list);

        break_list = codebuf.merge(body->break_list, else_clause->continue_list);
        continue_list = codebuf.merge(body->continue_list, else_clause->continue_list);
    }
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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

while_statement::~while_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete while_token;
}

void while_statement::emit_code()
{
    codebuf.backpatch(condition->jump_list, condition->label);
    codebuf.backpatch(body->next_list, condition->label);
    codebuf.backpatch(condition->true_list, body->label);
    codebuf.backpatch(body->continue_list, condition->label);
    
    this->next_list = codebuf.merge(condition->false_list, body->break_list);

    codebuf.emit("br label %%%s", condition->label);
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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
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
}

return_statement::return_statement(syntax_token* return_token): return_token(return_token), value(nullptr)
{
    auto& global_symbols = symtab.get_scopes().front().get_symbols();

    const symbol* func_sym = global_symbols.back();

    if (func_sym->type != type_kind::Void)
    {
        output::error_mismatch(return_token->position);
    }

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

return_statement::return_statement(syntax_token* return_token, expression_syntax* value): return_token(return_token), value(value)
{
    auto& global_symbols = symtab.get_scopes().front().get_symbols();

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

void return_statement::emit_code()
{
    if (value == nullptr)
    {
        codebuf.emit("ret void");
        return;
    }

    codebuf.backpatch(value->jump_list, value->label);
    
    if (value->return_type == type_kind::Bool)
    {
        string bool_reg = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();
        string next_label = ir_builder::fresh_label();

        codebuf.backpatch(value->true_list, true_label);
        codebuf.backpatch(value->false_list, false_label);

        codebuf.emit("%s:", true_label);

        codebuf.emit("br label %%%s", next_label);

        codebuf.emit("%s:", false_label);

        codebuf.emit("br label %%%s", next_label);

        codebuf.emit("%s:", false_label);

        codebuf.emit("%s = phi i32 [ 1 , %s ] [ 0 , %s ]", bool_reg, true_label, false_label);

        codebuf.emit("ret i32 %s", bool_reg);
    }
    else
    {
        codebuf.emit("ret %s %s", ir_builder::get_type(value->return_type), value->place);
    }
}

expression_statement::expression_statement(expression_syntax* expression): expression(expression)
{
    push_back_child(expression);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

expression_statement::~expression_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void expression_statement::emit_code()
{
    codebuf.backpatch(expression->jump_list, expression->label);
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

    push_back_child(value);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void assignment_statement::emit_code()
{
    codebuf.backpatch(value->jump_list, value->label);

    const symbol* symbol = symtab.get_symbol(identifier);

    string ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;

    if (value->return_type == type_kind::Bool)
    {
        string bool_reg = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();
        string next_label = ir_builder::fresh_label();

        codebuf.backpatch(value->true_list, true_label);
        codebuf.backpatch(value->false_list, false_label);

        codebuf.emit("%s:", true_label);

        codebuf.emit("br label %%%s", next_label);

        codebuf.emit("%s:", false_label);

        codebuf.emit("br label %%%s", next_label);

        codebuf.emit("%s:", false_label);

        codebuf.emit("%s = phi i32 [ 1 , %s ] [ 0 , %s ]", bool_reg, true_label, false_label);

        codebuf.emit("store i32 %s , i32* %s", bool_reg, ptr_reg);
    }
    else
    {
        string type = ir_builder::get_type(value->return_type);

        codebuf.emit("store %s %s , %s* %s", type, value->place, type, ptr_reg);
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

    push_back_child(type);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

    push_back_child(type);
    push_back_child(value);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void declaration_statement::emit_code() //todo: implement
{
    if(value != nullptr)
    {
        codebuf.backpatch(value->jump_list, value->label);
    }
}

block_statement::block_statement(list_syntax<statement_syntax>* statements): statements(statements)
{
    push_back_child(statements);
    
    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

block_statement::~block_statement()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void block_statement::emit_code()
{
    next_list = statements->back()->next_list;

    for (auto statement : *statements)
    {
        break_list = codebuf.merge(break_list, statement->break_list);
        continue_list = codebuf.merge(continue_list, statement->continue_list);
    }
}
