#include "scope.hpp"
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::vector;
using std::list;

scope::scope(int offset, bool loop_scope): _symbol_list(), _symbol_map(), _offset(offset), _param_offset(offset - 1), loop_scope(loop_scope)
{
}

scope::~scope()
{
    for (const symbol* sym : _symbol_list)
    {
        delete sym;
    }
}

bool scope::contains_symbol(const string& name) const
{
    return _symbol_map.find(name) != _symbol_map.end();
}

bool scope::contains_symbol(const string& name, symbol_kind kind) const
{
    auto key_val = _symbol_map.find(name);

    if (key_val == _symbol_map.end() || key_val->second->kind != kind)
    {
        return false;
    }

    return true;
}

const symbol* scope::get_symbol(const string& name) const
{
    if (contains_symbol(name) == false)
    {
        return nullptr;
    }

    return _symbol_map.at(name);
}

const symbol* scope::get_symbol(const string& name, symbol_kind kind) const
{
    if (contains_symbol(name, kind) == false)
    {
        return nullptr;
    }

    return _symbol_map.at(name);
}

const list<const symbol*>& scope::symbols() const
{
    return _symbol_list;
}

bool scope::add_variable(const string& name, type_kind type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new variable_symbol(name, type, _offset);

    _symbol_list.push_back(new_symbol);
    _symbol_map[name] = new_symbol;
    _offset += 1;
    return true;
}

bool scope::add_parameter(const string& name, type_kind type)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new parameter_symbol(name, type, _param_offset);

    _symbol_list.push_back(new_symbol);
    _symbol_map[name] = new_symbol;
    _param_offset -= 1;
    return true;
}

bool scope::add_function(const string& name, type_kind return_type, const vector<type_kind>& parameter_types)
{
    if (contains_symbol(name))
    {
        return false;
    }

    symbol* new_symbol = new function_symbol(name, return_type, parameter_types);

    _symbol_list.push_back(new_symbol);
    _symbol_map[name] = new_symbol;
    return true;
}
