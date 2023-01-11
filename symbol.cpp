#include "symbol.hpp"
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

symbol::symbol(const string& name, type_kind type, int offset, symbol_kind kind):
    kind(kind), name(name), offset(offset), type(type)
{

}

variable_symbol::variable_symbol(const string& name, type_kind type, int offset):
    symbol(name, type, offset, symbol_kind::Variable)
{

}

string variable_symbol::to_string() const
{
    stringstream res;

    res << name << " " << types::to_string(type) << " " << offset;

    return res.str();
}

function_symbol::function_symbol(const string& name, type_kind return_type, const vector<type_kind>& parameter_types):
    symbol(name, return_type, 0, symbol_kind::Function), parameter_types(parameter_types)
{

}

string function_symbol::to_string() const
{
    stringstream res;

    res << name << " " << "(";

    for (auto& param : parameter_types)
    {
        res << types::to_string(param);

        if (&param != &parameter_types.back())
        {
            res << ",";
        }
    }

    res << ")" << "->" << types::to_string(type) << " " << offset;

    return res.str();
}

parameter_symbol::parameter_symbol(const string& name, type_kind type, int offset):
    symbol(name, type, offset, symbol_kind::Parameter)
{
    
}

std::string parameter_symbol::to_string() const
{
    stringstream res;

    res << name << " " << types::to_string(type) << " " << offset;

    return res.str();
}
