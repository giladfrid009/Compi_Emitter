#include "generic_syntax.hpp"
#include "../errors.hpp"
#include "../symbol/symbol.hpp"
#include "../symbol/symbol_table.hpp"
#include <sstream>
#include <vector>

using std::string;
using std::stringstream;
using std::vector;

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
    delete type_token;
}

parameter_syntax::parameter_syntax(type_syntax* type, syntax_token* identifier_token):
    type(type), identifier_token(identifier_token), identifier(identifier_token->text)
{
    analyze();
    add_child(type);
}

parameter_syntax::~parameter_syntax()
{
    delete identifier_token;
}

void parameter_syntax::analyze() const
{
    if (type->kind == type_kind::Void)
    {
        output::error_mismatch(identifier_token->position);
    }

    if (sym_tab.contains_symbol(identifier))
    {
        output::error_def(identifier_token->position, identifier);
    }
}

void parameter_syntax::emit()
{
}

function_header_syntax::function_header_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters):
    return_type(return_type), identifier_token(identifier_token), identifier(identifier_token->text), parameters(parameters)
{
    analyze();

    vector<type_kind> param_types;

    for (auto param : *parameters)
    {
        param_types.push_back(param->type->kind);
    }

    sym_tab.add_function(identifier_token->text, return_type->kind, param_types);

    sym_tab.open_scope();

    for (auto param : *parameters)
    {
        sym_tab.add_parameter(param->identifier, param->type->kind);
    }

    add_children({ return_type, parameters });
}

function_header_syntax::~function_header_syntax()
{
    delete identifier_token;
}

void function_header_syntax::analyze() const
{
    string func_name = identifier_token->text;

    if (sym_tab.contains_symbol(func_name))
    {
        output::error_def(identifier_token->position, func_name);
    }
}

void function_header_syntax::emit()
{
    parameters->emit();

    stringstream header_text;

    string ret_type = ir_builder::get_ir_type(this->return_type->kind);

    header_text << ir_builder::format_string("define %s @%s (", ret_type, this->identifier);

    for (auto param = parameters->begin(); param != parameters->end(); param++)
    {
        header_text << ir_builder::get_ir_type((*param)->type->kind);

        if (std::distance(param, parameters->end()) > 1)
        {
            header_text << " , ";
        }
    }

    header_text << ")";

    code_buf.emit(header_text.str());
}

function_declaration_syntax::function_declaration_syntax(function_header_syntax* header, list_syntax<statement_syntax>* body):
    header(header), body(body)
{
    analyze();
    add_children({ header, body });
}

void function_declaration_syntax::analyze() const
{
}

void function_declaration_syntax::emit()
{
    header->emit();

    code_buf.emit("{");

    code_buf.increase_indent();

    body->emit();

    if (header->identifier == "main")
    {
        code_buf.emit("call void @exit(i32 0)");
    }

    if (header->return_type->kind == type_kind::Void)
    {
        code_buf.emit("ret void");
    }
    else
    {
        code_buf.emit("ret %s 0", ir_builder::get_ir_type(header->return_type->kind));
    }

    code_buf.decrease_indent();

    code_buf.emit("}\n");
}

root_syntax::root_syntax(list_syntax<function_declaration_syntax>* functions): functions(functions)
{
    analyze();
    add_child(functions);
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
