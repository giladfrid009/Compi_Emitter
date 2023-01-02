#ifndef _GENERIC_SYNTAX_HPP_
#define _GENERIC_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <vector>
#include <string>
#include <type_traits>

template<typename element_type> class list_syntax final: public syntax_base
{
    private:

    std::list<const element_type*> elements;

    public:

    list_syntax(): elements()
    {
        static_assert(std::is_base_of<syntax_base, element_type>::value, "must be of type syntax_base");
    }

    list_syntax(element_type* element): elements{ element }
    {
        static_assert(std::is_base_of<syntax_base, element_type>::value, "must be of type syntax_base");

        push_back_child(element);
    }

    list_syntax(const list_syntax& other) = delete;

    list_syntax& operator=(const list_syntax& other) = delete;

    list_syntax<element_type>* push_back(element_type* element)
    {
        elements.push_back(element);
        push_back_child(element);
        return this;
    }

    list_syntax<element_type>* push_front(element_type* element)
    {
        elements.push_front(element);
        push_front_child(element);
        return this;
    }

    std::size_t size() const
    {
        return elements.size();
    }

    typename std::list<const element_type*>::const_iterator begin() const
    {
        return elements.begin();
    }

    typename std::list<const element_type*>::const_iterator end() const
    {
        return elements.end();
    }

    ~list_syntax()
    {
        for (syntax_base* child : get_children())
        {
            delete child;
        }
    }

    protected:

    void emit_node() override
    {

    }
};

class type_syntax final: public syntax_base
{
    public:

    const syntax_token* const type_token;
    const type_kind kind;

    type_syntax(syntax_token* type_token);
    ~type_syntax();

    type_syntax(const type_syntax& other) = delete;
    type_syntax& operator=(const type_syntax& other) = delete;

    bool is_numeric() const;
    bool is_special() const;

    protected:

    void emit_node() override;
};

class parameter_syntax final: public syntax_base
{
    public:

    type_syntax* const type;
    const syntax_token* const identifier_token;
    const std::string identifier;

    parameter_syntax(type_syntax* type, syntax_token* identifier_token);
    ~parameter_syntax();

    parameter_syntax(const parameter_syntax& other) = delete;
    parameter_syntax& operator=(const parameter_syntax& other) = delete;

    protected:

    void emit_node() override;
};

class function_declaration_syntax final: public syntax_base
{
    public:

    type_syntax* const return_type;
    const syntax_token* const identifier_token;
    const std::string identifier;
    list_syntax<parameter_syntax>* const parameters;
    list_syntax<statement_syntax>* const body;

    function_declaration_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters, list_syntax<statement_syntax>* body);
    ~function_declaration_syntax();

    function_declaration_syntax(const function_declaration_syntax& other) = delete;
    function_declaration_syntax& operator=(const function_declaration_syntax& other) = delete;

    protected:

    void emit_node() override;
};

class root_syntax final: public syntax_base
{
    public:

    list_syntax<function_declaration_syntax>* const functions;

    root_syntax(list_syntax<function_declaration_syntax>* functions);
    ~root_syntax();

    root_syntax(const root_syntax& other) = delete;
    root_syntax& operator=(const root_syntax& other) = delete;

    void emit_tree();

    protected:

    void emit_node() override;
};

class label_syntax final: public syntax_base
{
    public:

    const std::string name;

    label_syntax();
    ~label_syntax();

    label_syntax(const root_syntax& other) = delete;
    label_syntax& operator=(const root_syntax& other) = delete;

    protected:

    void emit_node() override;
};

#endif
