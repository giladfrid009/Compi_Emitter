#include "ir_builder.hpp"
#include <string>
#include <stdexcept>

using std::string;

string ir_builder::fresh_register()
{
    static unsigned long long count = 0;

    string reg = ir_builder::format_string("%%reg_%llu", count);

    count++;

    return reg;
}

string ir_builder::fresh_label()
{
    static unsigned long long count = 0;

    string label = ir_builder::format_string("label_%llu", count);

    count++;

    return label;
}

std::string ir_builder::fresh_const()
{
    static unsigned long long count = 0;

    string label = ir_builder::format_string("@.const_var_%llu", count);

    count++;

    return label;
}

string ir_builder::get_register_type(type_kind data_type)
{
    switch (data_type)
    {
        case type_kind::Bool: return "i32";
        case type_kind::Byte: return "i32";
        case type_kind::Int: return "i32";
        case type_kind::String: return "i8*";
        case type_kind::Void: return "void";

        default: throw std::runtime_error("invalid data_type");
    }
}

string ir_builder::get_binary_instruction(arithmetic_operator oper, bool is_signed)
{
    switch (oper)
    {
        case arithmetic_operator::Add: return "add";
        case arithmetic_operator::Sub: return "sub";
        case arithmetic_operator::Mul: return "mul";
        case arithmetic_operator::Div: return is_signed ? "sdiv" : "udiv";

        default: throw std::runtime_error("unknown oper");
    }
}

string ir_builder::get_comparison_kind(relational_operator oper, bool is_signed)
{
    switch (oper)
    {
        case relational_operator::Equal: return "eq";
        case relational_operator::NotEqual: return "ne";
        case relational_operator::Greater: return is_signed ? "sgt" : "ugt";
        case relational_operator::GreaterEqual: return is_signed ? "sge" : "uge";
        case relational_operator::Less: return is_signed ? "slt" : "ult";
        case relational_operator::LessEqual: return is_signed ? "sle" : "ule";

        default: throw std::runtime_error("unknown oper");
    }
}