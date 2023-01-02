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

string get_type(type_kind type)
{
    switch (type)
    {
        case type_kind::Bool: return "i32";
        case type_kind::Byte: return "i32";
        case type_kind::Int: return "i32";
        case type_kind::String: return "i8*";
        case type_kind::Void: return "void";

        default: throw std::runtime_error("invalid type");
    }
}