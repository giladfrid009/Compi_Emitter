#ifndef _SYMBOL_TABLE_HPP_
#define _SYMBOL_TABLE_HPP_

#include <string>
#include <list>
#include "scope.hpp"

class symbol_table
{
    private:

    std::list<scope> _scope_list;

    symbol_table();

    public:

    static symbol_table& instance();

    void open_scope(bool loop_scope = false);
    void close_scope();

    const scope& current_scope() const;

    bool contains_symbol(const std::string& name) const;
    bool contains_symbol(const std::string& name, symbol_kind kind) const;

    const symbol* get_symbol(const std::string& name) const;
    const symbol* get_symbol(const std::string& name, symbol_kind kind) const;

    bool add_variable(const std::string& name, type_kind type);
    bool add_parameter(const std::string& name, type_kind type);
    bool add_function(const std::string& name, type_kind return_type);
    bool add_function(const std::string& name, type_kind return_type, const std::vector<type_kind>& parameter_types);

    const std::list<scope>& scopes() const;
};

#endif