#include "expression_syntax.hpp"
#include "symbol_table.hpp"
#include "output.hpp"
#include "bp.hpp"
#include <stdexcept>

using std::string;
using std::vector;

static code_buffer& codebuf = code_buffer::instance();

cast_expression::cast_expression(type_syntax* destination_type, expression_syntax* expression):
    expression_syntax(destination_type->kind), destination_type(destination_type), expression(expression)
{
    if (expression->is_numeric() == false || destination_type->is_numeric() == false)
    {
        output::error_mismatch(destination_type->type_token->position);
    }

    push_back_child(destination_type);
    push_back_child(expression);
}

cast_expression::~cast_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void cast_expression::emit()
{
    expression->emit();

    if (expression->return_type == type_kind::Int && destination_type->kind == type_kind::Byte)
    {
        codebuf.emit("%s = and i32 255 , %s", this->place, expression->place);
    }
    else
    {
        codebuf.emit("%s = add i32 0 , %s", this->place, expression->place);
    }
}

not_expression::not_expression(syntax_token* not_token, expression_syntax* expression):
    expression_syntax(type_kind::Bool), not_token(not_token), expression(expression)
{
    if (expression->return_type != type_kind::Bool)
    {
        output::error_mismatch(not_token->position);
    }

    push_back_child(expression);
}

not_expression::~not_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete not_token;
}

void not_expression::emit()
{
}

logical_expression::logical_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->return_type != type_kind::Bool || right->return_type != type_kind::Bool)
    {
        output::error_mismatch(oper_token->position);
    }

    push_back_child(left);
    push_back_child(right);
}

logical_expression::~logical_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete oper_token;
}

logical_expression::operator_kind logical_expression::parse_operator(string str)
{
    if (str == "and") return operator_kind::And;
    if (str == "or") return operator_kind::Or;

    throw std::invalid_argument("unknown oper");
}

void logical_expression::emit()
{
}

arithmetic_expression::arithmetic_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(types::cast_up(left->return_type, right->return_type)), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }

    push_back_child(left);
    push_back_child(right);
}

arithmetic_expression::~arithmetic_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete oper_token;
}

arithmetic_expression::operator_kind arithmetic_expression::parse_operator(string str)
{
    if (str == "+") return operator_kind::Add;
    if (str == "-") return operator_kind::Sub;
    if (str == "*") return operator_kind::Mul;
    if (str == "/") return operator_kind::Div;

    throw std::invalid_argument("unknown oper");
}

string arithmetic_expression::ir_operator() const
{
    switch (oper)
    {
        case operator_kind::Add: return "add";
        case operator_kind::Sub: return "sub";
        case operator_kind::Mul: return "mul";
        case operator_kind::Div: return return_type == type_kind::Byte ? "udiv" : "sdiv";
    }

    throw std::runtime_error("unknown oper");
}

void arithmetic_expression::emit()
{
    left->emit();
    right->emit();

    if (oper == operator_kind::Div)
    {
        string cmp_res = codebuf.register_name();
        string true_label = codebuf.label_name();
        string false_label = codebuf.label_name();

        codebuf.emit("%s = icmp eq i32 0 , %s", cmp_res, right->place);

        codebuf.emit("br i1 %s , label %s , label %s", cmp_res, true_label, false_label);

        codebuf.emit("%s:", true_label);

        codebuf.emit("call void @error_zero_div()");

        codebuf.emit("br label %s", false_label);

        codebuf.emit("%s:", false_label);
    }

    if (return_type == type_kind::Byte)
    {
        string res_reg = codebuf.register_name();

        codebuf.emit("%s = %s i32 %s , %s", res_reg, ir_operator(), left->place, right->place);

        codebuf.emit("%s = and i32 255 , %s", this->place, res_reg);
    }
    else if (return_type == type_kind::Int)
    {
        codebuf.emit("%s = %s i32 %s , %s", this->place, ir_operator(), left->place, right->place);
    }
}

relational_expression::relational_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }

    push_back_child(left);
    push_back_child(right);
}

relational_expression::~relational_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete oper_token;
}

relational_expression::operator_kind relational_expression::parse_operator(string str)
{
    if (str == "<") return operator_kind::Less;
    if (str == "<=") return operator_kind::LessEqual;
    if (str == ">") return operator_kind::Greater;
    if (str == ">=") return operator_kind::GreaterEqual;
    if (str == "==") return operator_kind::Equal;
    if (str == "!=") return operator_kind::NotEqual;

    throw std::invalid_argument("unknown oper");
}

