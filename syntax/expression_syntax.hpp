#ifndef _EXPRESSION_SYNTAX_HPP_
#define _EXPRESSION_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include "generic_syntax.hpp"
#include "../emit/code_buffer.hpp"
#include "syntax_operators.hpp"
#include "../output.hpp"
#include "../symbol/symbol.hpp"
#include <vector>
#include <string>
#include <stdexcept>

template<typename literal_type> class literal_expression final: public expression_syntax
{
    public:

    const syntax_token* const value_token;
    const literal_type value;

    literal_expression(syntax_token* value_token):
        expression_syntax(get_return_type()), value_token(value_token), value(get_literal_value(value_token))
    {
        semantic_analysis();
    }

    literal_expression(const literal_expression& other) = delete;

    literal_expression& operator=(const literal_expression& other) = delete;

    type_kind get_return_type() const
    {
        if (std::is_same<literal_type, unsigned char>::value) return type_kind::Byte;
        if (std::is_same<literal_type, int>::value) return type_kind::Int;
        if (std::is_same<literal_type, bool>::value) return type_kind::Bool;
        if (std::is_same<literal_type, std::string>::value) return type_kind::String;
        return type_kind::Invalid;
    }

    inline literal_type get_literal_value(syntax_token* value_token) const
    {
        throw std::runtime_error("invalid literal_type");
    }

    ~literal_expression()
    {
        for (syntax_base* child : get_children())
        {
            delete child;
        }

        delete value_token;
    }

    void semantic_analysis() const override
    {
        if (types::is_special(return_type) && return_type != type_kind::String)
        {
            throw std::runtime_error("invalid literal_type");
        }
    }

    void emit_code() override
    {
        code_buffer& code_buf = code_buffer::instance();

        std::string ret_type = ir_builder::get_type(return_type);

        code_buf.emit("%s = add %s 0, %d", this->reg, ret_type, value);
    }
};

template<> inline int literal_expression<int>::get_literal_value(syntax_token* value_token) const
{
    return std::stoi(value_token->text);
}

template<> inline unsigned char literal_expression<unsigned char>::get_literal_value(syntax_token* value_token) const
{
    int value = std::stoi(value_token->text);

    if (value < 0 || value > 255)
    {
        output::error_byte_too_large(value_token->position, value_token->text);
    }

    return static_cast<unsigned char>(value);
}

template<> inline std::string literal_expression<std::string>::get_literal_value(syntax_token* value_token) const
{
    return std::string(value_token->text);
}

template<> inline bool literal_expression<bool>::get_literal_value(syntax_token* value_token) const
{
    if (value_token->text == "true") return true;
    if (value_token->text == "false") return false;

    throw std::runtime_error("invalid value_token text");
}

template<> inline void literal_expression<std::string>::emit_code()
{
    code_buffer& code_buf = code_buffer::instance();

    std::string arr_name = ir_builder::fresh_global();
    std::string arr_content = value.substr(1, value.length() - 2);
    std::string arr_type = ir_builder::format_string("[%d x i8]", arr_content.length() + 1);

    code_buf.emit_global(ir_builder::format_string("%s = constant %s c\"%s\\00\"", arr_name, arr_type, arr_content));

    code_buf.emit("%s = getelementptr %s, %s* %s, i32 0, i32 0", reg, arr_type, arr_type, arr_name);
}

class cast_expression final: public expression_syntax
{
    public:

    type_syntax* const destination_type;
    expression_syntax* const value;

    cast_expression(type_syntax* destination_type, expression_syntax* expression);
    ~cast_expression();

    cast_expression(const cast_expression& other) = delete;
    cast_expression& operator=(const cast_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;
};

class not_expression final: public expression_syntax
{
    public:

    const syntax_token* const not_token;
    expression_syntax* const expression;

    not_expression(syntax_token* not_token, expression_syntax* expression);
    ~not_expression();

    not_expression(const not_expression& other) = delete;
    not_expression& operator=(const not_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;
};

class logical_expression final: public expression_syntax
{
    public:

    enum class operator_kind { And, Or };

    expression_syntax* const left;
    const syntax_token* const oper_token;
    expression_syntax* const right;
    const operator_kind oper;

    logical_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right);
    ~logical_expression();

    logical_expression(const logical_expression& other) = delete;
    logical_expression& operator=(const logical_expression& other) = delete;

    static operator_kind parse_operator(std::string str);

    void semantic_analysis() const override;
    void emit_code() override;
};

class arithmetic_expression final: public expression_syntax
{
    public:

    expression_syntax* const left;
    const syntax_token* const oper_token;
    expression_syntax* const right;
    const arithmetic_operator oper;

    arithmetic_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right);
    ~arithmetic_expression();

    arithmetic_expression(const arithmetic_expression& other) = delete;
    arithmetic_expression& operator=(const arithmetic_expression& other) = delete;

    static arithmetic_operator parse_operator(std::string str);

    void semantic_analysis() const override;
    void emit_code() override;
};

class relational_expression final: public expression_syntax
{
    public:

    expression_syntax* const left;
    const syntax_token* const oper_token;
    expression_syntax* const right;
    const relational_operator oper;

    relational_expression(expression_syntax* left, syntax_token* oper_token, expression_syntax* right);
    ~relational_expression();

    relational_expression(const relational_expression& other) = delete;
    relational_expression& operator=(const relational_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;

    protected:

    static relational_operator parse_operator(std::string str);
};

class conditional_expression final: public expression_syntax
{
    public:

    expression_syntax* const true_value;
    const syntax_token* const if_token;
    expression_syntax* const condition;
    const syntax_token* const else_token;
    expression_syntax* const false_value;

    conditional_expression(expression_syntax* true_value, syntax_token* if_token, expression_syntax* condition, syntax_token* else_token, expression_syntax* false_value);
    ~conditional_expression();

    conditional_expression(const conditional_expression& other) = delete;
    conditional_expression& operator=(const conditional_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;

    protected:

    static type_kind get_return_type(expression_syntax* left, expression_syntax* right);

};

class identifier_expression final: public expression_syntax
{
    public:

    const syntax_token* const identifier_token;
    const std::string identifier;

    private:

    symbol_kind kind;
    std::string ptr_reg;

    public:

    identifier_expression(syntax_token* identifier_token);
    ~identifier_expression();

    identifier_expression(const identifier_expression& other) = delete;
    identifier_expression& operator=(const identifier_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;

    protected:

    static type_kind get_return_type(std::string identifier);

};

class invocation_expression final: public expression_syntax
{
    public:

    const syntax_token* const identifier_token;
    const std::string identifier;
    list_syntax<expression_syntax>* const arguments;

    invocation_expression(syntax_token* identifier_token);
    invocation_expression(syntax_token* identifier_token, list_syntax<expression_syntax>* arguments);
    ~invocation_expression();

    invocation_expression(const invocation_expression& other) = delete;
    invocation_expression& operator=(const invocation_expression& other) = delete;

    void semantic_analysis() const override;
    void emit_code() override;

    protected:

    static type_kind get_return_type(std::string identifier);
    std::string get_arguments(const list_syntax<expression_syntax>* arguments);
};

#endif