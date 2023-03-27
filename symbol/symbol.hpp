#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include <string>
#include <vector>
#include "../syntax/abstract_syntax.hpp"

enum class symbol_kind { Variable, Parameter, Function };

class symbol
{
    public:

    const symbol_kind kind;
    const std::string name;
    const int offset;
    const type_kind type;

    protected:

    symbol(const std::string& name, type_kind type, int offset, symbol_kind kind);

    public:

    virtual ~symbol() = default;

    virtual std::string to_string() const = 0;
};

class variable_symbol: public symbol
{
    public:

    const std::string ptr_reg;

    variable_symbol(const std::string& name, type_kind type, int offset);

    std::string to_string() const override;
};

class parameter_symbol: public symbol
{
    public:

    parameter_symbol(const std::string& name, type_kind type, int offset);

    std::string to_string() const override;
};

class function_symbol: public symbol
{
    public:

    const std::vector<type_kind> parameter_types;

    function_symbol(const std::string& name, type_kind return_type, const std::vector<type_kind>& parameter_types);

    std::string to_string() const override;
};

#endif