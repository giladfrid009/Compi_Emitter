#include "generic_syntax.hpp"
#include "hw3_output.hpp"
#include "symbol.hpp"
#include "symbol_table.hpp"

using std::string;

static symbol_table& symtab = symbol_table::instance();
static code_buffer& codebuf = code_buffer::instance();

type_syntax::type_syntax(syntax_token* type_token): type_token(type_token), kind(types::parse(type_token->text))
{
    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

bool type_syntax::is_numeric() const
{
    return types::is_numeric(kind);
}

bool type_syntax::is_special() const
{
    return types::is_special(kind);
}

void type_syntax::emit_code()
{
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

    if (symtab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    push_back_child(type);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

parameter_syntax::~parameter_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void parameter_syntax::emit_code()
{
}

function_declaration_syntax::function_declaration_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters, list_syntax<statement_syntax>* body):
    return_type(return_type), identifier_token(identifier_token), identifier(identifier_token->text), parameters(parameters), body(body)
{
    const function_symbol* function = static_cast<const function_symbol*>(symtab.get_symbol(identifier, symbol_kind::Function));

    if (function == nullptr)
    {
        throw std::logic_error("function should be defined.");
    }

    if (function->parameter_types.size() != parameters->size())
    {
        throw std::logic_error("function parameter length mismatch.");
    }

    size_t i = 0;
    for (auto param : *parameters)
    {
        if (function->parameter_types[i++] != param->type->kind)
        {
            throw std::logic_error("function parameter type mismatch.");
        }
    }

    push_back_child(return_type);
    push_back_child(parameters);
    push_back_child(body);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

function_declaration_syntax::~function_declaration_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void function_declaration_syntax::emit_code()
{
}

root_syntax::root_syntax(list_syntax<function_declaration_syntax>* functions): functions(functions)
{
    const function_symbol* main = static_cast<const function_symbol*>(symtab.get_symbol("main", symbol_kind::Function));

    if (main == nullptr)
    {
        output::error_main_missing();
    }

    if (main->type != type_kind::Void || main->parameter_types.size() != 0)
    {
        output::error_main_missing();
    }

    push_back_child(functions);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

root_syntax::~root_syntax()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void root_syntax::emit_code()
{
}
