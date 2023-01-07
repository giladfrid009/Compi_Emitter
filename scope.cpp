#include "scope.hpp"
#include "hw3_output.hpp"
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::vector;
using std::list;

scope::scope(int offset, bool is_loop_scope):
    symbol_list(), symbol_map(), current_offset(offset), formal_offset(offset - 1), is_loop_scope(is_loop_scope)
{
}

scope::~scope()
{
    for (symbol* sym : symbol_list)
    {
        delete sym;
    }
}

bool scope::contains_symbol(string symbol_name) const
{
    return symbol_map.find(symbol_name) != symbol_map.end();
}

symbol* scope::get_symbol(string symbol_name) const
{
    if (contains_symbol(symbol_name) == false)
    {
        return nullptr;
    }

    return symbol_map.at(symbol_name);
}

const list<symbol*>& scope::get_symbols() const
{
    return symbol_list;
}

bool scope::add_variable(string name, fundamental_type type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new variable_symbol(name, type, current_offset);
    symbol_list.push_back(new_symbol);
    symbol_map[name] = new_symbol;

    current_offset += 1;

    return true;
}

bool scope::add_formal(string name, fundamental_type type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new variable_symbol(name, type, formal_offset);
    symbol_list.push_back(new_symbol);
    symbol_map[name] = new_symbol;

    formal_offset -= 1;

    return true;
}

bool scope::add_function(string name, fundamental_type return_type, vector<fundamental_type> parameter_types)
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
