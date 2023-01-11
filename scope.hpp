#ifndef _SCOPE_HPP_
#define _SCOPE_HPP_

#include <unordered_map>
#include <list>
#include <string>
#include "symbol.hpp"
#include "abstract_syntax.hpp"

class scope
{
    friend class symbol_table;

    private:

    std::list<const symbol*> symbol_list;
    std::unordered_map<std::string, const symbol*> symbol_map;
    int offset;
    int param_offset;

    public:

    const bool loop_scope;

    scope(int offset, bool loop_scope);

    ~scope();

    bool contains_symbol(const std::string& name) const;

    bool contains_symbol(const std::string& name, symbol_kind kind) const;

    const symbol* get_symbol(const std::string& name) const;

    const symbol* get_symbol(const std::string& name, symbol_kind kind) const;

    const std::list<const symbol*>& get_symbols() const;

    bool add_variable(const std::string& name, type_kind type);

    bool add_parameter(const std::string& name, type_kind type);

    bool add_function(const std::string& name, type_kind return_type, const std::vector<type_kind>& parameter_types);
};

#endif