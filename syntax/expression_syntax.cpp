#include "expression_syntax.hpp"
#include "../symbol/symbol_table.hpp"
#include "../output.hpp"
#include "../emit/code_buffer.hpp"
#include "../symbol/symbol.hpp"
#include "../emit/ir_builder.hpp"
#include <stdexcept>
#include <list>
#include <sstream>
#include <iterator>

using std::string;
using std::vector;
using std::list;
using std::stringstream;

static symbol_table& symtab = symbol_table::instance();
static code_buffer& codebuf = code_buffer::instance();

cast_expression::cast_expression(type_syntax* destination_type, expression_syntax* value):
    expression_syntax(destination_type->kind), destination_type(destination_type), value(value)
{
    semantic_analysis();

    add_child(destination_type);
    add_child(value);
}

cast_expression::~cast_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void cast_expression::semantic_analysis() const
{
    if (value->is_numeric() == false || destination_type->is_numeric() == false)
    {
        output::error_mismatch(destination_type->type_token->position);
    }
}

void cast_expression::emit_code()
{
    value->emit_code();

    if (value->return_type == type_kind::Int && destination_type->kind == type_kind::Byte)
    {
        codebuf.emit("%s = and i32 255 , %s", this->reg, value->reg);
    }
    else
    {
        codebuf.emit("%s = add i32 0 , %s", this->reg, value->reg);
    }
}

not_expression::not_expression(syntax_token* not_token, expression_syntax* expression):
    expression_syntax(type_kind::Bool), not_token(not_token), expression(expression)
{
    semantic_analysis();

    add_child(expression);
}

not_expression::~not_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete not_token;
}

void not_expression::semantic_analysis() const
{
    if (expression->return_type != type_kind::Bool)
    {
        output::error_mismatch(not_token->position);
    }
}

void not_expression::emit_code()
{
    expression->emit_code();

    codebuf.emit("%s = select i1 %s , i1 0 , i1 1", this->reg, expression->reg);
}

logical_expression::logical_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    semantic_analysis();

    add_child(left);
    add_child(right);
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

void logical_expression::semantic_analysis() const
{
    if (left->return_type != type_kind::Bool || right->return_type != type_kind::Bool)
    {
        output::error_mismatch(oper_token->position);
    }
}

void logical_expression::emit_code()
{
    left->emit_code();

    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string assign_label = ir_builder::fresh_label();
    
    if (oper == operator_kind::Or)
    {
        codebuf.emit("br i1 %s , label %%%s , label %%%s", left->reg, assign_label, false_label);
        codebuf.emit("%s:", false_label);
        right->emit_code();
        codebuf.emit("br label %%%s", assign_label);
        codebuf.emit("%s:", assign_label);
        codebuf.emit("%s = select i1 %s , i1 1 , i1 %s", this->reg, left->reg, right->reg);
    }
    else if (oper == operator_kind::And)
    {
        codebuf.emit("br i1 %s , label %%%s , label %%%s", left->reg, true_label, assign_label);
        codebuf.emit("%s:", true_label);
        right->emit_code();
        codebuf.emit("br label %%%s", assign_label);
        codebuf.emit("%s:", assign_label);
        codebuf.emit("%s = select i1 %s , i1 %s , i1 0", this->reg, left->reg, right->reg);
    }
}

arithmetic_expression::arithmetic_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(types::cast_up(left->return_type, right->return_type)), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    semantic_analysis();

    add_child(left);
    add_child(right);
}

arithmetic_expression::~arithmetic_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete oper_token;
}

arithmetic_operator arithmetic_expression::parse_operator(string str)
{
    if (str == "+") return arithmetic_operator::Add;
    if (str == "-") return arithmetic_operator::Sub;
    if (str == "*") return arithmetic_operator::Mul;
    if (str == "/") return arithmetic_operator::Div;

    throw std::invalid_argument("unknown oper");
}

void arithmetic_expression::semantic_analysis() const
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }
}

void arithmetic_expression::emit_code()
{
    left->emit_code();
    right->emit_code();

    if (oper == arithmetic_operator::Div)
    {
        string cmp_res = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();

        codebuf.emit("%s = icmp eq i32 0 , %s", cmp_res, right->reg);
        codebuf.emit("br i1 %s , label %%%s , label %%%s", cmp_res, true_label, false_label);
        codebuf.emit("%s:", true_label);
        codebuf.emit("call void @error_zero_div()");
        codebuf.emit("br label %%%s", false_label);
        codebuf.emit("%s:", false_label);
    }

    string inst = ir_builder::get_binary_instruction(oper, return_type == type_kind::Int);

    if (return_type == type_kind::Byte)
    {
        string res_reg = ir_builder::fresh_register();

        codebuf.emit("%s = %s i32 %s , %s", res_reg, inst, left->reg, right->reg);
        codebuf.emit("%s = and i32 255 , %s", this->reg, res_reg);
    }
    else if (return_type == type_kind::Int)
    {
        codebuf.emit("%s = %s i32 %s , %s", this->reg, inst, left->reg, right->reg);
    }
}

relational_expression::relational_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    semantic_analysis();

    add_child(left);
    add_child(right);
}

relational_expression::~relational_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete oper_token;
}

relational_operator relational_expression::parse_operator(string str)
{
    if (str == "<") return relational_operator::Less;
    if (str == "<=") return relational_operator::LessEqual;
    if (str == ">") return relational_operator::Greater;
    if (str == ">=") return relational_operator::GreaterEqual;
    if (str == "==") return relational_operator::Equal;
    if (str == "!=") return relational_operator::NotEqual;

    throw std::invalid_argument("unknown oper");
}

