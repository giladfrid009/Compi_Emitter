#include "scope.hpp"
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::vector;
using std::list;

scope::scope(int offset, bool loop_scope):
    symbol_list(), symbol_map(), offset(offset), param_offset(offset - 1), loop_scope(loop_scope)
{
}

scope::~scope()
{
    for (const symbol* sym : symbol_list)
    {
        delete sym;
    }
}

bool scope::contains_symbol(const string& name) const
{
    return symbol_map.find(name) != symbol_map.end();
}

const symbol* scope::get_symbol(const string& name) const
{
    if (contains_symbol(name) == false)
    {
        return nullptr;
    }

    return symbol_map.at(name);
}

const list<const symbol*>& scope::get_symbols() const
{
    return symbol_list;
}

bool scope::add_variable(const string& name, type_kind type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new variable_symbol(name, type, offset);
    symbol_list.push_back(new_symbol);
    symbol_map[name] = new_symbol;

    offset += 1;

    return true;
}

bool scope::add_parameter(const string& name, type_kind type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new variable_symbol(name, type, param_offset);
    symbol_list.push_back(new_symbol);
    symbol_map[name] = new_symbol;

    param_offset -= 1;

    return true;
}

bool scope::add_function(const string& name, type_kind return_type, const vector<type_kind>& parameter_types)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new function_symbol(name, return_type, parameter_types);
    symbol_list.push_back(new_symbol);
    symbol_map[name] = new_symbol;

    return true;
}
