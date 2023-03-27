#include "types.hpp"
#include <stdexcept>

using std::string;

string types::to_string(type_kind type)
{
    switch (type)
    {
        case (type_kind::Bool): return "bool";
        case (type_kind::Int): return "int";
        case (type_kind::Byte): return "byte";
        case (type_kind::String): return "string";
        case (type_kind::Void): return "void";

        default: throw std::invalid_argument("unknown type");
    }
}

type_kind types::parse(const string& str)
{
    if (str == "bool") return type_kind::Bool;
    if (str == "int") return type_kind::Int;
    if (str == "byte") return type_kind::Byte;
    if (str == "string") return type_kind::String;
    if (str == "void") return type_kind::Void;

    throw std::invalid_argument("can't parse str");
}

bool types::is_implictly_convertible(type_kind from, type_kind to)
{
    if (from == type_kind::Invalid || to == type_kind::Invalid)
    {
        return false;
    }

    if (from == type_kind::Void || to == type_kind::Void)
    {
        return false;
    }

    if (from == to)
    {
        return true;
    }

    if (from == type_kind::Byte && to == type_kind::Int)
    {
        return true;
    }

    return false;
}

type_kind types::cast_up(type_kind first, type_kind second)
{
    if (first == type_kind::Invalid || second == type_kind::Invalid)
    {
        return type_kind::Invalid;
    }

    if (first == type_kind::Void || second == type_kind::Void)
    {
        return type_kind::Invalid;
    }

    if (first == second)
    {
        return first;
    }

    if (first == type_kind::Int && second == type_kind::Byte)
    {
        return type_kind::Int;
    }

    if (first == type_kind::Byte && second == type_kind::Int)
    {
        return type_kind::Int;
    }

    return type_kind::Invalid;
}

bool types::is_numeric(type_kind type)
{
    return type == type_kind::Int || type == type_kind::Byte;
}

bool types::is_special(type_kind type)
{
    return type == type_kind::Invalid || type == type_kind::Void || type == type_kind::String;
}