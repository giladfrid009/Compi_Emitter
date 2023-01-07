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

void symbol_table::open_scope(bool is_loop_scope)
{
    if (scope_list.size() == 0)
    {
        scope_list.push_back(scope(0, is_loop_scope));
    }
    else
    {
        scope_list.push_back(scope(scope_list.back().current_offset, is_loop_scope));
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


bool symbol_table::contains_symbol(string symbol_name) const
{
    for (const scope& sc : scope_list)
    {
        if (sc.contains_symbol(symbol_name))
        {
            return true;
        }
    }

    return false;
}

symbol* symbol_table::get_symbol(string symbol_name) const
{
    for (const scope& sc : scope_list)
    {
        if (sc.contains_symbol(symbol_name))
        {
            return sc.get_symbol(symbol_name);
        }
    }

    return nullptr;
}

bool symbol_table::add_variable(string name, fundamental_type type)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_variable(name, type);
}

bool symbol_table::add_formal(string name, fundamental_type type)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_formal(name, type);
}

bool symbol_table::add_function(string name, fundamental_type return_type, vector<fundamental_type> parameter_types)
{
    if (scope_list.size() == 0)
    {
        return false;
    }

    return scope_list.back().add_function(name, return_type, parameter_types);
}

bool symbol_table::add_function(string name, fundamental_type return_type)
{
    return add_function(name, return_type, vector<fundamental_type>());
}

const list<scope>& symbol_table::get_scopes() const
{
    return scope_list;
}
