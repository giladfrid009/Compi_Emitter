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

static symbol_table& sym_tab = symbol_table::instance();
static code_buffer& code_buf = code_buffer::instance();

cast_expression::cast_expression(type_syntax* destination_type, expression_syntax* value):
    expression_syntax(destination_type->kind), destination_type(destination_type), value(value)
{
    analyze();
    add_children({ destination_type, value });
}

cast_expression::~cast_expression()
{
    for (syntax_base* child : children())
    {
        delete child;
    }
}

void cast_expression::analyze() const
{
    if (value->is_numeric() == false || destination_type->is_numeric() == false)
    {
        output::error_mismatch(destination_type->type_token->position);
    }
}

void cast_expression::emit()
{
    value->emit();

    if (value->return_type == type_kind::Int && destination_type->kind == type_kind::Byte)
    {
        code_buf.emit("%s = and i32 255, %s", this->reg, value->reg);
    }
    else
    {
        code_buf.emit("%s = add i32 0, %s", this->reg, value->reg);
    }
}

not_expression::not_expression(syntax_token* not_token, expression_syntax* expression):
    expression_syntax(type_kind::Bool), not_token(not_token), expression(expression)
{
    analyze();
    add_child(expression);
}

not_expression::~not_expression()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete not_token;
}

void not_expression::analyze() const
{
    if (expression->return_type != type_kind::Bool)
    {
        output::error_mismatch(not_token->position);
    }
}

void not_expression::emit()
{
    expression->emit();

    code_buf.emit("%s = select i1 %s, i1 0, i1 1", this->reg, expression->reg);
}

logical_expression::logical_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    analyze();
    add_children({ left, right });
}

logical_expression::~logical_expression()
{
    for (syntax_base* child : children())
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

void logical_expression::analyze() const
{
    if (left->return_type != type_kind::Bool || right->return_type != type_kind::Bool)
    {
        output::error_mismatch(oper_token->position);
    }
}

void logical_expression::emit()
{
    left->emit();

    string start_label = ir_builder::fresh_label();
    string right_label = ir_builder::fresh_label();
    string phi_label = ir_builder::fresh_label();
    string branch_label = ir_builder::fresh_label();

    code_buf.emit("br label %%%s", start_label);
    code_buf.emit("%s:", start_label);

    if (oper == operator_kind::Or)
    {
        code_buf.emit("br i1 %s, label %%%s, label %%%s", left->reg, phi_label, right_label);
    }
    else if (oper == operator_kind::And)
    {
        code_buf.emit("br i1 %s, label %%%s, label %%%s", left->reg, right_label, phi_label);
    }

    code_buf.emit("%s:", right_label);
    right->emit();
    code_buf.emit("br label %%%s", branch_label);
    code_buf.emit("%s:", branch_label);
    code_buf.emit("br label %%%s", phi_label);
    code_buf.emit("%s:", phi_label);
    code_buf.emit("%s = phi i1 [ %s, %%%s ], [ %s, %%%s ]", this->reg, left->reg, start_label, right->reg, branch_label);
}

arithmetic_expression::arithmetic_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(types::cast_up(left->return_type, right->return_type)), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    analyze();
    add_children({ left, right });
}

arithmetic_expression::~arithmetic_expression()
{
    for (syntax_base* child : children())
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

void arithmetic_expression::analyze() const
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }
}

void arithmetic_expression::emit()
{
    left->emit();
    right->emit();

    if (oper == arithmetic_operator::Div)
    {
        string cmp_res = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();

        code_buf.emit("%s = icmp eq i32 0, %s", cmp_res, right->reg);
        code_buf.emit("br i1 %s, label %%%s, label %%%s", cmp_res, true_label, false_label);
        code_buf.emit("%s:", true_label);
        code_buf.emit("call void @error_zero_div()");
        code_buf.emit("br label %%%s", false_label);
        code_buf.emit("%s:", false_label);
    }

    string inst = ir_builder::get_binary_instruction(oper, return_type == type_kind::Int);

    if (return_type == type_kind::Byte)
    {
        string res_reg = ir_builder::fresh_register();

        code_buf.emit("%s = %s i32 %s, %s", res_reg, inst, left->reg, right->reg);
        code_buf.emit("%s = and i32 255, %s", this->reg, res_reg);
    }
    else if (return_type == type_kind::Int)
    {
        code_buf.emit("%s = %s i32 %s, %s", this->reg, inst, left->reg, right->reg);
    }
}

relational_expression::relational_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), right(right), oper(parse_operator(oper_token->text))
{
    analyze();
    add_children({ left, right });
}

