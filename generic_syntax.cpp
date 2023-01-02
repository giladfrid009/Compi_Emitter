#include "generic_syntax.hpp"
#include "output.hpp"
#include "symbol.hpp"
#include "symbol_table.hpp"

using std::vector;
using std::string;

static code_buffer& codebuf = code_buffer::instance();

type_syntax::type_syntax(syntax_token* type_token):
    type_token(type_token), kind(types::parse(type_token->text))
{
}

bool type_syntax::is_numeric() const
{
    return types::is_numeric(kind);
}

bool type_syntax::is_special() const
{
    return types::is_special(kind);
}

void type_syntax::emit_node()
{
    //todo: probably shouldn't emit anything
}

type_syntax::~type_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete type_token;
}

parameter_syntax::parameter_syntax(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text)
{
    if (type->kind == type_kind::Void)
    {
        output::error_mismatch(identifier_token->position);
    }

    if (symbol_table::instance().contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    push_back_child(type);
}

parameter_syntax::~parameter_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void parameter_syntax::emit_node()
{
}

function_declaration_syntax::function_declaration_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters, list_syntax<statement_syntax>* body):
    return_type(return_type), identifier_token(identifier_token), identifier(identifier_token->text), parameters(parameters), body(body)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Function)
    {
        throw std::logic_error("function should be defined.");
    }

    const function_symbol* func_symbol = static_cast<const function_symbol*>(symbol);

    if (func_symbol->parameter_types.size() != parameters->size())
    {
        throw std::logic_error("function parameter length mismatch.");
    }

    size_t i = 0;
    for (auto param : *parameters)
    {
        if (func_symbol->parameter_types[i++] != param->type->kind)
        {
            throw std::logic_error("function parameter type mismatch.");
        }
    }

    push_back_child(return_type);
    push_back_child(parameters);
    push_back_child(body);
}

function_declaration_syntax::~function_declaration_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void function_declaration_syntax::emit_node()
{
}

root_syntax::root_syntax(list_syntax<function_declaration_syntax>* functions): functions(functions)
{
    const symbol* main_sym = symbol_table::instance().get_symbol("main");

    if (main_sym == nullptr || main_sym->kind != symbol_kind::Function)
    {
        output::error_main_missing();
    }

    const function_symbol* func_sym = static_cast<const function_symbol*>(main_sym);

    if (func_sym->type != type_kind::Void || func_sym->parameter_types.size() != 0)
    {
        output::error_main_missing();
    }

    push_back_child(functions);
}

root_syntax::~root_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void root_syntax::emit_node()
{
}

void root_syntax::emit_tree()
{
    syntax_base::emit_tree();
}

label_syntax::label_syntax() : name(ir_builder::fresh_label())
{
}

label_syntax::~label_syntax()
{
}

void label_syntax::emit_node()
{
    codebuf.emit("%s:", name);
}
