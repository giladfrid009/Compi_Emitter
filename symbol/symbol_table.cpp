#include "symbol_table.hpp"
#include "scope.hpp"

using std::string;
using std::vector;
using std::list;

symbol_table::symbol_table(): _scope_list()
{

}

symbol_table& symbol_table::instance()
{
    static symbol_table instance;
    return instance;
}

void symbol_table::open_scope(bool loop_scope)
{
    if (_scope_list.size() == 0)
    {
        _scope_list.push_back(scope(0, loop_scope));
    }
    else
    {
        _scope_list.push_back(scope(_scope_list.back()._offset, loop_scope));
    }
}

void symbol_table::close_scope()
{
    _scope_list.pop_back();
}

const scope& symbol_table::current_scope() const
{
    return _scope_list.back();
}


bool symbol_table::contains_symbol(const string& name) const
{
    for (const scope& sc : _scope_list)
    {
        if (sc.contains_symbol(name))
        {
            return true;
        }
    }

    return false;
}

bool symbol_table::contains_symbol(const string& name, symbol_kind kind) const
{
    for (const scope& sc : _scope_list)
    {
        if (sc.contains_symbol(name, kind))
        {
            return true;
        }
    }

    return false;
}

const symbol* symbol_table::get_symbol(const string& name) const
{
    for (const scope& sc : _scope_list)
    {
        if (sc.contains_symbol(name))
        {
            return sc.get_symbol(name);
        }
    }

    return nullptr;
}

const symbol* symbol_table::get_symbol(const string& name, symbol_kind kind) const
{
    for (const scope& sc : _scope_list)
    {
        if (sc.contains_symbol(name))
        {
            return sc.get_symbol(name, kind);
        }
    }

    return nullptr;
}

bool symbol_table::add_variable(const string& name, type_kind type)
{
    if (_scope_list.size() == 0)
    {
        return false;
    }

    return _scope_list.back().add_variable(name, type);
}

bool symbol_table::add_parameter(const string& name, type_kind type)
{
    if (_scope_list.size() == 0)
    {
        return false;
    }

    return _scope_list.back().add_parameter(name, type);
}

bool symbol_table::add_function(const string& name, type_kind return_type, const vector<type_kind>& parameter_types)
{
    if (_scope_list.size() == 0)
    {
        return false;
    }

    return _scope_list.back().add_function(name, return_type, parameter_types);
}

bool symbol_table::add_function(const string& name, type_kind return_type)
{
    return add_function(name, return_type, vector<type_kind>());
}

const list<scope>& symbol_table::scopes() const
{
    return _scope_list;
}
