#include "symbol_table.hpp"
#include "scope.hpp"

using std::string;
using std::vector;
using std::list;

symbol_table::symbol_table(): scope_list()
{

}

symbol_table& symbol_table::instance()
{
    static symbol_table instance;
    return instance;
}

void symbol_table::open_scope(bool loop_scope)
{
    if (scope_list.size() == 0)
    {
        scope_list.push_back(scope(0, loop_scope));
    }
    else
    {
        scope_list.push_back(scope(scope_list.back().offset, loop_scope));
    }
}

void symbol_table::close_scope()
{
    scope_list.pop_back();
}

const scope& symbol_table::current_scope() const
{
    return scope_list.back();
}


bool symbol_table::contains_symbol(const string& name) const
{
    for (const scope& sc : scope_list)
    {
        if (sc.contains_symbol(name))
        {
            return true;
        }
    }

    return false;
}

const symbol* symbol_table::get_symbol(const string& name) const
{
    for (const scope& sc : scope_list)
    {
        if (sc.contains_symbol(name))
        {
            return sc.get_symbol(name);
        }
    }

    return nullptr;
}

bool symbol_table::add_variable(const string& name, type_kind type)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_variable(name, type);
}

bool symbol_table::add_parameter(const string& name, type_kind type)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_parameter(name, type);
}

bool symbol_table::add_function(const string& name, type_kind return_type, const vector<type_kind>& parameter_types)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_function(name, return_type, parameter_types);
}

bool symbol_table::add_function(const string& name, type_kind return_type)
{
    return add_function(name, return_type, vector<type_kind>());
}

const list<scope>& symbol_table::get_scopes() const
{
    return scope_list;
}
