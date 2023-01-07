#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include <string>
#include <vector>
#include "abstract_syntax.hpp"

enum class symbol_type { Var, Func };

class symbol
{
    public:

    const symbol_type sym_type;

    const std::string name;

    const int offset;

    const fundamental_type type;

    symbol(std::string name, fundamental_type type, int offset, symbol_type sym_type);

    virtual ~symbol() = default;
};

class variable_symbol: public symbol
{
    public:

    variable_symbol(std::string name, fundamental_type type, int offset);
};

class function_symbol: public symbol
{
    public:

    const std::vector<fundamental_type> parameter_types;

    function_symbol(std::string name, fundamental_type return_type, std::vector<fundamental_type> parameter_types);
};

#endif