relational_expression::~relational_expression()
{
    for (syntax_base* child : children())
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

void relational_expression::analyze() const
{
    if (left->is_numeric() == false || right->is_numeric() == false)
    {
        output::error_mismatch(oper_token->position);
    }
}

void relational_expression::emit()
{
    left->emit();
    right->emit();

    type_kind operands_type = types::cast_up(left->return_type, right->return_type);

    string cmp_kind = ir_builder::get_comparison_kind(oper, operands_type == type_kind::Int);

    code_buf.emit("%s = icmp %s i32 %s, %s", this->reg, cmp_kind, left->reg, right->reg);
}

conditional_expression::conditional_expression(expression_syntax* true_value, syntax_token* if_token, expression_syntax* condition, syntax_token* else_token, expression_syntax* false_value):
    expression_syntax(types::cast_up(true_value->return_type, false_value->return_type)), true_value(true_value), if_token(if_token), condition(condition), else_token(else_token), false_value(false_value)
{
    analyze();
    add_children({ true_value, condition, false_value });
}

conditional_expression::~conditional_expression()
{
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete if_token;
    delete else_token;
}

void conditional_expression::analyze() const
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

void conditional_expression::emit()
{
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string true_branch = ir_builder::fresh_label();
    string false_branch = ir_builder::fresh_label();
    string phi_label = ir_builder::fresh_label();

    string ret_type = ir_builder::get_type(this->return_type);

    condition->emit();
    code_buf.emit("br i1 %s, label %%%s, label %%%s", condition->reg, true_label, false_label);
    code_buf.emit("%s:", true_label);
    true_value->emit();
    code_buf.emit("br label %%%s", true_branch);
    code_buf.emit("%s:", true_branch);
    code_buf.emit("br label %%%s", phi_label);
    code_buf.emit("%s:", false_label);
    false_value->emit();
    code_buf.emit("br label %%%s", false_branch);
    code_buf.emit("%s:", false_branch);
    code_buf.emit("br label %%%s", phi_label);
    code_buf.emit("%s:", phi_label);
    code_buf.emit("%s = phi %s [ %s, %%%s ], [ %s, %%%s ]", this->reg, ret_type, true_value->reg, true_branch, false_value->reg, false_branch);
}

identifier_expression::identifier_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), _kind(symbol_kind::Parameter), _ptr_reg()
{
    analyze();

    const symbol* symbol = sym_tab.get_symbol(identifier);

    _kind = symbol->kind;

    if (symbol->kind == symbol_kind::Parameter)
    {
        this->_ptr_reg = ir_builder::format_string("%%%d", -symbol->offset - 1);
    }
    else
    {
        this->_ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;
    }
}

type_kind identifier_expression::get_return_type(string identifier)
{
    const symbol* symbol = sym_tab.get_symbol(identifier);

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
    for (syntax_base* child : children())
    {
        delete child;
    }

    delete identifier_token;
}

void identifier_expression::analyze() const
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
}

void identifier_expression::emit()
{
    string res_type = ir_builder::get_type(return_type);

    if (_kind == symbol_kind::Parameter)
    {
        code_buf.emit("%s = add %s 0, %s", this->reg, res_type, _ptr_reg);
    }
    else if (_kind == symbol_kind::Variable)
    {
        code_buf.emit("%s = load %s, %s* %s", this->reg, res_type, res_type, _ptr_reg);
    }
}

invocation_expression::invocation_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(nullptr)
{
    analyze();
}

invocation_expression::invocation_expression(syntax_token* identifier_token, list_syntax<expression_syntax>* arguments):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(arguments)
{
    analyze();
    add_child(arguments);
}

type_kind invocation_expression::get_return_type(string identifier)
{
    const symbol* function = sym_tab.get_symbol(identifier, symbol_kind::Function);

    if (function == nullptr)
    {
        return type_kind::Invalid;
    }

    return function->type;
}

invocation_expression::~invocation_expression()
{
    for (syntax_base* child : children())
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

void invocation_expression::analyze() const
{
    const function_symbol* function = static_cast<const function_symbol*>(sym_tab.get_symbol(identifier, symbol_kind::Function));

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
            if (types::is_implicitly_convertible(arg->return_type, parameter_types[i++]) == false)
            {
                output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
            }
        }
    }
}

void invocation_expression::emit()
{
    if (arguments != nullptr)
    {
        arguments->emit();
    }

    if (return_type == type_kind::Void)
    {
        code_buf.emit("call void @%s(%s)", identifier, get_arguments(arguments));
        return;
    }

    string ret_str = ir_builder::get_type(return_type);

    code_buf.emit("%s = call %s @%s(%s)", this->reg, ret_str, identifier, get_arguments(arguments));
}
