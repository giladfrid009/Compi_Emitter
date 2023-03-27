#include "abstract_syntax.hpp"
#include "../output.hpp"
#include "../emit/code_buffer.hpp"
#include <stdexcept>
#include <string>

using std::string;
using std::list;

static code_buffer& codebuf = code_buffer::instance();

syntax_base::syntax_base(): children(), parent(nullptr)
{
}

const syntax_base* syntax_base::get_parent() const
{
    return parent;
}

void syntax_base::add_child(syntax_base* child)
{
    if (child == nullptr)
    {
        return;
    }

    children.push_back(child);
    child->parent = this;
}

void syntax_base::add_child_front(syntax_base* child)
{
    if (child == nullptr)
    {
        return;
    }

    children.push_front(child);
    child->parent = this;
}

const list<syntax_base*>& syntax_base::get_children() const
{
    return children;
}

expression_syntax::expression_syntax(type_kind return_type):
    return_type(return_type), reg(ir_builder::fresh_register())
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

statement_syntax::statement_syntax(): break_list(), continue_list()
{
}
