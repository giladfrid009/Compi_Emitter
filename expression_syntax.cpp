#include "expression_syntax.hpp"
#include "symbol_table.hpp"
#include "hw3_output.hpp"
#include "code_buffer.hpp"
#include "ir_builder.hpp"
#include <stdexcept>
#include <list>
#include <sstream>
#include <cassert>
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
    if (value->is_numeric() == false || destination_type->is_numeric() == false)
    {
        output::error_mismatch(destination_type->type_token->position);
    }

    add_child(destination_type);
    add_child(value);

    emit();
}

cast_expression::~cast_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void cast_expression::emit_node()
{
    true_list = value->true_list;
    false_list = value->false_list;
    jump_list = value->jump_list;
    jump_label = value->jump_label;

    if (value->return_type == type_kind::Int && destination_type->kind == type_kind::Byte)
    {
        codebuf.emit("%s = and i32 255 , %s", this->place, value->place);
    }
    else
    {
        codebuf.emit("%s = add i32 0 , %s", this->place, value->place);
    }
}

not_expression::not_expression(syntax_token* not_token, expression_syntax* expression):
    expression_syntax(type_kind::Bool), not_token(not_token), expression(expression)
{
    if (expression->return_type != type_kind::Bool)
    {
        output::error_mismatch(not_token->position);
    }

    add_child(expression);

    emit();
}

not_expression::~not_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete not_token;
}

void not_expression::emit_node()
{
    jump_list = expression->jump_list;
    jump_label = expression->jump_label;
    false_list = expression->true_list;
    true_list = expression->false_list;
}

logical_expression::logical_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->return_type != type_kind::Bool || right->return_type != type_kind::Bool)
    {
        output::error_mismatch(oper_token->position);
    }

    add_child(left);
    add_child(right);

    emit();
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

void logical_expression::emit_node()
{
    jump_list = left->jump_list;
    jump_label = left->jump_label;

    codebuf.backpatch(right->jump_list, right->jump_label);

    if (oper == operator_kind::Or)
    {
        codebuf.backpatch(left->false_list, right->jump_label);

        true_list = codebuf.merge(left->true_list, right->true_list);

        false_list = right->false_list;
    }
    else
    {
        codebuf.backpatch(left->true_list, right->jump_label);

        true_list = right->true_list;

        false_list = codebuf.merge(left->false_list, right->false_list);
    }
}

arithmetic_expression::arithmetic_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(types::cast_up(left->return_type, right->return_type)), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }

    add_child(left);
    add_child(right);

    emit();
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

void arithmetic_expression::emit_node()
{
    jump_list = left->jump_list;
    jump_label = left->jump_label;

    codebuf.backpatch(right->jump_list, right->jump_label);

    if (oper == arithmetic_operator::Div)
    {
        string cmp_res = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();

        codebuf.emit("%s = icmp eq i32 0 , %s", cmp_res, right->place);
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

        codebuf.emit("%s = %s i32 %s , %s", res_reg, inst, left->place, right->place);
        codebuf.emit("%s = and i32 255 , %s", this->place, res_reg);
    }
    else if (return_type == type_kind::Int)
    {
        codebuf.emit("%s = %s i32 %s , %s", this->place, inst, left->place, right->place);
    }
}

relational_expression::relational_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }

    add_child(left);
    add_child(right);

    emit();
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

void relational_expression::emit_node()
{
    jump_list = left->jump_list;
    jump_label = left->jump_label;

    codebuf.backpatch(right->jump_list, right->jump_label);

    string res_reg = ir_builder::fresh_register();
    string cmp_kind = ir_builder::get_comparison_kind(oper, return_type == type_kind::Int);

    codebuf.emit("%s = icmp %s i32 %s , %s", res_reg, cmp_kind, left->place, right->place);
    size_t line = codebuf.emit("br i1 %s , label @ , label @", res_reg);

    true_list.push_back(patch_record(line, label_index::First));
    false_list.push_back(patch_record(line, label_index::Second));
}

conditional_expression::conditional_expression(expression_syntax* true_value, syntax_token* if_token, expression_syntax* condition, syntax_token* else_token, expression_syntax* false_value):
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

    add_child(true_value);
    add_child(condition);
    add_child(false_value);

    emit();
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