string relational_expression::ir_operator() const
{
    switch (oper)
    {
        case operator_kind::Equal: return "add";
        case operator_kind::NotEqual: return "ne";
        case operator_kind::Greater: return return_type == type_kind::Byte ? "ugt" : "sgt";
        case operator_kind::GreaterEqual: return return_type == type_kind::Byte ? "uge" : "sge";
        case operator_kind::Less: return return_type == type_kind::Byte ? "ult" : "slt";
        case operator_kind::LessEqual: return return_type == type_kind::Byte ? "ule" : "sle";
    }

    throw std::runtime_error("unknown oper");
}

void relational_expression::emit()
{
    left->emit();
    right->emit();

    string res_reg = codebuf.register_name();

    codebuf.emit("%s = icmp %s i32 %s , %s", res_reg, ir_operator(), left->place, right->place);

    codebuf.emit("%s = zext i32 %s", this->place, res_reg);
}

conditional_expression::conditional_expression(expression_syntax* true_value, syntax_token* if_token, expression_syntax* condition, syntax_token* const else_token, expression_syntax* false_value):
    expression_syntax(types::cast_up(true_value->return_type, false_value->return_type)), true_value(true_value), if_token(if_token), condition(condition), else_token(else_token), false_value(false_value)
{
    if (return_type == type_kind::Void)
    {
        output::error_mismatch(if_token->position);
    }

    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }

    push_back_child(true_value);
    push_back_child(condition);
    push_back_child(false_value);
}

conditional_expression::~conditional_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete if_token;
    delete else_token;
}

void conditional_expression::emit()
{
    condition->emit();

    string true_label = codebuf.label_name();
    string false_label = codebuf.label_name();
    string exit_label = codebuf.label_name();

    // todo: codebuf.backpatch(condition->true_list, true_tabel) ?
    // codebuf.backpatch(condition->false_list, false_tabel) ?

    codebuf.emit("%s:", true_label);

    true_value->emit();

    codebuf.emit("br label %s", exit_label);

    codebuf.emit("%s:", false_label);

    false_value->emit();

    codebuf.emit("br label %s", exit_label);

    codebuf.emit("%s:", exit_label);

    string res_type = return_type == type_kind::String ? "i8*" : "i32";

    codebuf.emit("%s = phi %s [ %s , %s ] , [ %s , %s ]", this->place, res_type, true_value->place, true_label, false_value->place, false_label);
}

identifier_expression::identifier_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Variable)
    {
        output::error_undef(identifier_token->position, identifier);
    }
}

type_kind identifier_expression::get_return_type(string identifier)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Variable)
    {
        return type_kind::Invalid;
    }

    return symbol->type;
}

identifier_expression::~identifier_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void identifier_expression::emit()
{
    // todo: get identifier address
    // load identifier from address, can only be numeric or bool
}

invocation_expression::invocation_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(nullptr)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Function)
    {
        output::error_undef_func(identifier_token->position, identifier);
    }

    vector<type_kind> parameter_types = static_cast<const function_symbol*>(symbol)->parameter_types;

    vector<string> params_str;

    for (type_kind type : parameter_types)
    {
        params_str.push_back(types::to_string(type));
    }

    if (parameter_types.size() != 0)
    {
        output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
    }
}

invocation_expression::invocation_expression(syntax_token* identifier_token, list_syntax<expression_syntax>* arguments):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(arguments)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Function)
    {
        output::error_undef_func(identifier_token->position, identifier);
    }

    vector<type_kind> parameter_types = static_cast<const function_symbol*>(symbol)->parameter_types;

    vector<string> params_str;

    for (type_kind type : parameter_types)
    {
        params_str.push_back(types::to_string(type));
    }

    if (parameter_types.size() != arguments->size())
    {
        output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
    }

    size_t i = 0;
    for (auto arg : *arguments)
    {
        if (types::is_implictly_convertible(arg->return_type, parameter_types[i++]) == false)
        {
            output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
        }
    }

    push_back_child(arguments);
}

type_kind invocation_expression::get_return_type(string identifier)
{
    const symbol* symbol = symbol_table::instance().get_symbol(identifier);

    if (symbol == nullptr || symbol->kind != symbol_kind::Function)
    {
        return type_kind::Invalid;
    }

    return symbol->type;
}

invocation_expression::~invocation_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

void invocation_expression::emit()
{
    arguments->emit();
}
