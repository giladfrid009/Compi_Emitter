#include "abstract_syntax.hpp"
#include "hw3_output.hpp"
#include "code_buffer.hpp"
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

void syntax_base::emit_clean()
{
}

void syntax_base::emit()
{
    emit_node();

    for (auto child : children)
    {
        child->emit_clean();
    }
}

string syntax_base::get_bool_reg(const expression_syntax* bool_expression)
{
    if (bool_expression->return_type != type_kind::Bool)
    {
        throw std::logic_error("invalid bool_expression");
    }
    
    string bool_reg = ir_builder::fresh_register();
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string end_label = ir_builder::fresh_label();

    codebuf.backpatch(bool_expression->true_list, true_label);
    codebuf.backpatch(bool_expression->false_list, false_label);

    codebuf.emit("%s:", true_label);

    codebuf.emit("br label %%%s", end_label);

    codebuf.emit("%s:", false_label);

    codebuf.emit("br label %%%s", end_label);

    codebuf.emit("%s:", end_label);

    codebuf.emit("%s = phi i32 [ 1 , %%%s ] , [ 0 , %%%s ]", bool_reg, true_label, false_label);

    return bool_reg;
}

expression_syntax::expression_syntax(type_kind return_type):
    return_type(return_type), place(ir_builder::fresh_register()), true_list(), false_list(), jump_label(), jump_list()
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

void expression_syntax::emit_clean()
{
    true_list.clear();
    false_list.clear();
    jump_list.clear();
}

statement_syntax::statement_syntax(): break_list(), continue_list()
{
}

void statement_syntax::emit_clean()
{
    break_list.clear();
    continue_list.clear();
}
