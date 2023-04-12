#include "generic_syntax.hpp"
#include "../output.hpp"
#include "../symbol/symbol.hpp"
#include "../symbol/symbol_table.hpp"
#include <sstream>

using std::string;
using std::stringstream;

static symbol_table& sym_tab = symbol_table::instance();
static code_buffer& code_buf = code_buffer::instance();

type_syntax::type_syntax(syntax_token* type_token): type_token(type_token), kind(types::parse(type_token->text))
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

void type_syntax::analyze() const
{
}

void type_syntax::emit()
{
}

type_syntax::~type_syntax()
{
    for (syntax_base* child : children())
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

    if (sym_tab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }

    add_child(type);
}

parameter_syntax::~parameter_syntax()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete identifier_token;
}

void parameter_syntax::analyze() const
{
}

void parameter_syntax::emit()
{

}

function_declaration_syntax::function_declaration_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters, list_syntax<statement_syntax>* body):
    return_type(return_type), identifier_token(identifier_token), identifier(identifier_token->text), parameters(parameters), body(body)
{
    analyze();
    add_children({ return_type, parameters, body });
}

function_declaration_syntax::~function_declaration_syntax()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete identifier_token;
}

void function_declaration_syntax::analyze() const
{
    const function_symbol* function = static_cast<const function_symbol*>(sym_tab.get_symbol(identifier, symbol_kind::Function));

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
}

void function_declaration_syntax::emit()
{
    parameters->emit();

    stringstream instr;

    string ret_type = ir_builder::get_ir_type(this->return_type->kind);

    instr << ir_builder::format_string("define %s @%s (", ret_type, this->identifier);

    for (auto param = parameters->begin(); param != parameters->end(); param++)
    {
        instr << ir_builder::get_ir_type((*param)->type->kind);

        if (std::distance(param, parameters->end()) > 1)
        {
            instr << " , ";
        }
    }

    instr << ")\n{";

    code_buf.emit(instr.str());

    code_buf.increase_indent();

    body->emit();

    if (this->identifier == "main")
    {
        code_buf.emit("call void @exit(i32 0)");
    }

    if (this->return_type->kind == type_kind::Void)
    {
        code_buf.emit("ret void");
    }
    else
    {
        code_buf.emit("ret %s 0", ret_type);
    }

    code_buf.decrease_indent();

    code_buf.emit("}\n");
}

root_syntax::root_syntax(list_syntax<function_declaration_syntax>* functions): functions(functions)
{
    analyze();
    add_child(functions);
}

root_syntax::~root_syntax()
{
    for (syntax_base* child : children())
    {
        delete child;
    }
}

void root_syntax::analyze() const
{
    const function_symbol* main = static_cast<const function_symbol*>(sym_tab.get_symbol("main", symbol_kind::Function));

    if (main == nullptr)
    {
        output::error_main_missing();
    }

    if (main->type != type_kind::Void || main->parameter_types.size() != 0)
    {
        output::error_main_missing();
    }
}

void root_syntax::emit()
{
    functions->emit();
}
