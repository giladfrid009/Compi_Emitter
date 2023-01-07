#include "types.hpp"
#include <stdexcept>

using std::string;

string types::to_string(fundamental_type type)
{
    switch (type)
    {
        case (fundamental_type::Bool): return "BOOL";
        case (fundamental_type::Int): return "INT";
        case (fundamental_type::Byte): return "BYTE";
        case (fundamental_type::String): return "STRING";
        case (fundamental_type::Void): return "VOID";

        default: throw std::invalid_argument("unknown type");
    }
}

fundamental_type types::from_string(string str)
{
    if (str == "bool") return fundamental_type::Bool;
    if (str == "int") return fundamental_type::Int;
    if (str == "byte") return fundamental_type::Byte;
    if (str == "string") return fundamental_type::String;
    if (str == "void") return fundamental_type::Void;

    throw std::invalid_argument("can't parse str");
}

bool types::is_convertible(fundamental_type source, fundamental_type destiation)
{
    if (source == fundamental_type::Invalid || destiation == fundamental_type::Invalid)
    {
        return false;
    }

    if (source == fundamental_type::Void || destiation == fundamental_type::Void)
    {
        return false;
    }
    
    if (source == destiation)
    {
        return true;
    }

    if (source == fundamental_type::Byte && destiation == fundamental_type::Int)
    {
        return true;
    }

    return false;
}

fundamental_type types::cast_up(fundamental_type first, fundamental_type second)
{
    if (first == fundamental_type::Invalid || second == fundamental_type::Invalid)
    {
        return fundamental_type::Invalid;
    }

    if (first == fundamental_type::Void || second == fundamental_type::Void)
    {
        return fundamental_type::Invalid;
    }

    if (first == second)
    {
        return first;
    }

    if (first == fundamental_type::Int && second == fundamental_type::Byte)
    {
        return fundamental_type::Int;
    }

    if (first == fundamental_type::Byte && second == fundamental_type::Int)
    {
        return fundamental_type::Int;
    }

    return fundamental_type::Invalid;
}

bool types::is_numeric(fundamental_type type)
{
    if (type == fundamental_type::Invalid)
    {
        return false;
    }

    return type == fundamental_type::Int || type == fundamental_type::Byte;
}

bool types::is_special(fundamental_type type)
{
    return type == fundamental_type::Invalid || type == fundamental_type::Void || type == fundamental_type::String;
}