void relational_expression::semantic_analysis() const
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }
}

void relational_expression::emit_code()
{
    left->emit_code();
    right->emit_code();

    type_kind operands_type = types::cast_up(left->return_type, right->return_type);

    string cmp_kind = ir_builder::get_comparison_kind(oper, operands_type == type_kind::Int);
    
    codebuf.emit("%s = icmp %s i32 %s , %s", this->reg, cmp_kind, left->reg, right->reg);
}

conditional_expression::conditional_expression(expression_syntax* true_value, syntax_token* if_token, expression_syntax* condition, syntax_token* else_token, expression_syntax* false_value):
    expression_syntax(types::cast_up(true_value->return_type, false_value->return_type)), true_value(true_value), if_token(if_token), condition(condition), else_token(else_token), false_value(false_value)
{
    semantic_analysis();

    add_child(true_value);
    add_child(condition);
    add_child(false_value);
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

void conditional_expression::semantic_analysis() const
{
    if (return_type == type_kind::Void)
    {
        output::error_mismatch(if_token->position);
    }

    if (condition->return_type != type_kind::Bool)
    {
        output::error_mismatch(if_token->position);
    }
}

void conditional_expression::emit_code()
{
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string assign_label = ir_builder::fresh_label();

    string ret_type = ir_builder::get_type(this->return_type);

    condition->emit_code();

    codebuf.emit("br i1 %s , label %%%s, label %%%s", condition->reg, true_label, false_label);

    codebuf.emit("%s:", true_label);

    true_value->emit_code();

    codebuf.emit("br label %%%s", assign_label);

    codebuf.emit("%s:", false_label);

    false_value->emit_code();

    codebuf.emit("br label %%%s", assign_label);

    codebuf.emit("%s:", assign_label);

    codebuf.emit("%s = select i1 %s , %s %s , %s %s", this->reg, condition->reg, ret_type, true_value->reg, ret_type, false_value->reg);
}

identifier_expression::identifier_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), kind(symbol_kind::Parameter), ptr_reg()
{
    semantic_analysis();

    const symbol* symbol = symtab.get_symbol(identifier);

    kind = symbol->kind;

    if (symbol->kind == symbol_kind::Parameter)
    {
        this->ptr_reg = ir_builder::format_string("%%%d", -symbol->offset - 1);
    }
    else
    {
        this->ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;
    }
}

type_kind identifier_expression::get_return_type(string identifier)
{
    const symbol* symbol = symtab.get_symbol(identifier);

    if (symbol == nullptr)
    {
        return type_kind::Invalid;
    }

    if (symbol->kind != symbol_kind::Variable && symbol->kind != symbol_kind::Parameter)
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

void identifier_expression::semantic_analysis() const
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
}

void identifier_expression::emit_code()
{
    string res_type = ir_builder::get_type(return_type);

    if (kind == symbol_kind::Parameter)
    {
        codebuf.emit("%s = add %s 0 , %s", this->reg, res_type, ptr_reg);
    }
    else if (kind == symbol_kind::Variable)
    {   
        codebuf.emit("%s = load %s , %s* %s", this->reg, res_type, res_type, ptr_reg);
    }
}

invocation_expression::invocation_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(nullptr)
{
    semantic_analysis();
}

invocation_expression::invocation_expression(syntax_token* identifier_token, list_syntax<expression_syntax>* arguments):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(arguments)
{
    semantic_analysis();

    add_child(arguments);
}

type_kind invocation_expression::get_return_type(string identifier)
{
    const symbol* function = symtab.get_symbol(identifier, symbol_kind::Function);

    if (function == nullptr)
    {
        return type_kind::Invalid;
    }

    return function->type;
}

invocation_expression::~invocation_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete identifier_token;
}

string invocation_expression::get_arguments(const list_syntax<expression_syntax>* arguments)
{
    if (arguments == nullptr)
    {
        return "";
    }

    stringstream result;

    for (auto iter = arguments->begin(); iter != arguments->end(); iter++)
    {
        expression_syntax* arg = *iter;

        string arg_type = ir_builder::get_type(arg->return_type);

        result << arg_type << " " << arg->reg;

        if (std::distance(iter, arguments->end()) > 1)
        {
            result << " , ";
        }
    }

    return result.str();
}

void invocation_expression::semantic_analysis() const
{
    const function_symbol* function = static_cast<const function_symbol*>(symtab.get_symbol(identifier, symbol_kind::Function));

    if (function == nullptr)
    {
        output::error_undef_func(identifier_token->position, identifier);
    }

    vector<type_kind> parameter_types = function->parameter_types;

    vector<string> params_str;

    for (type_kind type : parameter_types)
    {
        params_str.push_back(types::to_string(type));
    }

    if (arguments == nullptr)
    {
        if (parameter_types.size() != 0)
        {
            output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
        }
    }
    else
    {
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
    }
}

void invocation_expression::emit_code()
{
    if (arguments != nullptr)
    {
        arguments->emit_code();
    }

    if (return_type == type_kind::Void)
    {
        codebuf.emit("call void @%s(%s)", identifier, get_arguments(arguments));
        return;
    }
    
    string ret_str = ir_builder::get_type(return_type);

    codebuf.emit("%s = call %s @%s(%s)", this->reg, ret_str, identifier, get_arguments(arguments));
}
