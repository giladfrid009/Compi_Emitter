#include "abstract_syntax.hpp"
#include "output.hpp"
#include "bp.hpp"
#include <stdexcept>
#include <string>

using std::list;

syntax_base::syntax_base(): children(), parent(nullptr)
{
}

const syntax_base* syntax_base::get_parent() const
{
    return parent;
}

void syntax_base::push_back_child(syntax_base* child)
{
    if (child == nullptr)
    {
        return;
    }

    children.push_back(child);
    child->parent = this;
}

void syntax_base::push_front_child(syntax_base* child)
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

void syntax_base::emit_tree()
{
    for (syntax_base* child : children)
    {
        if (child == nullptr) 
        {
            continue;
        }

        child->emit_tree();
    }

    this->emit_node();
}

expression_syntax::expression_syntax(type_kind return_type): return_type(return_type), place(code_buffer::instance().register_name())
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
