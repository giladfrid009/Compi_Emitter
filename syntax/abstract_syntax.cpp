#include "abstract_syntax.hpp"
#include "../output.hpp"
#include "../emit/code_buffer.hpp"
#include <stdexcept>
#include <string>
#include <initializer_list>

using std::string;
using std::list;
using std::initializer_list;

static code_buffer& code_buf = code_buffer::instance();

syntax_base::syntax_base(): _children(), _parent(nullptr)
{
}

syntax_base::~syntax_base()
{
    for (syntax_base* child : _children)
    {
        delete child;
    }
}

const syntax_base* syntax_base::parent() const
{
    return _parent;
}

void syntax_base::add_child(syntax_base* child)
{
    if (child == nullptr)
    {
        return;
    }

    _children.push_back(child);
    child->_parent = this;
}

void syntax_base::add_children(initializer_list<syntax_base*> children)
{
    for (auto child : children)
    {
        add_child(child);
    }
}

void syntax_base::add_child_front(syntax_base* child)
{
    if (child == nullptr)
    {
        return;
    }

    _children.push_front(child);
    child->_parent = this;
}

const list<syntax_base*>& syntax_base::children() const
{
    return _children;
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
