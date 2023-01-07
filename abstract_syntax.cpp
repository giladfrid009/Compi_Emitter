#include "abstract_syntax.hpp"
#include "hw3_output.hpp"
#include <stdexcept>
#include <string>
#include "bp.hpp"

syntax_base::syntax_base()
{
}

syntax_base* syntax_base::get_parent() const
{
    return parent;
}

void syntax_base::set_parent(syntax_base* new_parent)
{
    parent = new_parent;
}

expression_syntax::expression_syntax(fundamental_type return_type): place(code_buffer::instance().create_variable()), return_type(return_type)
{
    
}

bool expression_syntax::is_numeric() const
{
    return types::is_numeric(return_type);
}

bool expression_syntax::is_special() const
{
    return types::is_special(return_type);
}

statement_syntax::statement_syntax()
{
}
