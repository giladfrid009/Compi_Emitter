#include "expression_syntax.hpp"
#include "symbol_table.hpp"
#include "output.hpp"
#include "code_buffer.hpp"
#include "ir_builder.hpp"
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

void cast_expression::emit_node()
{
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

void not_expression::emit_node()
{
    false_list = expression->true_list;
    true_list = expression->false_list;

    expression->true_list.clear();
    expression->false_list.clear();
}

logical_expression::logical_expression(expression_syntax* left, syntax_token* oper_token, label_syntax* label, expression_syntax* right):
    expression_syntax(type_kind::Bool), left(left), oper_token(oper_token), label(label), right(right), oper(parse_operator(oper_token->text))
{
    if (left->return_type != type_kind::Bool || right->return_type != type_kind::Bool)
    {
        output::error_mismatch(oper_token->position);
    }

    push_back_child(left);
    push_back_child(label);
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

void logical_expression::emit_node()
{    
    if(oper == operator_kind::Or)
    {
        codebuf.backpatch(left->false_list, label->name);
        
        true_list = codebuf.merge(left->true_list, right->true_list);
        false_list = right->false_list;
    }
    else
    {
        codebuf.backpatch(left->true_list, label->name);

        true_list = right->true_list;
        false_list = codebuf.merge(left->false_list, right->false_list);
    }

    left->true_list.clear();
    left->false_list.clear();
    right->true_list.clear();
    right->false_list.clear();
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
    if (oper == arithmetic_operator::Div)
    {
        string cmp_res = ir_builder::fresh_register();
        string true_label = ir_builder::fresh_label();
        string false_label = ir_builder::fresh_label();

        codebuf.emit("%s = icmp eq i32 0 , %s", cmp_res, right->place);

        codebuf.emit("br i1 %s , label %s , label %s", cmp_res, true_label, false_label);

        codebuf.emit("%s:", true_label);

        codebuf.emit("call void @error_zero_div()");

        codebuf.emit("br label %s", false_label);

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
    string res_reg = ir_builder::fresh_register();

    string cmp_kind = ir_builder::get_comparison_kind(oper, return_type == type_kind::Int);

    codebuf.emit("%s = icmp %s i32 %s , %s", res_reg, cmp_kind, left->place, right->place);

    size_t line = codebuf.emit("br i1 eq 0 , %s label @ , label @", res_reg);

    true_list.push_back(patch_record(line, label_index::first));
    false_list.push_back(patch_record(line, label_index::second));
}

conditional_expression::conditional_expression(
    jump_syntax* condition_jump, 
    label_syntax* true_label, 
    expression_syntax* true_value, 
    jump_syntax* true_next, 
    syntax_token* if_token, 
    label_syntax* condition_label, 
    expression_syntax* condition, 
    syntax_token* else_token, 
    label_syntax* false_label, 
    expression_syntax* false_value, 
    jump_syntax* false_next) :
    
    expression_syntax(types::cast_up(true_value->return_type, false_value->return_type)), 
    condition_jump(condition_jump),
    true_label(true_label),
    true_value(true_value),
    true_next(true_next), 
    if_token(if_token), 
    condition_label(condition_label),
    condition(condition), 
    else_token(else_token), 
    false_label(false_label),
    false_value(false_value),
    false_next(false_next)
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

void conditional_expression::emit_node()
{
    string next_label = codebuf.emit_label();

    codebuf.backpatch(condition_jump->next_list, condition_label->name);

    codebuf.backpatch(condition->true_list, true_label->name);
    codebuf.backpatch(condition->false_list, false_label->name);
    
    codebuf.backpatch(true_next->next_list, next_label);
    codebuf.backpatch(false_next->next_list, next_label);

    codebuf.emit("%s:", next_label);

    //todo: correct only if return_type != Bool
    codebuf.emit("%s = phi i32 [ %s , %s ] [ %s , %s ]", this->place, true_value->place, true_label->name, false_value->place, false_label->name);

    condition_jump->next_list.clear();
    condition->true_list.clear();
    condition->false_list.clear();
    true_next->next_list.clear();
    false_next->next_list.clear();
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

void identifier_expression::emit_node()
{
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

void invocation_expression::emit_node()
{
}
