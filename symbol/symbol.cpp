#include "symbol.hpp"
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

symbol::symbol(const string& name, type_kind type, int offset, symbol_kind kind):
    kind(kind), name(name), offset(offset), type(type)
{
}

variable_symbol::variable_symbol(const string& name, type_kind type, int offset):
    symbol(name, type, offset, symbol_kind::Variable), ptr_reg(ir_builder::fresh_register())
{
}

function_symbol::function_symbol(const string& name, type_kind return_type, const vector<type_kind>& parameter_types):
    symbol(name, return_type, 0, symbol_kind::Function), parameter_types(parameter_types)
{

}

parameter_symbol::parameter_symbol(const string& name, type_kind type, int offset):
    symbol(name, type, offset, symbol_kind::Parameter)
{
}