void conditional_expression::emit_node() //todo: make sure is correct
{
    string start_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    codebuf.backpatch(true_value->jump_list, start_label);
    codebuf.backpatch(condition->jump_list, end_label);
    codebuf.backpatch(condition->true_list, true_value->jump_label);
    codebuf.backpatch(condition->false_list, false_value->jump_label);
    codebuf.backpatch(false_value->jump_list, end_label);

    codebuf.emit("br label %%%s", end_label);
    codebuf.emit("%s:", start_label);
    size_t line = codebuf.emit("br label @");
    codebuf.emit("br label %%%s", condition->jump_label);
    codebuf.emit("%s:", end_label);

    jump_list.push_back(patch_record(line, label_index::First));

    if (return_type == type_kind::Bool)
    {
        true_list = codebuf.merge(true_value->true_list, false_value->true_list);
        false_list = codebuf.merge(true_value->false_list, false_value->false_list);
    }
    else
    {
        string res_type = ir_builder::get_type(return_type);

        codebuf.emit("%s = phi %s [ %s , %s ] , [ %s , %s ]", place, res_type, true_value->place, true_value->jump_label, false_value->place, false_value->jump_label);
    }
}

identifier_expression::identifier_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text)
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

    emit();
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

void identifier_expression::emit_node()
{
    size_t line = codebuf.emit("br label @");
    jump_label = codebuf.emit_label();
    
    jump_list.push_back(patch_record(line, label_index::First));

    const symbol* symbol = symtab.get_symbol(identifier);

    if (symbol->kind == symbol_kind::Parameter)
    {
        string param_reg = ir_builder::format_string("%%%d", -symbol->offset - 1);

        codebuf.emit("%s = add i32 0 , %s", this->place, param_reg);
    }
    else if (symbol->kind == symbol_kind::Variable)
    {
        string ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;
        string res_type = ir_builder::get_type(return_type);

        codebuf.emit("%s = load %s , %s* %s", this->place, res_type, res_type, ptr_reg);
    }

    if (return_type == type_kind::Bool)
    {
        string bool_reg = ir_builder::fresh_register();

        codebuf.emit("%s = trunc i32 %s to i1", bool_reg, this->place);
        size_t line = codebuf.emit("br i1 %s , label @ , label @", bool_reg);

        true_list.push_back(patch_record(line, label_index::First));
        false_list.push_back(patch_record(line, label_index::Second));
    }
}

invocation_expression::invocation_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(nullptr)
{
    const function_symbol* symbol = static_cast<const function_symbol*>(symtab.get_symbol(identifier, symbol_kind::Function));

    if (symbol == nullptr)
    {
        output::error_undef_func(identifier_token->position, identifier);
    }

    vector<type_kind> parameter_types = symbol->parameter_types;

    vector<string> params_str;

    for (type_kind type : parameter_types)
    {
        params_str.push_back(types::to_string(type));
    }

    if (parameter_types.size() != 0)
    {
        output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
    }

    emit();
}

invocation_expression::invocation_expression(syntax_token* identifier_token, list_syntax<expression_syntax>* arguments):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(arguments)
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

    add_child(arguments);

    emit();
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

        string arg_reg = arg->return_type != type_kind::Bool ? arg->place : get_bool_reg(arg);

        result << arg_type << " " << arg_reg;

        if (std::distance(iter, arguments->end()) > 1)
        {
            result << " , ";
        }
    }

    return result.str();
}

void invocation_expression::emit_node()
{
    if (arguments == nullptr)
    {
        size_t line = codebuf.emit("br label @");
        jump_label = codebuf.emit_label();

        jump_list.push_back(patch_record(line, label_index::First));
    }
    else
    {
        for (auto arg : *arguments)
        {
            if (arg == arguments->front())
            {
                jump_list = arg->jump_list;
                jump_label = arg->jump_label;  
            }
            else
            {
                codebuf.backpatch(arg->jump_list, arg->jump_label);
            }
        }
    }

    if (return_type == type_kind::Void)
    {
        codebuf.emit("call void @%s(%s)", identifier, get_arguments(arguments));
    }
    else
    {
        string ret_str = ir_builder::get_type(return_type);

        codebuf.emit("%s = call %s @%s(%s)", this->place, ret_str, identifier, get_arguments(arguments));
    }
}
