#include "expression_syntax.hpp"
#include "symbol_table.hpp"
#include "hw3_output.hpp"
#include "code_buffer.hpp"
#include "ir_builder.hpp"
#include <stdexcept>
#include <list>
#include <sstream>

using std::string;
using std::vector;
using std::list;
using std::stringstream;

static symbol_table& symtab = symbol_table::instance();
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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

cast_expression::~cast_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }
}

void cast_expression::emit_code()
{
    codebuf.backpatch(expression->jump_list, expression->label);

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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
}

not_expression::~not_expression()
{
    for (syntax_base* child : get_children())
    {
        delete child;
    }

    delete not_token;
}

void not_expression::emit_code()
{
    codebuf.backpatch(expression->jump_list, expression->label);

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

    push_back_child(left);
    push_back_child(right);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void logical_expression::emit_code()
{
    codebuf.backpatch(left->jump_list, left->label);
    codebuf.backpatch(right->jump_list, right->label);

    if (oper == operator_kind::Or)
    {
        codebuf.backpatch(left->false_list, right->label);

        true_list = codebuf.merge(left->true_list, right->true_list);
        false_list = right->false_list;
    }
    else
    {
        codebuf.backpatch(left->true_list, right->label);

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

    push_back_child(left);
    push_back_child(right);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void arithmetic_expression::emit_code()
{
    codebuf.backpatch(left->jump_list, left->label);
    codebuf.backpatch(right->jump_list, right->label);

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

    push_back_child(left);
    push_back_child(right);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void relational_expression::emit_code()
{
    codebuf.backpatch(left->jump_list, left->label);
    codebuf.backpatch(right->jump_list, right->label);

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

    push_back_child(true_value);
    push_back_child(condition);
    push_back_child(false_value);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void conditional_expression::emit_code()
{
    codebuf.backpatch(true_value->jump_list, condition->label);
    codebuf.backpatch(condition->jump_list, this->label);
    codebuf.backpatch(false_value->jump_list, this->label);
    codebuf.backpatch(condition->true_list, true_value->label);
    codebuf.backpatch(condition->false_list, false_value->label);

    if (return_type == type_kind::Bool)
    {
        true_list = codebuf.merge(true_value->true_list, false_value->true_list);
        false_list = codebuf.merge(true_value->false_list, false_value->false_list);
    }
    else
    {
        string res_type = ir_builder::get_register_type(return_type);

        codebuf.emit("%s = phi %s [ %s , %s ] [ %s , %s ]", this->place, res_type, true_value->place, true_value->label, false_value->place, false_value->label);
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

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
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

void identifier_expression::emit_code()
{
    const symbol* symbol = symtab.get_symbol(identifier); 

    if (symbol == nullptr)
    {
        throw std::logic_error("wtf");
    }

    if (symbol->kind == symbol_kind::Parameter)
    {
        string param_reg = ir_builder::format_string("%%%d", -symbol->offset - 1);

        codebuf.emit("%s = add i32 0 , %s", this->place, param_reg);
    }
    else if (symbol->kind == symbol_kind::Variable)
    {
        string ptr_reg = static_cast<const variable_symbol*>(symbol)->ptr_reg;

        string res_type = ir_builder::get_register_type(return_type);

        codebuf.emit("%s = load %s , %s* %s", this->place, res_type, res_type, ptr_reg);
    }

    if (return_type == type_kind::Bool)
    {
        size_t line = codebuf.emit("br i1 %s , label @ , label @", this->place);

        true_list.push_back(patch_record(line, label_index::First));
        false_list.push_back(patch_record(line, label_index::Second));
    }
}

invocation_expression::invocation_expression(syntax_token* identifier_token):
    expression_syntax(get_return_type(identifier_token->text)), identifier_token(identifier_token), identifier(identifier_token->text), arguments(nullptr)
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

    if (parameter_types.size() != 0)
    {
        output::error_prototype_mismatch(identifier_token->position, identifier, params_str);
    }

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

    push_back_child(arguments);

    emit_init();
    emit_code();

    for (syntax_base* child : get_children())
    {
        child->emit_clean();
    }
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

void invocation_expression::emit_code()
{
    list<string> arg_regs;

    for (auto arg : *arguments)
    {
        codebuf.backpatch(arg->jump_list, arg->label);

        if (arg->return_type != type_kind::Bool)
        {
            arg_regs.push_back(arg->place);
        }
        else
        {
            string bool_reg = ir_builder::fresh_register();
            string true_label = ir_builder::fresh_label();
            string false_label = ir_builder::fresh_label();
            string next_label = ir_builder::fresh_label();

            codebuf.backpatch(arg->true_list, true_label);
            codebuf.backpatch(arg->false_list, false_label);

            codebuf.emit("%s:", true_label);

            codebuf.emit("br label %%%s", next_label);

            codebuf.emit("%s:", false_label);

            codebuf.emit("br label %%%s", next_label);

            codebuf.emit("%s:", false_label);

            codebuf.emit("%s = phi i32 [ 1 , %s ] [ 0 , %s ]", bool_reg, true_label, false_label);

            arg_regs.push_back(bool_reg);
        }
    }

    string ret_type = ir_builder::get_register_type(return_type);

    stringstream instr;

    if (return_type == type_kind::Void)
    {
        instr << ir_builder::format_string("call %s @%s(", ret_type, identifier);
    }
    else
    {
        instr << ir_builder::format_string("%s = call %s @%s(", this->place, ret_type, identifier);
    }

    auto arg_reg = arg_regs.begin();
    auto arg = arguments->begin();

    for (; arg_reg != arg_regs.end() && arg != arguments->end(); arg_reg++, arg++)
    {
        string arg_type = ir_builder::get_register_type((*arg)->return_type);

        instr << ir_builder::format_string("%s %s", arg_type, *arg_reg);

        if (std::distance(arg_reg, arg_regs.end()) > 1)
        {
            instr << " , ";
        }
    }

    instr << ")";

    codebuf.emit(instr.str());
}
