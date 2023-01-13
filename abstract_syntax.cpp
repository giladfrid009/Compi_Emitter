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
    emit_init();
    emit_node();

    for (auto child : children)
    {
        child->emit_clean();
    }
}

void syntax_base::emit_init()
{
}

string syntax_base::emit_get_bool(const expression_syntax* bool_expression)
{
    string bool_reg = ir_builder::fresh_register();
    string true_label = ir_builder::fresh_label();
    string false_label = ir_builder::fresh_label();
    string next_label = ir_builder::fresh_label();

    codebuf.backpatch(bool_expression->true_list, true_label);
    codebuf.backpatch(bool_expression->false_list, false_label);

    codebuf.emit("%s:", true_label);

    codebuf.emit("br label %%%s", next_label);

    codebuf.emit("%s:", false_label);

    codebuf.emit("br label %%%s", next_label);

    codebuf.emit("%s:", false_label);

    codebuf.emit("%s = phi i32 [ 1 , %s ] , [ 0 , %s ]", bool_reg, true_label, false_label);

    return bool_reg;
}

expression_syntax::expression_syntax(type_kind return_type):
    return_type(return_type), place(ir_builder::fresh_register()), label(ir_builder::fresh_label()), true_list(), false_list(), jump_list()
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

void expression_syntax::emit_init()
{
    size_t line = codebuf.emit("br label @");

    codebuf.emit("%s:", this->label);

    jump_list.push_back(patch_record(line, label_index::First));
}

void expression_syntax::emit_clean()
{
    true_list.clear();
    false_list.clear();
    jump_list.clear();
}

statement_syntax::statement_syntax(): next_list(), break_list(), continue_list(), label(ir_builder::fresh_label())
{
}

void statement_syntax::emit_init()
{
    codebuf.emit("%s:", this->label);
}

void statement_syntax::emit_clean()
{
    next_list.clear();
    break_list.clear();
    continue_list.clear();
}
