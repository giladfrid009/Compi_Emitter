#include "symbol.hpp"
#include "bp.hpp"
#include <vector>

using std::string;
using std::vector;

symbol::symbol(string name, fundamental_type type, int offset, symbol_type sym_type):
    sym_type(sym_type), name(name), offset(offset), type(type)
{

}

variable_symbol::variable_symbol(string name, fundamental_type type, int offset):
    symbol(name, type, offset, symbol_type::Var) , ir_pointer_register(code_buffer::instance().register_name())
{

}

function_symbol::function_symbol(string name, fundamental_type return_type, vector<fundamental_type> parameter_types):
    symbol(name, return_type, 0, symbol_type::Func), parameter_types(parameter_types)
{

}